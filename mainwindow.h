#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QFile>
#include "buttonsthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void bindCloseButton(QApplication& a);
    ~MainWindow();
private slots:
    void handleReadyRead();
    void handleButtonPush();
    void handleButtonLongPush();
private:
    QSerialPort *serialPort;
    QByteArray readData;
    Ui::MainWindow *ui;
    QFile *file;
    ButtonsThread buttons;

    struct GPSData
    {
        float latitude;//широта
        char latDir;
        float longtitude;//долгота
        char longDir;
        int numOfSatelites;
    };

    GPSData gpsData{0.0, 'X', 0.0, 'Y', 0};

    void parseGPSData(const QString& rawData);

    void handleUpButton();
    void handleDownButton();
};
#endif // MAINWINDOW_H
