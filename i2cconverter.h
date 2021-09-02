#ifndef I2CCONVERTER_H
#define I2CCONVERTER_H

#include <QObject>
#include <wiringPiI2C.h>
#include <QThread>

/*
 * I2C to UART converter made on WK 2132 chip
 * based on https://github.com/DFRobot/DFRobot_IICSerial python/raspberrypi subfolder
 */

const int GENA = 0x00;
const int GRST = 0x01;
const int GMUT =  0x02;
const int GIER = 0x10;
const int GIFR = 0x11;

const int SPAGE = 0x03;
//PAGE 0
const int SCR = 0x04;
const int LCR = 0x05;
const int FCR = 0x06;
const int SIER = 0x07;
const int SIFR = 0x08;
const int TFCNT = 0x09;
const int RFCNT = 0x0A;
const int FSR = 0x0B;
const int LSR = 0x0C;
const int FDAT = 0x0D;
//PAGE 1
const int BAUD0 = 0x05;
const int BAUD1 = 0x04;
const int PRES = 0x06;
const int RFTL = 0x07;
const int TFTL = 0x08;

const int UART1 = 0x00;
const int UART2 = 0x01;

const int PAGE0 = 0;
const int PAGE1 = 1;

class I2cConverter:public QObject
{
    Q_OBJECT
private:
    int fd;
public:
    I2cConverter(int uart);

    void configReg(int reg, int val);
    void switchPage(int page);
    int available();
    QByteArray readAll();
    void write(const QByteArray& data);
};

class UartThread:public QThread
{
    Q_OBJECT
private:
    I2cConverter uart;

    bool threadState;
public:
    UartThread(int uartNum, QObject* parent=nullptr);
    void stop();
    QByteArray readAll();
protected:
    void run() override;
signals:
    void readyRead();
};

#endif // I2CCONVERTER_H
