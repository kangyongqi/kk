#ifndef FFCapHeaderWidget_H
#define FFCapHeaderWidget_H
// 自定义窗口标题栏部件类
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui { class FFCapHeaderWidget; }
QT_END_NAMESPACE


class FFCapHeaderWidget : public QWidget
{
    Q_OBJECT

public:
    FFCapHeaderWidget(QWidget *parent = nullptr);
    ~FFCapHeaderWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    int getResizeDirction(const QPoint &pos);
private:
    // 用于记录窗口拖拽时的初始鼠标位置（相对于当前标题栏部件的局部坐标）
    QPoint dragPos;
    // 用于记录窗口开始拖拽/缩放时的初始几何信息（位置、大小）
    QRect initialGeometry;
    // 用于记录当前的窗口缩放方向
    int resizeDir;
};
#endif // FFCapHeaderWidget_H
