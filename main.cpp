#include "mainwindow.h"
#include <QApplication>
#include <gpiod.h>
#include<QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.bindCloseButton(a);
    w.setWindowState(Qt::WindowMaximized);
//    w.show();

    w.showFullScreen();
    return a.exec();
}
