#include "mainwindow.h"

#include <QApplication>

extern "C" {
// 设备
#include <libavdevice/avdevice.h>
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;



    avdevice_register_all();
    w.show();
    return a.exec();
}
