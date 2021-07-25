#include "i2cconverter.h"

const int FIXED_ADR_PART = 0x70;

const int INTERVAL = 100;

I2cConverter::I2cConverter(int uart)
{
    fd = wiringPiI2CSetup(FIXED_ADR_PART);
    if(fd < 0)
    {
        qInfo("I2C setup failed");
        return;
    }
    //initial configuration
    int tmp; /*= wiringPiI2CReadReg8(fd, GENA);
    if((tmp & 0x80) == 0)
    {
        qInfo("failed to read GENA");
        return;
    }*/

    if(uart == UART1)
        tmp=0x01;
    else
        tmp=0x02;
    configReg(GENA, tmp);
    configReg(GRST, tmp);
    configReg(GIER, tmp);

    fd = wiringPiI2CSetup(FIXED_ADR_PART | (uart << 1));
    if(fd < 0)
    {
        qInfo("I2C setup failed");
        return;
    }

    switchPage(PAGE0);
    configReg(SIER, 0x8F);
    configReg(FCR, 0x0C);
    configReg(SCR, 0x03);
    //setting baudarate
    tmp = wiringPiI2CReadReg8(fd, SCR);
    configReg(SCR, 0);
    switchPage(PAGE1);
    //baudrate 115200
    configReg(BAUD0, 7);
    configReg(BAUD1, 0);
    configReg(PRES, 0);
    switchPage(PAGE0);
    configReg(SCR, tmp);
    //setting format and other features
    //no parity, 1 stop bit, 8 data bits
    tmp = wiringPiI2CReadReg8(fd, LCR);
    tmp &= 0xC0;
    wiringPiI2CWriteReg8(fd, LCR, tmp);
    wiringPiI2CReadReg8(fd, LCR);
}

void I2cConverter::configReg(int reg, int val)
{
    int current = wiringPiI2CReadReg8(fd, reg);
    val|=current;
    wiringPiI2CWriteReg8(fd, reg, val);
    wiringPiI2CReadReg8(fd, reg);
}

void I2cConverter::switchPage(int page)
{
    int val = wiringPiI2CReadReg8(fd, SPAGE);
    if(page==0)
        val&=0xFE;
    else if(page==1)
        val|=0x01;
    wiringPiI2CWriteReg8(fd, SPAGE, val);
    wiringPiI2CReadReg8(fd, SPAGE);
}

int I2cConverter::available()
{
    return wiringPiI2CReadReg8(fd, RFCNT);
}

QByteArray I2cConverter::readAll()
{
    int size = available();
    QByteArray res(size, 0);
    for(int i=0;i<size;i++)
        res[i] = wiringPiI2CReadReg8(fd, FDAT);
    return res;
}

void I2cConverter::write(const QByteArray &data)
{
    for(int d:data)
        wiringPiI2CWriteReg8(fd, FDAT, d);
}

UartThread::UartThread(int uartNum, QObject* parent):uart(uartNum), QThread(parent), threadState(true)
{}

void UartThread::stop()
{
    threadState = false;
}

QByteArray UartThread::read()
{
    return uart.readAll();
}

void UartThread::run()
{
    while(threadState)
    {
        QThread::msleep(INTERVAL);
        if(uart.available() > 0)
            emit readyRead();
    }
    exit();
}
