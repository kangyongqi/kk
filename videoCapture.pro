#-------------------------------------------------
QT       += core gui
#QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = videoCapture
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS



CONFIG += c++14

SOURCES += \
    main.cpp \
    ui/ffcapwindow.cpp \
    queue/ffaframequeue.cpp \
    queue/ffapacketqueue.cpp \
    queue/ffvframequeue.cpp \
    queue/ffvpacketqueue.cpp \
    resampler/ffaresampler.cpp \
    resampler/ffvresampler.cpp \
    decoder/ffadecoder.cpp \
    decoder/ffvdecoder.cpp \
    encoder/ffaencoder.cpp \
    encoder/ffvencoder.cpp \
    demuxer/ffdemuxer.cpp \
    opengl/ffglrenderwidget.cpp\
    thread/ffadecoderthread.cpp\
    thread/ffvdecoderthread.cpp\
    thread/ffdemuxerthread.cpp \
    thread/ffthreadpool.cpp \
    thread/ffthread.cpp\
    event/ffevent.cpp \
    event/ffeventloop.cpp \
    queue/ffeventqueue.cpp \
#    render/ffvrender.cpp \
#    render/ffarender.cpp \
#    timer/fftimer.cpp \
    muxer/ffmuxer.cpp \
    thread/ffaencoderthread.cpp \
    thread/ffvencoderthread.cpp \
    thread/ffmuxerthread.cpp \
    filter/ffafilter.cpp \
    filter/ffvfilter.cpp \
    thread/ffvfilterthread.cpp \
    thread/ffafilterthread.cpp \
    ui/ffcapheaderwidget.cpp \
    opencv/fffacedetector.cpp \
    ui/ffrenderwidget.cpp \
    opencv/ffoverlayprocessor.cpp \
    opencv/ffvideoadapter.cpp \
    clock/ffglobalclock.cpp \
    event/ffstartevent.cpp \
    capture/ffcapturecontext.cpp \
    event/ffpauseevent.cpp \
    event/ffopensourceevent.cpp \
    thread/ffopensourcethread.cpp \
    event/ffclosesourceevent.cpp \
    event/ffstopevent.cpp \
    event/ffcaptureprocessevent.cpp \
    event/ffbeautyevent.cpp \
    event/ffvolumeevent.cpp \
    capture/ffcaptureutil.cpp


HEADERS += \
    ui/ffcapwindow.h \
    queue/ffaframequeue.h \
    queue/ffapacketqueue.h \
    queue/ffvframequeue.h \
    queue/ffvpacketqueue.h \
    resampler/ffaresampler.h \
    resampler/ffvresampler.h \
    decoder/ffadecoder.h \
    decoder/ffvdecoder.h \
    encoder/ffaencoder.h \
    encoder/ffvencoder.h \
    demuxer/ffdemuxer.h \
    opengl/ffglrenderwidget.h \
    thread/ffadecoderthread.h \
    thread/ffvdecoderthread.h \
    thread/ffdemuxerthread.h \
    thread/ffthreadpool.h \
    thread/ffthread.h\
    event/ffevent.h \
    event/ffeventloop.h \
    queue/ffeventqueue.h \
#    render/ffvrender.h \
#    render/ffarender.h \
#    timer/fftimer.h \
    muxer/ffmuxer.h \
    thread/ffaencoderthread.h \
    thread/ffvencoderthread.h \
    thread/ffmuxerthread.h \
    filter/ffafilter.h \
    filter/ffvfilter.h \
    thread/ffvfilterthread.h \
    thread/ffafilterthread.h \
    ui/ffcapheaderwidget.h \
    opencv/fffacedetector.h \
    ui/ffrenderwidget.h \
    opencv/ffoverlayprocessor.h \
    opencv/ffvideoadapter.h \
    clock/ffglobalclock.h \
    capture/ffcapturecontext.h \
    event/ffstartevent.h \
    event/ffpauseevent.h \
    event/ffopensourceevent.h \
    thread/ffopensourcethread.h \
    event/ffclosesourceevent.h \
    event/ffstopevent.h \
    event/ffcaptureprocessevent.h \
    event/ffbeautyevent.h \
    event/ffvolumeevent.h \
    capture/ffcaptureutil.h



FORMS += \
        ui/ffcapwindow.ui \


  LIBS+= -L$$PWD\3rdparty\ffmpeg-amf\lib

    LIBS+=$$PWD\3rdparty\opencv\lib\libopencv_*.a


    LIBS+= \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavformat  \
        -lavutil \
        -lswresample \
        -lswscale \

    LIBS += -lOpengl32 \
              -lglu32

INCLUDEPATH += $$PWD/3rdparty/opencv/include
INCLUDEPATH += $$PWD/3rdparty/ffmpeg-amf/include

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc



