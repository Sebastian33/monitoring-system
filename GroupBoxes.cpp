#include "GroupBoxes.h"
const QString LAT = "Latitude: %1%2";
const QString LONG = "Longtitude: %1%2";
const QString SATS = "Satellites: %1";
const QString TMPOUT = "Tmp outside: %1%2";
const QString HUM = "Humidity: %1";
const QString WD = "Direction: %1 %2";
const QString WS = "Speed: %1 %2";

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

void GPSBox::update(const GPSData& data)
{
    latitude->setText(LAT.arg(data.latitude).arg(data.latDir));
    longtitude->setText(LONG.arg(data.longtitude).arg(data.longDir));
    numOfSats->setText(SATS.arg(data.numOfSatelites));
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

void TmpOutsideBox::update(const THData& data)
{
    temperature->setText(TMPOUT.arg(data.tmp).arg(data.tmpUnit));
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

void HumidityBox::update(const THData& data)
{
    humidity->setText(HUM.arg(data.hum));
}

WindBox::WindBox(QWidget* parent):
    GroupBox(parent),
    direction(new QLabel),
    speed(new QLabel)
{
    setTitle("Wind");
    direction->setText("kuda-to tuda");
    speed->setText("15 popugayev/sec");
    layout->addWidget(direction);
    layout->addWidget(speed);
}

WindBox::~WindBox()
{
    delete direction;
    delete speed;
}

void WindBox::update(const WData& data)
{
    direction->setText(WD.arg(data.direction).arg(data.dirType));
    speed->setText(WS.arg(data.speed).arg(data.speedUnit));
}
