#include "ffvresampler.h"
#include"decoder/ffvdecoder.h"

FFVResampler::FFVResampler()
{

}

FFVResampler::~FFVResampler()
{
    if(swsCtx){
        sws_freeContext(swsCtx);
    }
    if(srcPars){
        delete srcPars;
        srcPars = nullptr;
    }
    if(dstPars){
        delete dstPars;
        srcPars = nullptr;
    }
    if(vBuffer){
        av_freep(&vBuffer);
        //av_free(&vBuffer);
        vBuffer = nullptr;
    }

}

void FFVResampler::init(FFVideoPars *srcPars_, FFVideoPars *dstPars_)
{
    srcPars = new FFVideoPars();
    memcpy(srcPars,srcPars_,sizeof(FFVideoPars));

    dstPars = new FFVideoPars();
    memcpy(dstPars,dstPars_,sizeof(FFVideoPars));

    std::cout<<"==========src video format==========" <<std::endl;
    std::cout<<"width:" << srcPars->width<< std::endl;
    std::cout<<"height:" << srcPars->height<< std::endl;
    std::cout<<"framerate:" << srcPars->frameRate.num << "/" << srcPars->frameRate.den << std::endl;
    std::cout<<"pixFmt:" << av_get_pix_fmt_name(srcPars->pixFmtEnum) << std::endl;

    std::cout<<"==========dst video format==========" <<std::endl;
    std::cout<<"width:" << dstPars->width<< std::endl;
    std::cout<<"height:" << dstPars->height<< std::endl;
    std::cout<<"framerate:" << dstPars->frameRate.num << "/" << dstPars->frameRate.den << std::endl;
    std::cout<<"pixFmt:" << av_get_pix_fmt_name(dstPars->pixFmtEnum) << std::endl;


    initSws();
}

void FFVResampler::resample(AVFrame *srcFrame, AVFrame **dstFrame)
{
    *dstFrame = allocFrame(dstPars,srcFrame);
    if(dstFrame == nullptr){
        return;
    }
    // 执行重采样
    /*
        vBuffer 就像仓库
           ↓
        frame->data[0] = *dstFrame->data[0] 就像仓库的大门钥匙，指向仓库里的 Y 区
           ↓
        sws_scale 拿着这把钥匙，打开大门，把 Y 数据搬进去
           ↓
        虽然 sws_scale 没直接看到 vBuffer，但它通过钥匙（frame->data）找到了仓库！
     */
    sws_scale(swsCtx,
              srcFrame->data,srcFrame->linesize,0,srcFrame->height,
              (*dstFrame)->data,(*dstFrame)->linesize);
}
// 分配视频帧
AVFrame* FFVResampler::allocFrame(FFVideoPars* vPars,AVFrame* srcFrame)
{
    AVFrame* frame = av_frame_alloc();
    // 计算图像缓冲区所需的大小
    // 大概算一下要点多少人，要点多少菜。只是计算需要多少内存，还没有分配
    int bufSize = av_image_get_buffer_size(vPars->pixFmtEnum,vPars->width,vPars->height,1);
    if(bufSize > maxbufSize){
        maxbufSize = bufSize;
        // vBuffer 视频缓冲区，用于存储重采样后的视频数据
        if(vBuffer){
            av_freep(&vBuffer);
        }
        // 点菜，真正的开始分配 bufSize 内存
        vBuffer = static_cast<uint8_t*>(av_mallocz(bufSize));
        if(!bufSize){
            av_frame_free(&frame);
            std::cerr << "malloc vBuffer error!" << std::endl;
            return nullptr;
        }

    }
    // 填充帧数据
    // 在空地上划三个区域：Y区、U区、V区
    // 给你一张地图（frame->data），告诉你每个区在哪里
    // 此时三个区还是空的
    // 告诉frame->data，YUV在vBuffer的哪里
    int ret = av_image_fill_arrays(frame->data,frame->linesize,vBuffer,
                                   vPars->pixFmtEnum,vPars->width,vPars->height,1);

    if(ret < 0){
        printError(ret);
        av_frame_free(&frame);
        return nullptr;
    }

    frame->width = vPars->width;
    frame->height = vPars->height;
    frame->format = vPars->pixFmtEnum;
    frame->pts = srcFrame->pts;

    return frame;
}

void FFVResampler::initSws()
{
    // [1] 创建并配置SwsContext
    // sws_getContext：创建视频缩放上下文
    // 参数：
    //   srcPars->width：源宽度（如1920）
    //   srcPars->height：源高度（如1080）
    //   srcPars->pixFmtEnum：源像素格式（如AV_PIX_FMT_YUV420P）
    //   dstPars->width：目标宽度（如1280）
    //   dstPars->height：目标高度（如720）
    //   dstPars->pixFmtEnum：目标像素格式（如AV_PIX_FMT_YUV420P）
    //   SWS_FAST_BILINEAR：缩放算法（快速双线性插值）
    //   nullptr：源滤镜（不使用）
    //   nullptr：目标滤镜（不使用）
    //   nullptr：参数（不使用）
    // 返回值：SwsContext指针
    // 缩放算法：
    //   SWS_FAST_BILINEAR：快速双线性插值（速度快，质量一般）
    //   SWS_BILINEAR：双线性插值（速度中等，质量中等）
    //   SWS_BICUBIC：双三次插值（速度慢，质量好）
    //   SWS_LANCZOS：Lanczos重采样（速度最慢，质量最好）

    // 缩放算法。SWS_FAST_BILINEAR快速双线性插值（速度快，质量一般）
    swsCtx = sws_getContext(srcPars->width,srcPars->height,srcPars->pixFmtEnum,
                            dstPars->width,dstPars->height,dstPars->pixFmtEnum
                            ,SWS_FAST_BILINEAR,nullptr,nullptr,nullptr);
    if(!swsCtx){
        std::cerr << "sws_getContext error!" << std::endl;
        return;
    }

}

void FFVResampler::printError(int ret)
{
    char errorBuffer[AV_ERROR_MAX_STRING_SIZE];
    // 将错误码转换为错误字符串
    int res = av_strerror(ret,errorBuffer,sizeof errorBuffer);
    if(res < 0){
        std::cerr << "Unknow Error!" << std::endl;
    }
    else{
        std::cerr << "Error:" << errorBuffer << std::endl;
    }
}
