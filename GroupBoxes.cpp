#include "GroupBoxes.h"

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
