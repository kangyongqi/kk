#ifndef FFCAPWINDOW_H
#define FFCAPWINDOW_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include<QSet>
#include<QBoxLayout>
#include<QFileDialog>
#include<mutex>
#include<QTimer>
#include<QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class FFCapWindow; }
QT_END_NAMESPACE

#ifndef RESIZE_DIR
#define RESIZE_DIR
enum ResizeDir {
    NODIR,
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    TOP_LEFT,
    TOP_RIGHT
};
#endif

const int BORDER_WIDTH = 5;

class AVFrame;
class FFRenderWidget;
class FFVFilter;
//class FFVRender;
class FFCaptureContext;
class FFEventQueue;

class FFCapWindow : public QWidget
{
    Q_OBJECT

public:
    FFCapWindow(QWidget *parent = nullptr);
    virtual ~FFCapWindow() override;

    bool getMaxSizeFlag();
    void setVFilter(FFVFilter* vFilter);
//    void setVRender(FFVRender* vRender_);

    void init(FFCaptureContext* captureCtx_);
    void adjustStatckNumber(int type);

public slots:
    void sendScreenFrame(AVFrame*frame);
    void sendCameraFrame(AVFrame* frame);
    void sendVideoFrame(AVFrame* frame);
    bool peekVideoReady();
    void getOverlayPos(int *x,int *y,int *w,int *h,int type);
    const std::vector<int>& getOverlayNumbers();
    void setCaptureProcessTime(int64_t seconds);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event)override;
private slots:
    void on_closeBtn_clicked();
    void on_maximizeBtn_clicked();
    void on_minimizeBtn_clicked();

    void on_startBtn_clicked();
    void on_cameraCheckBox_toggled(bool checked);

    void on_cameraComboBox_currentIndexChanged(int index);

    void on_screenCheckBox_toggled(bool checked);

    void on_videoCheckBox_toggled(bool checked);
    void on_microphoneCheckBox_toggled(bool checked);

    void on_audioCheckBox_toggled(bool checked);

    void on_stopBtn_clicked();

    void on_beautyCheckBox_toggled(bool checked);

    void on_microphoneVlomeSlider_valueChanged(int value);

    void on_smoothSlider_valueChanged(int value);

    void on_whiteSlider_valueChanged(int value);

    void on_audioVolumeSlider_valueChanged(int value);

private:
    enum ResizeDir getResizeDirction(const QPoint &pos);
    void resizeChildWidget(FFRenderWidget* pChildWidget);
    void calcChildWidget(FFRenderWidget* pChildWidget);
    void setStackOrder(FFRenderWidget* widget1,FFRenderWidget* widget2);
    void getCameraOverlayPos(int *x,int *y,int *w,int *h);
    void getScreenOverlayPos(int *x,int *y,int *w,int *h);
    void getVideoOverlayPos(int *x,int *y,int *w,int *h);
    void changeUIState(QWidget *widget);

    void setPauseUI(bool pause);
private:
    Ui::FFCapWindow *ui;
    QPoint dragPos;
    QRect initialGeometry;
    enum ResizeDir resizeDir;
    bool pauseFlag = true;
    FFRenderWidget* cameraWidget = nullptr;
    FFRenderWidget* screenWidget = nullptr;
    FFRenderWidget* videoWidget = nullptr;
//    FFVRender* vRender = nullptr;

    bool maxSizeFlag = false;

    FFVFilter* vFilter = nullptr;

    QSet<FFRenderWidget*>renderWidgetSet;
    FFCaptureContext* captureCtx = nullptr;
    FFEventQueue* evQueue = nullptr;

    std::mutex mutex;
    std::vector<int>overlayNumbers;

    bool startFlag = false;

};
#endif // FFCAPWINDOW_H
