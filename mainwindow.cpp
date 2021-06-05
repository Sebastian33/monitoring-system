#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>

const char *GPSRD = "+GPSRD";
const char *GNVTG = "GNVTG";
const char *GPGSV = "GPGSV";
const char *GNGGA = "GNGGA";

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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , serialPort(new QSerialPort)
    , ui(new Ui::MainWindow)
    , buttons(parent)
    , layoutMenu(new SubWindow(parent))
{
    serialPort->setPortName("/dev/ttyS0");
    serialPort->setBaudRate(QSerialPort::Baud115200);
    if(serialPort->open(QIODevice::ReadWrite))
    {
        connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::handleReadyRead);
        serialPort->write("AT+GPS=1\r\n");
        serialPort->write("AT+GPSRD=5\r\n");
    }
    else
    {
        qInfo("Couldn't open the port\n");
        return;
    }

    file = new QFile("out");
    file->open(QIODevice::WriteOnly|QIODevice::Text);

    if(buttons.getCode() & ERROR_CODE)
    {
        return;
    }
    connect(&buttons, &ButtonsThread::push, this, &MainWindow::handleButtonPush);
    connect(&buttons, &ButtonsThread::longPush, this, &MainWindow::handleButtonLongPush);
    buttons.start();
    ui->setupUi(this);

    QGridLayout* tmp;
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        tmp = new QGridLayout;
        tabLayouts.push_back(tmp);
        ui->tabWidget->widget(i)->setLayout(tmp);
    }
}

void MainWindow::handleReadyRead()
{
    if(readData.size()==0)
        readData=serialPort->readAll();
    else
        readData.append(serialPort->readAll());
    if(readData[readData.size()-1]!='\n')
        return;

    if(qstrncmp(readData, GPSRD, qstrlen(GPSRD)) == 0)
    {
        if(!readData.contains(GNVTG)) //GNVTG is the last line
            return;
        QString rawData = QString(readData).remove(0, 7); // remove +GPRSD
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
    out<<readData;
    out<<"**********\n\n";
    readData.clear();
}

MainWindow::~MainWindow()
{
    if(serialPort->isOpen())
    {
        serialPort->write("AT+GPS=0\r\n");
        serialPort->close();
    }
    delete serialPort;
    buttons.stop();

    for(QWidget* w: gpsMap.values())
        delete w;
    for(QWidget* w: tmpOutMap.values())
        delete w;
    for(QWidget* w: humidityMap.values())
        delete w;

//    QTextStream out(file);
//    out<<"that's all, folks!1!\n";
//    file->close();

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
        if(tmp.startsWith(GPGSV))
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

void MainWindow::handleButtonPush()
{
    //qInfo("push %d", buttons.getCode());
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
    //qInfo("long push %d", buttons.getCode());
}

void MainWindow::bindCloseButton(QApplication& a)
{
    connect(ui->closeButton, &QAbstractButton::clicked, &a, &QCoreApplication::quit);
}

void MainWindow::handleUpButton()
{
    int idx = ui->tabWidget->currentIndex() - 1;
    idx = idx < 0 ? ui->tabWidget->count() - 1 : idx;
    //qInfo("up %d->%d", ui->tabWidget->currentIndex(), idx);
    ui->tabWidget->setCurrentIndex(idx);
}

void MainWindow::handleDownButton()
{
    int idx = ui->tabWidget->currentIndex() + 1;
    idx = idx==ui->tabWidget->count() ? 0 : idx;
    //qInfo("down %d->%d", ui->tabWidget->currentIndex(), idx);
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

SubWindow::SubWindow(QWidget *parent):
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

SubWindow::~SubWindow()
{
    delete gpsCheckBox;
    delete tmpOutsideCheckBox;
    delete humidityCheckBox;
    delete layout;
}

void SubWindow::uncheckAll()
{
    gpsCheckBox->setChecked(false);
    tmpOutsideCheckBox->setChecked(false);
    humidityCheckBox->setChecked(false);
}

GroupBox::GroupBox(QWidget *parent):
    QGroupBox(parent),
    layout(new QVBoxLayout)
{
    setLayout(layout);
    setStyleSheet("QGroupBox {border: 1px solid blue}");
}

GroupBox::~GroupBox()
{
    delete layout;
}

GPSBox::GPSBox(QWidget* parent):
    GroupBox(parent),
    latitude(new QLabel),
    longtitude(new QLabel),
    numOfSats(new QLabel)
{
    setTitle("GPS");
    latitude->setText("there are no");
    longtitude->setText("easter eggs here,");
    numOfSats->setText("go away");
    layout->addWidget(latitude);
    layout->addWidget(longtitude);
    layout->addWidget(numOfSats);
}

GPSBox::~GPSBox()
{
    delete longtitude;
    delete latitude;
    delete numOfSats;
}

TmpOutsideBox::TmpOutsideBox(QWidget* parent):
    GroupBox(parent),
    temperature(new QLabel)
{
    setTitle("Temperature outside");
    temperature->setText("+/- 30");
    layout->addWidget(temperature);
}

TmpOutsideBox::~TmpOutsideBox()
{
    delete temperature;
}

HumidityBox::HumidityBox(QWidget* parent):
    GroupBox(parent),
    humidity(new QLabel)
{
    setTitle("Humidity");
    humidity->setText("-10.5%");
    layout->addWidget(humidity);
}

HumidityBox::~HumidityBox()
{
    delete humidity;
}
