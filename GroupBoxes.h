#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

#ifndef GROUPBOXES_H
#define GROUPBOXES_H
struct GPSData
{
    float latitude;//широта
    char latDir;
    float longtitude;//долгота
    char longDir;
    int numOfSatelites;
};

struct THData // tmp and humidity
{
    float tmp;
    char tmpUnit;
    float hum;
};

struct WData
{
    int direction;
    char dirType;
    float speed;
    char speedUnit;
};

class GroupBox:public QGroupBox
{
    Q_OBJECT
public:
    GroupBox(QWidget *parent = nullptr);
    ~GroupBox();
protected:
    QVBoxLayout *layout;
};

class GPSBox:public GroupBox
{
    Q_OBJECT
public:
    GPSBox(QWidget *parent = nullptr);
    ~GPSBox();
    void update(const GPSData& data);
private:
    QLabel *latitude;
    QLabel *longtitude;
    QLabel *numOfSats;
};

class TmpOutsideBox:public GroupBox
{
    Q_OBJECT
public:
    TmpOutsideBox(QWidget *parent = nullptr);
    ~TmpOutsideBox();
    void update(const THData& data);
private:
    QLabel *temperature;
};

class HumidityBox:public GroupBox
{
    Q_OBJECT
public:
    HumidityBox(QWidget *parent = nullptr);
    ~HumidityBox();
    void update(const THData& data);
private:
    QLabel *humidity;
};

class WindBox: public GroupBox
{
    Q_OBJECT
public:
    WindBox(QWidget* parent = nullptr);
    ~WindBox();
    void update(const WData& data);
private:
    QLabel *direction;
    QLabel *speed;
};

#endif // GROUPBOXES_H
