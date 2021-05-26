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
        ui->LatitudeVal->setText(QString::number(gpsData.latitude) + gpsData.latDir);
        ui->LongtitudeVal->setText(QString::number(gpsData.longtitude) + gpsData.longDir);
        ui->NumOfSatsVal->setText(QString::number(gpsData.numOfSatelites));
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
    buttons.stop();
    QTextStream out(file);
    out<<"that's all, folks!1!\n";
    file->close();
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
            break;
        }
    case CONFIG_CODE:
        {
            break;
        }
    case SET_CODE:
        {
            break;
        }
    case CANCEL_CODE:
        {
            break;
        }
    }

}

void MainWindow::handleButtonLongPush()
{
    qInfo("long push %d", buttons.getCode());
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
