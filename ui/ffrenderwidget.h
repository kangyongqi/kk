#ifndef FFRenderWidget_H
#define FFRenderWidget_H
//主要用于视频捕获系统中显示摄像头、屏幕等视频源
// 可拖动、可缩放的音视频渲染控件
// 音视频渲染控件
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include<QFrame>
#include<QVBoxLayout>
#include<QHash>
#include<mutex>

#include"opengl/ffglrenderwidget.h"

class FFVFilter;
class FFCapWindow;

class FFRenderWidget : public QWidget
{
    Q_OBJECT

public:
    FFRenderWidget(FFVFilter* vFilter_,int type,QWidget *parent = nullptr);
    virtual ~FFRenderWidget() override;

    bool getMoveFlag();
    void setMoveFlag(bool flag);
    void setFilterPos(int x,int y,int w,int h);
    void getOverlayPos(int *x,int *y,int *w,int *h);
    void calcPos();

    void setStackNumber(int number);
    int getStackNumber();
    int getType();
public slots:
    void setYUVData(AVFrame*frame);
//    void setYUVData(uint8_t *yData, uint8_t *uData, uint8_t *vData, int width, int height);

public:
    double xRatio;
    double yRatio;
    double widthRatio;
    double heightRatio;
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent* event)override;

    void enterEvent(QEvent*event)override;
    void leaveEvent(QEvent*event)override;
private:
    int getResizeDirction(const QPoint &pos);
    FFGLRenderWidget* glRenderWidget = nullptr;

private:
    int type = 0;
    QPoint dragPos;
    QRect initialGeometry;
    int resizeDir;
    bool startFlag = true;

    bool isFoucs = false;
    bool moveFlag ;
    QVBoxLayout* layout = nullptr;

    FFVFilter* vFilter = nullptr;
    int overlayX;
    int overlayY;
    int overlayW;
    int overlayH;

    int stackNumber = 0;

    std::mutex mutex;
    FFCapWindow* capWindow = nullptr;

};
#endif // FFRenderWidget_H
