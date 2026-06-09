#include "ffglrenderwidget.h"
#include<QMouseEvent>

// 顶点数据（位置(x,y) + 纹理坐标(u,v)）
const QVector<float> FFGLRenderWidget::vertices = {
    -1.0f,  1.0f,  0.0f, 0.0f,  // 顶点0：左上 (-1,1) | 纹理(0,0)
    -1.0f, -1.0f,  0.0f, 1.0f,  // 顶点1：左下 (-1,-1)| 纹理(0,1)
    1.0f, -1.0f,  1.0f, 1.0f,   // 顶点2：右下 (1,-1) | 纹理(1,1)
    1.0f,  1.0f,  1.0f, 0.0f    // 顶点3：右上 (1,1)  | 纹理(1,0)
};

const QVector<unsigned int> FFGLRenderWidget::indices = {0, 1, 2, 3};

FFGLRenderWidget::FFGLRenderWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setMinimumSize(100,100);
    // 确保每个实例使用独立的着色器程序
    shaderProgram = new QOpenGLShaderProgram(this);
}

FFGLRenderWidget::~FFGLRenderWidget() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vboVertice);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &yTexture);
    glDeleteTextures(1, &uTexture);
    glDeleteTextures(1, &vTexture);

    // 清理着色器程序
    delete shaderProgram;

    doneCurrent();
}

void FFGLRenderWidget::setAspect(float aspect_)
{
    aspect = aspect_;
}

void FFGLRenderWidget::setBlackScreen()
{
    makeCurrent();

    // 配置纹理参数并初始化为黑色YUV数据
    uint8_t yData = 0;    // Y分量全0
    uint8_t uData = 128;  // U分量全128
    uint8_t vData = 128;  // V分量全128

    // 初始化Y纹理
    glBindTexture(GL_TEXTURE_2D, yTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &yData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 初始化U纹理
    glBindTexture(GL_TEXTURE_2D, uTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &uData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 初始化V纹理
    glBindTexture(GL_TEXTURE_2D, vTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &vData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    update();
    doneCurrent();
}

void FFGLRenderWidget::setKeepRatio(bool flag)
{
    keepRatio = flag;
}

void FFGLRenderWidget::initializeGL() {
    // 把画笔准备好！以后擦画布的时候，用黑色擦。
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // 初始化缓冲对象
    // 买一个画框盒子（VAO）
    glGenVertexArrays(1, &vao);
    // 买一个顶点盒子（VBO）
    glGenBuffers(1, &vboVertice);
    // 买一个索引盒子（EBO）装 “连线顺序” 的盒子
    glGenBuffers(1, &ebo);

    // 配置顶点数据
    // 把画框盒子（VAO）打开！后面的操作都会记录在这个盒子里。打开就行了，相当于记事本！！！！！！！！！！！
    glBindVertexArray(vao);

    // 把顶点盒子（VBO）打开，把那4个点的数据（ vertices ）装进去！
    // GL_ARRAY_BUFFER 表示这是顶点缓冲数据
    glBindBuffer(GL_ARRAY_BUFFER, vboVertice);// 把图纸放到画板上
    // 将顶点数据从 CPU 内存复制到 GPU 内存     // GL_STATIC_DRAW ：告诉GPU，这些数据不会改，优化一下存储。
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);// 把图纸上的内容复印到画板上

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // 把索引盒子（EBO）打开，把连点顺序（ indices ）装进去！
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    /*
    给小画家（着色器）写个说明书，告诉他
    - 位置属性（索引0） ：每4个float里，前2个是位置（x,y）
    - 纹理坐标属性（索引1） ：每4个float里，后2个是纹理坐标（u,v）
    - glEnableVertexAttribArray ：把这两个属性打开，小画家才能用！
     */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 解绑VAO
    glBindVertexArray(0);

    // 初始化纹理
    // 纹理是 GPU 内存中存储图像数据的对象
    glGenTextures(1, &yTexture);
    glGenTextures(1, &uTexture);
    glGenTextures(1, &vTexture);

    //初始绑定为黑屏
    // 配置纹理参数并初始化为黑色YUV数据
    uint8_t yData = 0;    // Y分量全0
    uint8_t uData = 128;  // U分量全128
    uint8_t vData = 128;  // V分量全128

    /*
    往调色板上涂颜色！初始涂成黑色（yData=0）。
    虽然参数很多，但意思就是：
    - 这是个 2D 调色板
    - 大小是 1x1（先随便弄个小的，以后会改）
    - 颜色格式是单通道（GL_RED，因为 Y 只有亮度）
    - 数据是无符号字节（GL_UNSIGNED_BYTE）

    调色板缩小时候（比如视频缩小），怎么涂？
    - GL_LINEAR ：线性插值，涂出来平滑，不会有马赛克！
     */
    // 初始化Y纹理
    glBindTexture(GL_TEXTURE_2D, yTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &yData);
    // 告诉画家，放大缩小时要平滑过渡
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 告诉画家，画到边缘时不要重复，就用边缘的颜色  设置纹理环绕方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 初始化U纹理
    glBindTexture(GL_TEXTURE_2D, uTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &uData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 初始化V纹理
    glBindTexture(GL_TEXTURE_2D, vTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &vData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 加载着色器
    if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaderSource/source.vert") ||
            !shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaderSource/source.frag") ||
            !shaderProgram->link()) {
        qCritical() << "Shader error:" << shaderProgram->log();
    }
}

void FFGLRenderWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 确保使用当前实例的着色器程序
    shaderProgram->bind();

    // 设置视口
    if(keepRatio){
        glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    }
    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, yTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, uTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, vTexture);

    shaderProgram->setUniformValue("yTexture", 0);
    shaderProgram->setUniformValue("uTexture", 1);
    shaderProgram->setUniformValue("vTexture", 2);

    // 绘制四边形
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    shaderProgram->release();
}
// 窗口大小改变时的处理函数（重写 QOpenGLWidget 虚函数）
// 小画家完整的画画过程！从擦画布，到拿调色板，到画画，最后下班！每帧都重复一次，你就看到视频了！
void FFGLRenderWidget::resizeGL(int w, int h) {
    if(keepRatio){
        const float targetAspect = aspect; // 视频目标宽高比
        const float currentAspect = static_cast<float>(w) / h;// 当前窗口宽高比
        // 窗口过宽，左右加黑边
        if (currentAspect > targetAspect) {
            // 窗口过宽，左右加黑边
            viewportWidth = static_cast<int>(h * targetAspect);
            viewportHeight = h;
            viewportX = (w - viewportWidth) / 2;
            viewportY = 0;
        }
        // 窗口过高，上下加黑边
        else {
            // 窗口过高，上下加黑边
            viewportWidth = w;
            viewportHeight = static_cast<int>(w / targetAspect);
            // 从什么位置开始贴
            viewportX = 0;//最左边
            viewportY = (h - viewportHeight) / 2;//上下居中的位置
        }
    }
}

void FFGLRenderWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        emit mouseDoubleClick();
    }

    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void FFGLRenderWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        emit mouseClick();
    }

    QOpenGLWidget::mousePressEvent(event);
}

//void FFGLRenderWidget::setYUVData(uint8_t *yData, uint8_t *uData, uint8_t *vData, int width, int height) {
//    makeCurrent();

//    // 上传Y分量
//    glBindTexture(GL_TEXTURE_2D, yTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, yData);

//    // 上传UV分量（宽高各减半）
//    int uvWidth = width / 2;
//    int uvHeight = height / 2;
//    glBindTexture(GL_TEXTURE_2D, uTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uvWidth, uvHeight, 0, GL_RED, GL_UNSIGNED_BYTE, uData);
//    glBindTexture(GL_TEXTURE_2D, vTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uvWidth, uvHeight, 0, GL_RED, GL_UNSIGNED_BYTE, vData);

//    update();
//    doneCurrent();

//    delete[] yData;
//    delete[] uData;
//    delete[] vData;
//}

void FFGLRenderWidget::setYUVData(AVFrame *frame)
{
    makeCurrent();

    int width = frame->width;
    int height = frame->height;

    // 上传Y分量
    glBindTexture(GL_TEXTURE_2D, yTexture);
    // 设置行对齐
    // frame->linesize[0] ：Y 分量每行的 字节数 （如 1920）
    glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[0] / sizeof(uint8_t));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[0]);
    // 恢复默认行对齐
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // 上传UV分量（宽高各减半）
    int uvWidth = width / 2;
    int uvHeight = height / 2;

    // 上传U分量
    glBindTexture(GL_TEXTURE_2D, uTexture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[1] / sizeof(uint8_t));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uvWidth, uvHeight, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[1]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // 上传V分量
    glBindTexture(GL_TEXTURE_2D, vTexture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[2] / sizeof(uint8_t));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uvWidth, uvHeight, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[2]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // 这里自动调用 void FFGLRenderWidget::paintGL()
    update();
    doneCurrent();

    av_frame_unref(frame);
    av_frame_free(&frame);
}
