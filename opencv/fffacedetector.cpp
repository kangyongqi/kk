#include "fffacedetector.h"
/*
 * 拿到视频帧 → 看要不要美颜 → 转成图片格式 → 缩小图片找人脸（先正脸后侧脸）→
 * 还原人脸位置 → 防止人脸框闪灭 → 给人脸美白 + 磨皮 → 转回视频帧 → 释放垃圾 → 返回美颜帧。
 */
FFFaceDetector::FFFaceDetector()
{
    smoothValue = 0;
    whiteValue = 0;
}

FFFaceDetector::~FFFaceDetector()
{
    if(faceDetector){
        delete faceDetector;
        faceDetector = nullptr;
    }
    if(adapter){
        delete adapter;
        adapter = nullptr;
    }
}

void FFFaceDetector::init()
{
    faceDetector = new cv::CascadeClassifier();
    profileDetector = new cv::CascadeClassifier();
    int ret = faceDetector->load("../videoCapture/3rdparty/data/lbpcascades/lbpcascade_frontalface.xml");
    if(!ret){
        std::cerr << "Load lbpcascade_frontalface.xml Fail!" << std::endl;
        return;
    }

    ret = profileDetector->load("../videoCapture/3rdparty/data/haarcascades/haarcascade_profileface.xml");

    if(!ret){
        std::cerr << "Load lbpcascade_profileface.xml Fail!" << std::endl;
        return;
    }

    adapter = new FFVideoAdapter();

    // 创建一个小的测试图像用于预初始化
    cv::Mat testImage = cv::Mat::zeros(100, 100, CV_8UC3);
    cv::Mat testGray;
    // 转灰度图像
    cv::cvtColor(testImage, testGray, cv::COLOR_BGR2GRAY);

    // 预检测，触发分类器初始化
    std::vector<cv::Rect> testFaces;
    /*
    CV_WRAP void detectMultiScale(
        InputArray image,                // 输入图像
        CV_OUT std::vector<Rect>& objects, // 输出检测到的目标矩形框
        double scaleFactor = 1.1,        // 缩放因子（默认1.1）
        int minNeighbors = 3,            // 最小邻域数（默认3）
        int flags = 0,                   // 检测标志（默认0）
        Size minSize = Size(),           // 最小目标尺寸（默认空）
        Size maxSize = Size()            // 最大目标尺寸（默认空）
);
     */
    faceDetector->detectMultiScale(testGray, testFaces, 1.3, 3, 0, cv::Size(30, 30));
    profileDetector->detectMultiScale(testGray, testFaces, 1.3, 3, 0, cv::Size(30, 30));
}


AVFrame* FFFaceDetector::detectFace(AVFrame *frame)
{

    if(whiteValue.load() == 0 && smoothValue.load() == 0){
        return frame;
    }

    cv::Mat bgrMat = adapter->convertFrameToMat(frame);
    cv::Mat scaledBgrMat;
    cv::resize(bgrMat, scaledBgrMat, cv::Size(), 0.25, 0.25);  // 缩小75%

    cv::Mat grayMat;
    cv::cvtColor(scaledBgrMat,grayMat,cv::COLOR_BGR2GRAY);
    // 把照片调整一下，让暗的地方亮一点，亮的地方暗一点，整体对比度更好，人脸检测更容易！
    cv::equalizeHist(grayMat,grayMat);
    std::vector<cv::Rect>faces;
    //正脸
    faceDetector->detectMultiScale(grayMat,faces,
                                   1.3, //缩放因子
                                   3,
                                   0,
                                   cv::Size(50,50)); //最小人脸大小

    //侧脸
    std::string text = "Face";
    if(faces.empty()){
        profileDetector->detectMultiScale(grayMat,faces,
                                          1.3, //缩放因子
                                          3,
                                          0,
                                          cv::Size(50,50)); //最小人脸大小
        text = "Profile";
    }

    for (auto& face : faces) {
        face.x *= 4; face.y *= 4;
        face.width *= 4; face.height *= 4;
    }


    //保证人脸识别平滑

    const int noFaceThreshold = 60;
    if(!faces.empty()){
        lastFaces = std::move(faces);
    }
    else{
        noFaceCount ++;
        if(noFaceCount == noFaceThreshold){
            noFaceCount = 0;
            lastFaces.clear();
        }
    }

    for (const auto& face : lastFaces) {
        // 提取人脸ROI
        cv::Rect roi = face & cv::Rect(0, 0, bgrMat.cols, bgrMat.rows);
        if (roi.width <= 0 || roi.height <= 0) continue;

        cv::Mat faceROI = bgrMat(roi);

        //计算ROI的平均RGB值
        cv::Scalar meanRGB = cv::mean(faceROI);
        int avgB = static_cast<int>(meanRGB[0]);
        int avgG = static_cast<int>(meanRGB[1]);
        int avgR = static_cast<int>(meanRGB[2]);

        const int threshold = 40;

        //对接近平均值的像素增加亮度
        cv::Mat resultROI = faceROI.clone();
        for (int y = 0; y < faceROI.rows; ++y) {
            cv::Vec3b* ptr = resultROI.ptr<cv::Vec3b>(y);
            // 这里的faceROI.cols是图像的宽度，不是通道数！！！！！！！！
            for (int x = 0; x < faceROI.cols; ++x) {
                int b = ptr[x][0];
                int g = ptr[x][1];
                int r = ptr[x][2];

                // 计算与平均值的RGB距离
                int dist = std::abs(b - avgB) + std::abs(g - avgG) + std::abs(r - avgR);

                // 如果距离小于阈值，则增加亮度
                // 皮肤区域，差距小（是皮肤），就提亮
                if (dist < threshold) {
                    ptr[x][0] = cv::saturate_cast<uchar>(b + whiteValue);// <uchar>无符号字符型
                    ptr[x][1] = cv::saturate_cast<uchar>(g + whiteValue);// saturate_cast:把括号里的计算结果，转换成尖括号里的类型
                    ptr[x][2] = cv::saturate_cast<uchar>(r + whiteValue);
                }
            }
        }


        // 应用双边滤波
        int d = smoothValue * 2 + 1;  // 滤波核直径（必须为奇数）
        int sigmaSpace = (smoothValue + 1) * 10; // 空间高斯核标准差
        int sigmaColor = sigmaSpace; // 颜色高斯核标准差

        cv::Mat finalROI;
        /*
         * void cv::bilateralFilter(
        InputArray src,       // 输入图像（要磨皮的人脸ROI）
        OutputArray dst,      // 输出图像（磨皮后的人脸ROI）
        int d,                // 滤波核直径（磨皮强度核心参数）
        double sigmaColor,    // 颜色空间的高斯标准差（控制颜色相似度）
        double sigmaSpace,    // 空间空间的高斯标准差（控制距离相似度）
        int borderType = BORDER_DEFAULT // 边界处理方式，默认即可
        );
         */
        cv::bilateralFilter(resultROI, finalROI,d, sigmaColor, sigmaSpace);

        // 将结果复制回原图
        finalROI.copyTo(bgrMat(roi));
    }

    AVFrame* faceFrame = adapter->convertMatToFrame(bgrMat);
    faceFrame->pts = frame->pts;

    av_frame_unref(frame);
    av_frame_free(&frame);


    return faceFrame;
}

void FFFaceDetector::setSmoothValue(int value)
{
    smoothValue.store(value);
}

void FFFaceDetector::setWhiteValue(int value)
{
    whiteValue.store(value);
}

