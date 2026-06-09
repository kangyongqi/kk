#include "ui/ffcapwindow.h"
#include <QApplication>
#include<QIcon>
#include"capture/ffcaptureutil.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QIcon icon(":/image/icon.svg");
    a.setWindowIcon(icon);

    FFCaptureUtil captureUtil;
    captureUtil.initialize();
    captureUtil.startCapture();

    return a.exec();
}
