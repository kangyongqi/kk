#include "ffcapheaderwidget.h"

#include<QPainter>
#include<QMouseEvent>
#include"ffcapwindow.h"

FFCapHeaderWidget::FFCapHeaderWidget(QWidget *parent) :
    QWidget(parent)
{
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setMouseTracking(true);
}

FFCapHeaderWidget::~FFCapHeaderWidget()
{

}

void FFCapHeaderWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setBrush(QBrush(QColor(32,32,32)));
    painter.fillRect(this->rect(), painter.brush());
}

void FFCapHeaderWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragPos = event->globalPos() - this->parentWidget()->frameGeometry().topLeft(); // 记录鼠标按下时相对窗口左上角的偏移
        initialGeometry = this->parentWidget()->geometry(); // 记录窗口初始几何信息
        resizeDir = getResizeDirction(event->pos());
        event->accept();
    }
}

void FFCapHeaderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        FFCapWindow* parentWidget = static_cast<FFCapWindow*>(this->parentWidget());
        if (resizeDir == NODIR && !parentWidget->getMaxSizeFlag()) {
            this->parentWidget()->move(event->globalPos() - dragPos); // 移动窗口
        }
        else {
            QPoint delta = event->globalPos() - (dragPos + initialGeometry.topLeft());
            // 计算鼠标相对于窗口初始位置的偏移
            QRect newGeometry = initialGeometry;
            switch (resizeDir) {
            case TOP:
                newGeometry.setTop(newGeometry.top() + delta.y());
                break;
            case LEFT:
                newGeometry.setLeft(newGeometry.left() + delta.x());
                break;
            case RIGHT:
                newGeometry.setRight(newGeometry.right() + delta.x());
                break;
            case TOP_LEFT:
                newGeometry.setLeft(newGeometry.left() + delta.x());
                newGeometry.setTop(newGeometry.top() + delta.y());
                break;
            case TOP_RIGHT:
                newGeometry.setRight(newGeometry.right() + delta.x());
                newGeometry.setTop(newGeometry.top() + delta.y());
                break;
            default:
                break;
            }
            bool canResize = true;
            QWidget* parentWidget = this->parentWidget();
            if (resizeDir == TOP && newGeometry.height() < parentWidget->minimumHeight()) {
                newGeometry.setTop(initialGeometry.top() + initialGeometry.height() - parentWidget->minimumHeight());
                canResize = false;
            }
            else if ((resizeDir == LEFT || resizeDir == RIGHT) && newGeometry.width() < parentWidget->minimumWidth()) {
                if (resizeDir == LEFT) {
                    newGeometry.setLeft(initialGeometry.right() - parentWidget->minimumWidth());
                }
                else {
                    newGeometry.setRight(initialGeometry.left() + parentWidget->minimumWidth());
                }
                canResize = false;
            }
            // 添加左上角缩放最小尺寸判断
            else if (resizeDir == TOP_LEFT) {
                if (newGeometry.width() < parentWidget->minimumWidth()) {
                    newGeometry.setLeft(initialGeometry.right() - parentWidget->minimumWidth());
                    canResize = false;
                }
                if (newGeometry.height() < parentWidget->minimumHeight()) {
                    newGeometry.setTop(initialGeometry.bottom() - parentWidget->minimumHeight());
                    canResize = false;
                }
            }
            // 添加右上角缩放最小尺寸判断
            else if (resizeDir == TOP_RIGHT) {
                if (newGeometry.width() < parentWidget->minimumWidth()) {
                    newGeometry.setRight(initialGeometry.left() + parentWidget->minimumWidth());
                    canResize = false;
                }
                if (newGeometry.height() < parentWidget->minimumHeight()) {
                    newGeometry.setTop(initialGeometry.bottom() - parentWidget->minimumHeight());
                    canResize = false;
                }
            }

            if (canResize) {
                parentWidget->setGeometry(newGeometry);
            }
        }
    }
    else {
        int dir= getResizeDirction(event->pos());
        switch (dir) {
        case TOP:
            setCursor(Qt::SizeVerCursor);
            break;
        case LEFT:
            setCursor(Qt::SizeHorCursor);
            break;
        case RIGHT:
            setCursor(Qt::SizeHorCursor);
            break;
        case TOP_LEFT:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case TOP_RIGHT:
            setCursor(Qt::SizeBDiagCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
        }
        QWidget::mouseMoveEvent(event);
    }
}

void FFCapHeaderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        resizeDir = NODIR;
        setCursor(Qt::ArrowCursor);
    }
}

int FFCapHeaderWidget::getResizeDirction(const QPoint &pos)
{
    enum ResizeDir ret = NODIR;
    if (pos.x() < BORDER_WIDTH) {
        ret = LEFT;
    }
    else if (pos.x() > width() - BORDER_WIDTH) {
        ret = RIGHT;
    }
    else if (pos.y() < BORDER_WIDTH) {
        ret = TOP;
    }

    if(pos.x() < BORDER_WIDTH && pos.y() < BORDER_WIDTH){
        ret = TOP_LEFT;
    }

    else if(pos.x() > width() - BORDER_WIDTH && pos.y() < BORDER_WIDTH){
        ret = TOP_RIGHT;
    }
    return ret;
}
