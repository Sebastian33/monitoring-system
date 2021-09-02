#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QFile>
#include "buttonsthread.h"
#include "GroupBoxes.h"
#include <QWidget>
#include <QGridLayout>
#include <QCheckBox>
#include <QMap>
#include"i2cconverter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LayoutMenu: public QWidget
{
    Q_OBJECT
public:
    LayoutMenu(QWidget *parent = nullptr);
    ~LayoutMenu();
    void uncheckAll();

    QCheckBox *gpsCheckBox;
    QCheckBox *tmpOutsideCheckBox;
    QCheckBox *humidityCheckBox;
private:
    QGridLayout *layout;
};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void bindCloseButton(QApplication& a);
    ~MainWindow();

    enum BoxType
    {
        GPS,
        TMP_OUTSIDE,
        HUMIDITY
    };

private slots:
    void readSerialPort0();
    void readSerialPort1();
    void handleButtonPush();
    void handleButtonLongPush();
private:
    QSerialPort *serialPort0;
    UartThread *serialPort1;

    Ui::MainWindow *ui;
    QFile *file;
    ButtonsThread buttons;
    LayoutMenu *layoutMenu;
    QVector<QGridLayout*> tabLayouts;

    QByteArray buf0;
    QByteArray buf1;

    struct GPSData
    {
        float latitude;//широта
        char latDir;
        float longtitude;//долгота
        char longDir;
        int numOfSatelites;
    };
    GPSData gpsData{0.0, 'X', 0.0, 'Y', 0};

    struct THData // tmp and humidity
    {
        float tmp;
        char tmpUnit;
        float hum;
    };
    THData thData{0.0, 'C', 0.0};

    struct WData
    {
        int direction;
        char dirType;
        float speed;
        char speedUnit;
    };
    WData wData;

    QMap<int, GPSBox *> gpsMap;
    QMap<int, TmpOutsideBox *> tmpOutMap;
    QMap<int, HumidityBox *> humidityMap;
    QMap<BoxType, QWidget*> currentTabWidgets;

    void parseGPSData(const QString& rawData);
    void parseTHData(const QString& rawData);
    void parseWData(const QString& rawData);

    void handleUpButton();
    void handleDownButton();
    void handleMenuButton();
    void handleSetButton();
};
#endif // MAINWINDOW_H
