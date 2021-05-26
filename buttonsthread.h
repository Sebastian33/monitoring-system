#include <gpiod.h>
#include <QThread>

#ifndef BUTTONSET_H
#define BUTTONSET_H
const unsigned UP_OFFSET = 0;
const unsigned DOWN_OFFSET = 1;
const unsigned MENU_OFFSET = 2;
const unsigned CONFIG_OFFSET = 3;
const unsigned SET_OFFSET = 4;
const unsigned CANCEL_OFFSET = 5;


const unsigned ERROR_CODE=1<<31;
const unsigned UP_CODE = 1;
const unsigned DOWN_CODE = 1<<DOWN_OFFSET;
const unsigned MENU_CODE = 1<<MENU_OFFSET;
const unsigned CONFIG_CODE = 1<<CONFIG_OFFSET;
const unsigned SET_CODE = 1<<SET_OFFSET;
const unsigned CANCEL_CODE = 1<<CANCEL_OFFSET;

class ButtonsThread : public QThread
{
    Q_OBJECT
private:
    struct gpiod_chip *chip;
    struct gpiod_line *upLine;
    struct gpiod_line *downLine;
    struct gpiod_line *menuLine;
    struct gpiod_line *configLine;
    struct gpiod_line *setLine;
    struct gpiod_line *cancelLine;

    unsigned code;
    bool threadState; // true - go on
public:
    explicit ButtonsThread(QObject *parent = nullptr);
    unsigned getCode();
    void stop();
    ~ButtonsThread() override;
protected:
    void run() override;
signals:
    void push();
    void longPush();
};

#endif // BUTTONSET_H
