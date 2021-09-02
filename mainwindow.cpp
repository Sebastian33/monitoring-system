#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>

const char *GPSRD = "+GPSRD";
const char *GNVTG = "GNVTG";
const char *GPGSV = "GPGSV";
const char *GNGGA = "GNGGA";

const char *PBMSA = "$PBMSA";
const char *WIMWD = "$WIMWD";

bool checksum(const QString& str, unsigned char sum)
{
    for(unsigned char c: str.toLatin1())
    {
        sum ^= c;
    }
    return sum == 0;
}

void processSatellites(const QString& line, QMap<QString, QString>& data)
{
    int coma = line.indexOf(',');// skip GPGSV
    coma = line.indexOf(',', coma+1);//skip total number of lines
    coma = line.indexOf(',', coma+1);//skip current line number
    coma = line.indexOf(',', coma+1);//skip number of satellites
    if(coma==-1)
        return;
    else if(!data.contains(GPGSV))
        data[GPGSV] = "";
    data[GPGSV] += line.right(line.size()-coma-1) + ',';
}


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    serialPort0(new QSerialPort),
    serialPort1(new UartThread(UART1, parent)),
    ui(new Ui::MainWindow),
    buttons(parent),
    layoutMenu(new LayoutMenu(parent))
{
    //built-in serial port config and GPS module initialization
//    serialPort0->setPortName("/dev/ttyS0");
//    serialPort0->setBaudRate(QSerialPort::Baud115200);
//    if(serialPort0->open(QIODevice::ReadWrite))
//    {
//        connect(serialPort0, &QSerialPort::readyRead, this, &MainWindow::readSerialPort0);
//        serialPort0->write("AT+GPS=1\r\n"); //turn GPS on
//        serialPort0->write("AT+GPSRD=5\r\n"); //send data every 5 seconds
//    }
//    else
//    {
//        qInfo("Couldn't open the port\n");
//        return;
//    }

    //additional serial ports config
    connect(serialPort1, &UartThread::readyRead, this, &MainWindow::readSerialPort1);
    serialPort1->start();

    //"logging"
    file = new QFile("out");
    file->open(QIODevice::WriteOnly|QIODevice::Text);

    //external buttons config
    if(buttons.getCode() & ERROR_CODE)
    {
        return;
    }
    connect(&buttons, &ButtonsThread::push, this, &MainWindow::handleButtonPush);
    connect(&buttons, &ButtonsThread::longPush, this, &MainWindow::handleButtonLongPush);
    buttons.start();

    //ui initialization
    ui->setupUi(this);
    QGridLayout* tmp;
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        tmp = new QGridLayout;
        tabLayouts.push_back(tmp);
        ui->tabWidget->widget(i)->setLayout(tmp);
    }
}

void MainWindow::readSerialPort0()
{
    if(buf0.size()==0)
        buf0=serialPort0->readAll();
    else
        buf0.append(serialPort0->readAll());
    if(buf0[buf0.size()-1]!='\n')
        return;

    if(qstrncmp(buf0, GPSRD, qstrlen(GPSRD)) == 0)
    {
        if(!buf0.contains(GNVTG)) //GNVTG is the last line
            return;
        QString rawData = QString(buf0).remove(0, 7); // remove +GPRSD
        parseGPSData(rawData);

//        ui->LatitudeVal->setText(QString::number(gpsData.latitude) + gpsData.latDir);
//        ui->LongtitudeVal->setText(QString::number(gpsData.longtitude) + gpsData.longDir);
//        ui->NumOfSatsVal->setText(QString::number(gpsData.numOfSatelites));

//        qInfo("latitude: %f %c", gpsData.latitude, gpsData.latDir );
//        qInfo("longtitude: %f %c", gpsData.longtitude, gpsData.longDir );
//        qInfo("number of satellites: %i", gpsData.numOfSatelites);
//        qInfo("**************");
//        out<<"latitude: "<< gpsData.latitude<<gpsData.latDir<<endl;
//        out<< "longtitude: "<<gpsData.longtitude<<gpsData.longDir<<endl;
//        out<<"number of satellites: " <<gpsData.numOfSatelites<<endl;
    }
    QTextStream out(file);
    out<<"***raw data***"<<endl;
    out<<buf0;
    out<<"**********\n\n";
    buf0.clear();
}

void MainWindow::readSerialPort1()
{
    if(buf1.size()==0)
        buf1=serialPort1->readAll();
    else
        buf1.append(serialPort1->readAll());

    if(buf1[buf1.size()-1]!='\n')
        return;

    QStringList rawDataLines = QString(buf1).split("\r\n");
    buf1.clear();

    QStringList splitted;
    for(QString &line: rawDataLines)
    {
        QString prefix = line.left(6); //$abcde
        if(prefix==PBMSA)
        {
            parseTHData(line);
            qInfo()<<line;
            qInfo("tmp: %f%c, %f",thData.tmp, thData.tmpUnit, thData.hum);
        }
        else if(prefix==WIMWD)
        {
            parseWData(line);
            qInfo()<<line;
            qInfo("wind: %i%c, %f%c", wData.direction, wData.dirType, wData.speed, wData.speedUnit);
        }
    }
}

MainWindow::~MainWindow()
{
    if(serialPort0->isOpen())
    {
        serialPort0->write("AT+GPS=0\r\n");
        serialPort0->close();
    }
    delete serialPort0;
    serialPort1->stop();
    delete serialPort1;
    buttons.stop();


    for(QWidget* w: gpsMap.values())
        delete w;
    for(QWidget* w: tmpOutMap.values())
        delete w;
    for(QWidget* w: humidityMap.values())
        delete w;

    for(auto l: tabLayouts)
        delete l;
    delete layoutMenu;
    delete ui;
}

void MainWindow::parseGPSData(const QString& rawData)
{
    QTextStream out(file);
    QStringList preparsed = rawData.split("\r\n");
    QMap<QString, QString> data;
    unsigned char sum;
    QString tmp;
    for(int i = 0; i < preparsed.size(); i++)
    {
        if(preparsed[i].size()==0)
            continue;
        tmp = preparsed[i].remove(0, 1);//remove '$'

        if(tmp.size()>3 && tmp[tmp.size()-3]!='*') //invalid line
            continue;
        sum = tmp.right(2).toShort(nullptr, 16);
        tmp = tmp.left(tmp.size()-3);
        if(!checksum(tmp, sum))
            continue;
        if(tmp.startsWith(GPGSV))//there can be several GPGSV lines
            processSatellites(tmp, data);
        else
            data[tmp.left(5)] = tmp.remove(0,6);
    }
    if(data.contains(GPGSV))
        out<<"sats: "<< data[GPGSV]<<endl;

    QStringList parsed = data[GNGGA].split(',');
    gpsData.latitude = parsed[1].toFloat();
    gpsData.latDir = parsed[2].toLatin1()[0];
    gpsData.longtitude = parsed[3].toFloat();
    gpsData.longDir = parsed[4].toLatin1()[0];

    parsed = data[GPGSV].split(',');
    gpsData.numOfSatelites = parsed.size()/4;
}

void MainWindow::parseTHData(const QString& rawData)
{
    QStringList preparsed = rawData.split(',');
    if(preparsed.size()!=5 || preparsed[4].toLatin1()[0]!='A') //A - data is valid
        return;
    //checksum should be verified here but there is no checksum yet

    thData.tmp = preparsed[1].toFloat();
    thData.tmpUnit = preparsed[2].toLatin1()[0];
    thData.hum = preparsed[3].toFloat();
}

void MainWindow::parseWData(const QString& rawData)
{
    QStringList preparsed = rawData.split(',');
    if(preparsed.size()!=6 || preparsed[5].toLatin1()[0]!='A') //A - data is valid
        return;
    //checksum should be verified here but there is no checksum yet

    wData.direction = preparsed[1].toInt();
    wData.dirType = preparsed[2].toLatin1()[0];
    wData.speed = preparsed[3].toFloat();
    wData.speedUnit = preparsed[4].toLatin1()[0];
}

void MainWindow::handleButtonPush()
{
    switch(buttons.getCode())
    {
    case UP_CODE:
        {
            handleUpButton();
            break;
        }
    case DOWN_CODE:
        {
            handleDownButton();
            break;
        }
    case MENU_CODE:
        {
            handleMenuButton();
            break;
        }
    case CONFIG_CODE:
        {
            break;
        }
    case SET_CODE:
        {
            handleSetButton();
            break;
        }
    case CANCEL_CODE:
        {
            layoutMenu->hide();
            break;
        }
    }

}

void MainWindow::handleButtonLongPush()
{
    //not implemented yet
}

void MainWindow::bindCloseButton(QApplication& a)
{
    connect(ui->closeButton, &QAbstractButton::clicked, &a, &QCoreApplication::quit);
}

void MainWindow::handleUpButton()
{
    int idx = ui->tabWidget->currentIndex() - 1;
    idx = idx < 0 ? ui->tabWidget->count() - 1 : idx;
    ui->tabWidget->setCurrentIndex(idx);
}

void MainWindow::handleDownButton()
{
    int idx = ui->tabWidget->currentIndex() + 1;
    idx = idx==ui->tabWidget->count() ? 0 : idx;
    ui->tabWidget->setCurrentIndex(idx);
}

void MainWindow::handleMenuButton()
{
    if(layoutMenu->isVisible())
        return;
    layoutMenu->uncheckAll();
    int currentTab = ui->tabWidget->currentIndex();
    currentTabWidgets.clear();

    if(gpsMap.contains(currentTab))
    {
        layoutMenu->gpsCheckBox->setChecked(true);
        currentTabWidgets.insert(GPS, gpsMap[currentTab]);
    }
    if(tmpOutMap.contains(currentTab))
    {
        layoutMenu->tmpOutsideCheckBox->setChecked(true);
        currentTabWidgets.insert(TMP_OUTSIDE, tmpOutMap[currentTab]);
    }
    if(humidityMap.contains(currentTab))
    {
        layoutMenu->humidityCheckBox->setChecked(true);
        currentTabWidgets.insert(HUMIDITY, humidityMap[currentTab]);
    }
    layoutMenu->show();
}

void MainWindow::handleSetButton()
{
    QTextStream out(file);
    if(layoutMenu->isHidden())
    {
        out<<"hidden "<<layoutMenu->isHidden()<<endl;
        return;
    }
    int currentTab = ui->tabWidget->currentIndex();
    QGridLayout* lt = dynamic_cast<QGridLayout*>(ui->tabWidget->widget(currentTab)->layout());
    for(QWidget* gb: currentTabWidgets.values())
    {
        out<<"removing "<<gb<<endl;
        lt->removeWidget(gb);
        gb->close();
    }
    QWidget* widget;
    if(currentTabWidgets.contains(GPS) && (!layoutMenu->gpsCheckBox->isChecked()))
    {
        gpsMap.remove(currentTab);
        widget = currentTabWidgets[GPS];
        currentTabWidgets.remove(GPS);
        delete widget;
    }
    else if((!currentTabWidgets.contains(GPS)) && layoutMenu->gpsCheckBox->isChecked())
    {
        gpsMap.insert(currentTab, new GPSBox);
        currentTabWidgets.insert(GPS, gpsMap[currentTab]);
    }
    if(currentTabWidgets.contains(TMP_OUTSIDE) && (!layoutMenu->tmpOutsideCheckBox->isChecked()))
    {
        tmpOutMap.remove(currentTab);
        widget = currentTabWidgets[TMP_OUTSIDE];
        currentTabWidgets.remove(TMP_OUTSIDE);
        delete widget;
    }
    else if((!currentTabWidgets.contains(TMP_OUTSIDE)) && layoutMenu->tmpOutsideCheckBox->isChecked())
    {
        tmpOutMap.insert(currentTab, new TmpOutsideBox);
        currentTabWidgets.insert(TMP_OUTSIDE, tmpOutMap[currentTab]);
    }
    if(currentTabWidgets.contains(HUMIDITY) && (!layoutMenu->humidityCheckBox->isChecked()))
    {
        humidityMap.remove(currentTab);
        widget = currentTabWidgets[HUMIDITY];
        currentTabWidgets.remove(HUMIDITY);
        delete widget;
    }
    else if((!currentTabWidgets.contains(HUMIDITY)) && layoutMenu->humidityCheckBox->isChecked())
    {
        humidityMap.insert(currentTab, new HumidityBox);
        currentTabWidgets.insert(HUMIDITY, humidityMap[currentTab]);
    }
    QList<QWidget*> tmp = currentTabWidgets.values();
    int i=0;
    while(i<4 && i<tmp.size())
    {
        out<<"adding "<<tmp[i]<<endl;
        lt->addWidget(tmp[i], (i>>1)&1, i&1);
        tmp[i]->show();
        i++;
    }
    layoutMenu->hide();
}

LayoutMenu::LayoutMenu(QWidget *parent):
    QWidget(parent),
    gpsCheckBox(new QCheckBox()),
    tmpOutsideCheckBox(new QCheckBox()),
    humidityCheckBox(new QCheckBox()),
    layout(new QGridLayout())
{
    setWindowFlag(Qt::WindowType::SubWindow);
    setLayout(layout);
    gpsCheckBox->setText("GPS");
    tmpOutsideCheckBox->setText("Temperature outside");
    humidityCheckBox->setText("Humidity");
    layout->addWidget(gpsCheckBox, 0, 0);
    layout->addWidget(tmpOutsideCheckBox, 0, 1);
    layout->addWidget(humidityCheckBox, 1, 0);
}

LayoutMenu::~LayoutMenu()
{
    delete gpsCheckBox;
    delete tmpOutsideCheckBox;
    delete humidityCheckBox;
    delete layout;
}

void LayoutMenu::uncheckAll()
{
    gpsCheckBox->setChecked(false);
    tmpOutsideCheckBox->setChecked(false);
    humidityCheckBox->setChecked(false);
}
