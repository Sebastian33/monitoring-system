#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include"i2cconverter.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.bindCloseButton(a);

    w.showFullScreen();
    return a.exec();
}
