#include <gpiod.h>
#include <QThread>

const unsigned ERROR_CODE=1<<31;

#ifndef BUTTONSET_H
#define BUTTONSET_H
class ButtonsThread : public QThread
{
    Q_OBJECT
private:
    struct gpiod_chip *chip;
    struct gpiod_line *upLine;
    struct gpiod_line *downLine;

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
