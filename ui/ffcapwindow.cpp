#include "ffcapwindow.h"
#include "ui_ffcapwindow.h"
#include "filter/ffvfilter.h"
#include"capture/ffcapturecontext.h"
#include"ffrenderwidget.h"
//#include"render./ffvrender.h"
#include"event/ffstartevent.h"
#include"queue/ffeventqueue.h"
#include"event/ffpauseevent.h"
#include"event/ffopensourceevent.h"
#include"event/ffclosesourceevent.h"
#include"event/ffstopevent.h"
#include"thread/ffopensourcethread.h"
#include"event/ffbeautyevent.h"
#include"event/ffvolumeevent.h"
#include"capture/ffcaptureutil.h"

using namespace FFCaptureContextType;
FFCapWindow::FFCapWindow(QWidget *parent):
    QWidget(parent),
    ui(new Ui::FFCapWindow)
{
    evQueue = &FFEventQueue::getInstance();

    ui->setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setMouseTracking(true);
    setMinimumSize(1000,800);
    resize(1000,800);
    overlayNumbers.resize(3,-1);
}

FFCapWindow::~FFCapWindow()
{
    delete ui;
}

bool FFCapWindow::getMaxSizeFlag()
{
    return maxSizeFlag;
}

void FFCapWindow::setVFilter(FFVFilter *vFilter)
{
    this->vFilter = vFilter;
}

//void FFCapWindow::setVRender(FFVRender *vRender_)
//{
//    vRender = vRender_;
//}
// 获取叠加窗口的位置和大小
// 被FFVFilterThread::overlayFrame调用
void FFCapWindow::getOverlayPos(int *x, int *y, int *w, int *h, int type)
{
    switch (type) {
    case CAMERA:
        getCameraOverlayPos(x,y,w,h);
        break;
    case SCREEN:
        getScreenOverlayPos(x,y,w,h);
        break;
    case VIDEO:
        getVideoOverlayPos(x,y,w,h);
        break;
    default:
        break;
    }
}

void FFCapWindow::getCameraOverlayPos(int *x, int *y, int *w, int *h)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(cameraWidget){
        cameraWidget->getOverlayPos(x,y,w,h);
    }
    else{
        *x = *y = *w = *h = 0;
    }
}

void FFCapWindow::getScreenOverlayPos(int *x, int *y, int *w, int *h)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(screenWidget){
        screenWidget->getOverlayPos(x,y,w,h);
    }
    else{
        *x = *y = *w = *h = 0;
    }
}

void FFCapWindow::getVideoOverlayPos(int *x, int *y, int *w, int *h)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(videoWidget){
        videoWidget->getOverlayPos(x,y,w,h);
    }
    else{
        *x = *y = *w = *h = 0;
    }
}

void FFCapWindow::changeUIState(QWidget *widget)
{
    widget->setEnabled(false);

    QTimer::singleShot(200,[widget](){
        widget->setEnabled(true);
    });
}

void FFCapWindow::setPauseUI(bool pause)
{
    static const QString captureBtnStyle[2] =
    {
        "QPushButton{background-image:url(:/image/start.svg);background-repeat:no-repeat;background-position:center; border:none;}"
        "QPushButton:hover{background-color:#2B2B2B;}",

        "QPushButton{background-image:url(:/image/pause.svg);background-repeat:no-repeat;background-position:center; border:none;}"
        "QPushButton:hover{background-color:#2B2B2B;}",
    };

    ui->startBtn->setStyleSheet(captureBtnStyle[pause]);

    changeUIState(ui->startBtn);
    changeUIState(ui->stopBtn);
}

void FFCapWindow::init(FFCaptureContext *captureCtx_)
{
    captureCtx = captureCtx_;
}

bool FFCapWindow::peekVideoReady()
{
//    return vRender->peekReadyFlag();
    return true;
}

void FFCapWindow::sendCameraFrame(AVFrame *frame)
{
    if(cameraWidget){
        cameraWidget->setYUVData(frame);
    }
    else{
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}

void FFCapWindow::sendVideoFrame(AVFrame *frame)
{
    if(videoWidget){
        videoWidget->setYUVData(frame);
    }
    else{
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}

void FFCapWindow::sendScreenFrame(AVFrame *frame)
{
    if(screenWidget){
        screenWidget->setYUVData(frame);
    }
    else{
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}

void FFCapWindow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setBrush(QBrush(QColor("#FF0000"))); // #272A33
    painter.fillRect(this->rect(), painter.brush());
}

void FFCapWindow::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton) {
//        dragPos = event->globalPos() - frameGeometry().topLeft(); // 记录鼠标按下时相对窗口左上角的偏移
        dragPos = event->globalPos();

        initialGeometry = this->geometry(); // 记录窗口初始几何信息
        resizeDir = getResizeDirction(event->pos());
        event->accept();
    }
}

void FFCapWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 需要过滤掉QComboBox，否则会瞬移
    QWidget *clickedWidget = childAt(event->pos());
    if (clickedWidget && clickedWidget->isWidgetType() && qobject_cast<QComboBox*>(clickedWidget)) {
        return; // 点击在 QComboBox 上，不执行窗口操作
    }

    if (event->buttons() & Qt::LeftButton && resizeDir != NODIR) {
//        QPoint delta = event->globalPos() - (dragPos + initialGeometry.topLeft());
        QPoint delta = event->globalPos() - dragPos;
        // 计算鼠标相对于窗口初始位置的偏移
        QRect newGeometry = initialGeometry;
        switch (resizeDir) {
        case TOP:
            newGeometry.setTop(newGeometry.top() + delta.y());
            break;
        case BOTTOM:
            newGeometry.setBottom(newGeometry.bottom() + delta.y());
            break;
        case LEFT:
            newGeometry.setLeft(newGeometry.left() + delta.x());
            break;
        case RIGHT:
            newGeometry.setRight(newGeometry.right() + delta.x());
            break;
        case BOTTOM_LEFT:
            newGeometry.setLeft(newGeometry.left() + delta.x());
            newGeometry.setBottom(newGeometry.bottom() + delta.y());
            break;
        case BOTTOM_RIGHT:
            newGeometry.setRight(newGeometry.right() + delta.x());
            newGeometry.setBottom(newGeometry.bottom() + delta.y());
            break;
        default:
            break;
        }

        bool canResize = true;
        if ((resizeDir == TOP || resizeDir == BOTTOM) && newGeometry.height() < this->minimumHeight()) {
            newGeometry.setTop(initialGeometry.top() + initialGeometry.height() - this->minimumHeight());
            canResize = false;
        }
        else if ((resizeDir == LEFT || resizeDir == RIGHT) && newGeometry.width() < this->minimumWidth()) {
            if (resizeDir == LEFT) {
                newGeometry.setLeft(initialGeometry.right() - this->minimumWidth());
            }
            else {
                newGeometry.setRight(initialGeometry.left() + this->minimumWidth());
            }
            canResize = false;
        }
        // 添加左下角缩放最小尺寸判断
        else if (resizeDir == BOTTOM_LEFT) {
            if (newGeometry.width() < this->minimumWidth()) {
                newGeometry.setLeft(initialGeometry.right() - this->minimumWidth());
                canResize = false;
            }
            if (newGeometry.height() < this->minimumHeight()) {
                newGeometry.setBottom(initialGeometry.top() + this->minimumHeight());
                canResize = false;
            }
        }
        // 添加右下角缩放最小尺寸判断
        else if (resizeDir == BOTTOM_RIGHT) {
            if (newGeometry.width() < this->minimumWidth()) {
                newGeometry.setRight(initialGeometry.left() + this->minimumWidth());
                canResize = false;
            }
            if (newGeometry.height() < this->minimumHeight()) {
                newGeometry.setBottom(initialGeometry.top() + this->minimumHeight());
                canResize = false;
            }
        }

        if (canResize) {
            this->setGeometry(newGeometry);
        }
    }
    else {
        ResizeDir dir = getResizeDirction(event->pos());
        switch (dir) {
        case TOP:
        case BOTTOM:
            setCursor(Qt::SizeVerCursor);
            break;
        case LEFT:
        case RIGHT:
            setCursor(Qt::SizeHorCursor);
            break;
        case BOTTOM_LEFT:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case BOTTOM_RIGHT:
            setCursor(Qt::SizeFDiagCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
        }
        QWidget::mouseMoveEvent(event);
    }
}

void FFCapWindow::mouseReleaseEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton) {
        resizeDir = NODIR;
        setCursor(Qt::ArrowCursor);
    }
}
void FFCapWindow::resizeEvent(QResizeEvent *event)
{

    for(auto pChildWidget : renderWidgetSet){
        if(pChildWidget != nullptr && pChildWidget->getMoveFlag()){
            calcChildWidget(pChildWidget);
            pChildWidget->setMoveFlag(false);
        }

        resizeChildWidget(pChildWidget);
    }
    QWidget::resizeEvent(event);
}

void FFCapWindow::on_closeBtn_clicked()
{
    this->close();
}

void FFCapWindow::on_maximizeBtn_clicked()
{

    for(auto pChildWidget : renderWidgetSet){
        if(pChildWidget == nullptr || !pChildWidget->getMoveFlag()){
            continue;
        }
        calcChildWidget(pChildWidget);
        resizeChildWidget(pChildWidget);

        pChildWidget->setMoveFlag(false);
    }

    static const QString maxmizeStyle[2] =
    {
        "QPushButton{background-image:url(:/image/maximize_1.svg);background-repeat:no-repeat;background-position:center; border:none;}"
        "QPushButton:hover{background-color:#2B2B2B;}",

        "QPushButton{background-image:url(:/image/maximize_0.svg);background-repeat:no-repeat;background-position:center; border:none;}"
        "QPushButton:hover{background-color:#2B2B2B;}",
    };
    static bool flag =false;
    if (flag) {
        this->showNormal();
        ui->maximizeBtn->setStyleSheet(maxmizeStyle[0]);
    }
    else {
        this->showMaximized();
        ui->maximizeBtn->setStyleSheet(maxmizeStyle[1]);
    }
    flag ^=1 ;
    maxSizeFlag = flag;

}
void FFCapWindow::on_minimizeBtn_clicked()
{
    showMinimized();
}

enum ResizeDir FFCapWindow::getResizeDirction(const QPoint &pos)
{
    enum ResizeDir ret = NODIR;
    // BORDER_WIDTH 边缘宽度阈值
    if (pos.x() < BORDER_WIDTH) {
        ret = LEFT;
    }
    else if (pos.x() > width() - BORDER_WIDTH) {
        ret = RIGHT;
    }
    else if (pos.y() < BORDER_WIDTH) {
        ret = TOP;
    }
    else if (pos.y() > height() - BORDER_WIDTH) {
        ret = BOTTOM;
    }

    if(pos.x() < BORDER_WIDTH && pos.y() > height() - BORDER_WIDTH){
        ret = BOTTOM_LEFT;
    }

    else if(pos.x() > width() - BORDER_WIDTH && pos.y() > height() - BORDER_WIDTH){
        ret = BOTTOM_RIGHT;
    }
    return ret;
}

void FFCapWindow::resizeChildWidget(FFRenderWidget *pChildWidget)
{
    QRect glRect = ui->GLRenderWidget->geometry();

    int newX = glRect.x() + glRect.width() * pChildWidget->xRatio;
    int newY = glRect.y() + glRect.height() * pChildWidget->yRatio;
    int newWidth = glRect.width() * pChildWidget->widthRatio;
    int newHeight = glRect.height() * pChildWidget->heightRatio;
    pChildWidget->setGeometry(newX, newY, newWidth, newHeight);
}

void FFCapWindow::calcChildWidget(FFRenderWidget *pChildWidget)
{
    QRect glRect = ui->GLRenderWidget->geometry();

    pChildWidget->xRatio = (pChildWidget->x() - glRect.x()) / (qreal)glRect.width();// qreal ：Qt中的浮点类型,确保除法运算得到浮点数结果，而不是整数除法,如果不转换 ：整数除法会截断小数部分，导致比例计算错误
    pChildWidget->yRatio = (pChildWidget->y() - glRect.y()) / (qreal)glRect.height();
    pChildWidget->widthRatio = pChildWidget->width() / (qreal)glRect.width();
    pChildWidget->heightRatio = pChildWidget->height() / (qreal)glRect.height();
}


void FFCapWindow::adjustStatckNumber(int type)
{
    overlayNumbers[0] = type;

    switch (type) {
    case VIDEO:
        if (cameraWidget && screenWidget) {
//            if (cameraWidget->getStackNumber() > screenWidget->getStackNumber()) {
//                setStackOrder(cameraWidget, screenWidget);  //2,3
//            }
//            else {
//                setStackOrder(screenWidget, cameraWidget);
//            }
            setStackOrder(screenWidget, cameraWidget);
        }
        else {
            setStackOrder(cameraWidget, screenWidget);
        }
        break;

    case CAMERA:
        if(videoWidget && screenWidget){
//            if(videoWidget->getStackNumber() > screenWidget->getStackNumber()){
//                setStackOrder(videoWidget, screenWidget);
//            }
//            else{
//                setStackOrder(screenWidget, videoWidget);
//            }
            setStackOrder(screenWidget, videoWidget);
        }
        else{
            setStackOrder(videoWidget, screenWidget);
        }
        break;

    case SCREEN:
        if(videoWidget && cameraWidget){
//            if(videoWidget->getStackNumber() > cameraWidget->getStackNumber()){
//                setStackOrder(videoWidget, cameraWidget);
//            }
//            else{
//                setStackOrder(cameraWidget, videoWidget);
//            }
            setStackOrder(cameraWidget, videoWidget);
        }
        else{
            setStackOrder(videoWidget, cameraWidget);
        }
        break;
    }
}

const std::vector<int>& FFCapWindow::getOverlayNumbers()
{
    return overlayNumbers;
}

void FFCapWindow::setCaptureProcessTime(int64_t seconds)
{
    int64_t hours = seconds / 3600;
    seconds %= 3600;

    int64_t minutes = seconds / 60;
    seconds %= 60;

    ui->captureProcessTimeLabel->setText(
                QString("%1:%2:%3")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'))
                );
}

void FFCapWindow::setStackOrder(FFRenderWidget *widget1, FFRenderWidget *widget2)
{
    if (widget1 && widget2) {
        widget1->setStackNumber(3);
        widget2->setStackNumber(2);
        overlayNumbers[2] = widget1->getType();
        overlayNumbers[1] = widget2->getType();
    }
    else if (widget1) {
        widget1->setStackNumber(2);
        overlayNumbers[2] = widget1->getType();
        overlayNumbers[1] = -1;
    }
    else if (widget2) {
        widget2->setStackNumber(2);
        overlayNumbers[2] = widget2->getType();
        overlayNumbers[1] = -1;
    }
}

void FFCapWindow::on_startBtn_clicked()
{
    pauseFlag ^= 1;
    if(pauseFlag){
        FFEvent* pauseEvent = new FFPauseEvent(captureCtx);
        evQueue->enqueue(pauseEvent);
    }
    else{
        std::string url,format;
        if(ui->captureRadioButton->isChecked()){
            url = ui->captureLineEdit->text().toStdString();
            format = "mp4";
        }
        else if(ui->liveRadioButton->isChecked()){
            url = ui->liveLineEdit->text().toStdString();
            if(url.substr(0,4) == "rtmp")
                format = "flv";
            else if(url.substr(0,4) == "rtsp"){
                format = "rtsp";
            }
        }

        if(!startFlag){
            FFEvent* startEvent = new FFStartEvent(captureCtx,url,format);
            evQueue->enqueue(startEvent);
        }
        else{
            FFEvent* pauseEvent = new FFPauseEvent(captureCtx);
            evQueue->enqueue(pauseEvent);
        }

    }

    startFlag = true;
    setPauseUI(pauseFlag);
}

void FFCapWindow::on_cameraCheckBox_toggled(bool checked)
{
    if(ui->cameraComboBox->currentIndex() == 0){
        return;
    }
    if(checked){
        if(!cameraWidget){
            cameraWidget = new FFRenderWidget(vFilter,CAMERA,ui->GLRenderWidget);
            cameraWidget->setVisible(true);
            renderWidgetSet.insert(cameraWidget);
            adjustStatckNumber(CAMERA);

            FFEvent* openSourceEv = nullptr;
            if(ui->cameraComboBox->currentIndex() == 1){
                openSourceEv = new FFOpenSourceEvent(captureCtx,CAMERA,FFCaptureURLS::CAMERA1_URL,"dshow");
            }
            else if(ui->cameraComboBox->currentIndex() == 2){
                openSourceEv = new FFOpenSourceEvent(captureCtx,CAMERA,FFCaptureURLS::CAMERA2_URL,"dshow");
            }
            evQueue->enqueue(openSourceEv);
        }
    }
    else{
        if(cameraWidget){
            auto iter = renderWidgetSet.find(cameraWidget);
            if(iter != renderWidgetSet.end()){
                renderWidgetSet.erase(iter);
            }
//            cameraWidget->setFilterPos(0,0,0,0);
            delete cameraWidget;
            cameraWidget = nullptr;

            FFEvent* closeSourceEv = new FFCloseSourceEvent(captureCtx,CAMERA);
            evQueue->enqueue(closeSourceEv);
            changeUIState(ui->cameraCheckBox);
            changeUIState(ui->cameraComboBox);
        }
    }
}

void FFCapWindow::on_cameraComboBox_currentIndexChanged(int index)
{
    if(!ui->cameraCheckBox->isChecked()){
        return;
    }
    if(index == 0){
        if(cameraWidget){
            auto iter = renderWidgetSet.find(cameraWidget);
            if(iter != renderWidgetSet.end()){
                renderWidgetSet.erase(iter);
            }
//            cameraWidget->setFilterPos(0,0,0,0);
            delete cameraWidget;
            cameraWidget = nullptr;

            FFEvent* closeSourceEv = new FFCloseSourceEvent(captureCtx,CAMERA);
            evQueue->enqueue(closeSourceEv);
            changeUIState(ui->cameraComboBox);
            changeUIState(ui->cameraCheckBox);
        }
    }
    else{
        if(!cameraWidget){
            cameraWidget = new FFRenderWidget(vFilter,CAMERA,ui->GLRenderWidget);
            //             cameraWidget->setFilterPos(0,0,cameraWidget->width(),cameraWidget->height());
            cameraWidget->setVisible(true);
            renderWidgetSet.insert(cameraWidget);
            adjustStatckNumber(CAMERA);

            FFEvent* openSourceEv = nullptr;
            if(ui->cameraComboBox->currentIndex() == 1){
                openSourceEv = new FFOpenSourceEvent(captureCtx,CAMERA,FFCaptureURLS::CAMERA1_URL,"dshow");
            }
            else if(ui->cameraComboBox->currentIndex() == 2){
                openSourceEv = new FFOpenSourceEvent(captureCtx,CAMERA,FFCaptureURLS::CAMERA2_URL,"dshow");
            }
            evQueue->enqueue(openSourceEv);

            changeUIState(ui->cameraComboBox);
            changeUIState(ui->cameraCheckBox);
        }
        else{
            //删除
            auto iter = renderWidgetSet.find(cameraWidget);
            if(iter != renderWidgetSet.end()){
                renderWidgetSet.erase(iter);
            }
//            cameraWidget->setFilterPos(0,0,0,0);

            auto cameraPos = cameraWidget->pos();
            delete cameraWidget;
            cameraWidget = nullptr;

            FFEvent* closeSourceEv = new FFCloseSourceEvent(captureCtx,CAMERA);
            evQueue->enqueue(closeSourceEv);

            //防止竞争
            //            QThread::msleep(50);

            //创建
            cameraWidget = new FFRenderWidget(vFilter,CAMERA,ui->GLRenderWidget);
            cameraWidget->move(cameraPos);
            cameraWidget->setVisible(true);
            renderWidgetSet.insert(cameraWidget);
            adjustStatckNumber(CAMERA);

            FFEvent* openSourceEv = nullptr;
            if(ui->cameraComboBox->currentIndex() == 1){
                openSourceEv = new FFOpenSourceEvent(captureCtx,CAMERA,FFCaptureURLS::CAMERA1_URL,"dshow");
            }
            else if(ui->cameraComboBox->currentIndex() == 2){
                openSourceEv = new FFOpenSourceEvent(captureCtx,CAMERA,FFCaptureURLS::CAMERA2_URL,"dshow");
            }
            evQueue->enqueue(openSourceEv);

            changeUIState(ui->cameraComboBox);
            changeUIState(ui->cameraCheckBox);
        }
    }
}
void FFCapWindow::on_screenCheckBox_toggled(bool checked)
{
    if(checked){
        if(screenWidget == nullptr){
            screenWidget = new FFRenderWidget(vFilter,SCREEN,ui->GLRenderWidget);
            screenWidget->setVisible(true);
            renderWidgetSet.insert(screenWidget);
            adjustStatckNumber(SCREEN);

            FFEvent* openSourceEv = new FFOpenSourceEvent(captureCtx,SCREEN,FFCaptureURLS::SCREEN_URL,"dshow");
            evQueue->enqueue(openSourceEv);
            changeUIState(ui->screenCheckBox);
        }
    }
    else{
        if(screenWidget){
            auto iter = renderWidgetSet.find(screenWidget);
            if(iter != renderWidgetSet.end()){
                renderWidgetSet.erase(iter);
            }
//            screenWidget->setFilterPos(0,0,0,0);
            delete screenWidget;
            screenWidget = nullptr;

            FFEvent* closeSourceEv = new FFCloseSourceEvent(captureCtx,SCREEN);
            evQueue->enqueue(closeSourceEv);
            changeUIState(ui->screenCheckBox);
        }
    }
}

void FFCapWindow::on_videoCheckBox_toggled(bool checked) // 通过命名约定自动连接信号和槽
{
    if(checked){
        QString filePath = QFileDialog::getOpenFileName(this,"选择视频文件",QDir::homePath(), "视频文件 (*.mp4 *.flv *.ts)");
        if (filePath.isEmpty()) {
            ui->videoCheckBox->setChecked(false);
        }
        else{
            if(videoWidget == nullptr){
                videoWidget = new FFRenderWidget(vFilter,VIDEO,ui->GLRenderWidget);
                videoWidget->setVisible(true);
                renderWidgetSet.insert(videoWidget);
                adjustStatckNumber(VIDEO);

                std::string format =  filePath.section('.',-1).toStdString();
                FFEvent* openSourceEv = new FFOpenSourceEvent(captureCtx,VIDEO,filePath.toStdString(),format);
                evQueue->enqueue(openSourceEv);
                changeUIState(ui->videoCheckBox);
            }
        }
    }
    else{
        if(videoWidget){
            auto iter = renderWidgetSet.find(videoWidget);
            // 说明找到了元素
            if(iter != renderWidgetSet.end()){
                renderWidgetSet.erase(iter);
            }
//            videoWidget->setFilterPos(0,0,0,0);
            delete videoWidget;
            videoWidget = nullptr;

            FFEvent* closeSourceEv = new FFCloseSourceEvent(captureCtx,VIDEO);
            evQueue->enqueue(closeSourceEv);
            changeUIState(ui->videoCheckBox);
        }
    }
}


void FFCapWindow::on_microphoneCheckBox_toggled(bool checked)
{
    if(checked){
        FFEvent* openSourceEv = new FFOpenSourceEvent(captureCtx,MICROPHONE,FFCaptureURLS::MICROPHONE_URL,"dshow");
        evQueue->enqueue(openSourceEv);
    }
    else{
        FFEvent* closeSourceEv = new FFCloseSourceEvent(captureCtx,MICROPHONE);
        evQueue->enqueue(closeSourceEv);
        changeUIState(ui->microphoneCheckBox);
    }
}

void FFCapWindow::on_audioCheckBox_toggled(bool checked)
{
    if(checked){
        FFEvent* openSourceEv = new FFOpenSourceEvent(captureCtx,AUDIO,FFCaptureURLS::AUDIO_URL,"dshow");
        evQueue->enqueue(openSourceEv);
    }
    else{
        FFEvent* closeSourceEv = new FFCloseSourceEvent(captureCtx,AUDIO);
        evQueue->enqueue(closeSourceEv);
        changeUIState(ui->audioCheckBox);
    }
}

void FFCapWindow::on_stopBtn_clicked()
{
    if(!startFlag){
        return;
    }

    pauseFlag = true;
    startFlag = false;
    setPauseUI(pauseFlag);
    ui->captureProcessTimeLabel->setText("00:00:00");

    FFEvent* stopEvent = new FFStopEvent(captureCtx);
    evQueue->enqueue(stopEvent);


}

void FFCapWindow::on_beautyCheckBox_toggled(bool checked)
{
    FFEvent* beautyEvent = nullptr;
    if(checked){
        int smoothValue = ui->smoothSlider->value();
        int whiteValue = ui->whiteSlider->value();

        beautyEvent = new FFBeautyEvent(captureCtx,smoothValue,whiteValue);
        evQueue->enqueue(beautyEvent);
    }
    else{
        beautyEvent = new FFBeautyEvent(captureCtx,0,0);
        evQueue->enqueue(beautyEvent);
    }
}

void FFCapWindow::on_microphoneVlomeSlider_valueChanged(int value)
{
    double volume = value * 1.0 / 100;

    FFEvent* volumeEvent = new FFVolumeEvent(captureCtx,volume,MICROPHONE);
    evQueue->enqueue(volumeEvent);
}

void FFCapWindow::on_smoothSlider_valueChanged(int value)
{
    if(!ui->beautyCheckBox->isChecked()){
        return;
    }
    int whiteValue = ui->whiteSlider->value();

    FFEvent* beautyEvent = new FFBeautyEvent(captureCtx,value,whiteValue);
    evQueue->enqueue(beautyEvent);
}

void FFCapWindow::on_whiteSlider_valueChanged(int value)
{
    if(!ui->beautyCheckBox->isChecked()){
        return;
    }
    int smoothValue = ui->smoothSlider->value();

    FFEvent* beautyEvent = new FFBeautyEvent(captureCtx,smoothValue,value);
    evQueue->enqueue(beautyEvent);
}

void FFCapWindow::on_audioVolumeSlider_valueChanged(int value)
{
    double volume = value * 1.0 / 100;

    FFEvent* volumeEvent = new FFVolumeEvent(captureCtx,volume,AUDIO);
    evQueue->enqueue(volumeEvent);
}
