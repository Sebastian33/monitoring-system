#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

#ifndef GROUPBOXES_H
#define GROUPBOXES_H
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
private:
    QLabel *temperature;
};

class HumidityBox:public GroupBox
{
    Q_OBJECT
public:
    HumidityBox(QWidget *parent = nullptr);
    ~HumidityBox();
private:
    QLabel *humidity;
};

class WindBox: public GroupBox
{
    Q_OBJECT
public:
    WindBox(QWidget* parent = nullptr);
    ~WindBox();
private:
    QLabel *direction;
    QLabel *speed;
};

#endif // GROUPBOXES_H
