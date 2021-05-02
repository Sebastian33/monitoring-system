#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void handleReadyRead();
private:
    QSerialPort *serialPort;
    QByteArray readData;
    Ui::MainWindow *ui;
    QFile *file;

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
};
#endif // MAINWINDOW_H
