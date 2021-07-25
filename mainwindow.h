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
    void handleReadyRead();
    void handleButtonPush();
    void handleButtonLongPush();
private:
    QSerialPort *serialPort;
    QByteArray readData;
    Ui::MainWindow *ui;
    QFile *file;
    ButtonsThread buttons;
    LayoutMenu *layoutMenu;
    QVector<QGridLayout*> tabLayouts;

    struct GPSData
    {
        float latitude;//широта
        char latDir;
        float longtitude;//долгота
        char longDir;
        int numOfSatelites;
    };

    GPSData gpsData{0.0, 'X', 0.0, 'Y', 0};

    QMap<int, GPSBox *> gpsMap;
    QMap<int, TmpOutsideBox *> tmpOutMap;
    QMap<int, HumidityBox *> humidityMap;
    QMap<BoxType, QWidget*> currentTabWidgets;

    void parseGPSData(const QString& rawData);

    void handleUpButton();
    void handleDownButton();
    void handleMenuButton();
    void handleSetButton();
};
#endif // MAINWINDOW_H
