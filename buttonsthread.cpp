#include "buttonsthread.h"

const char *CONSUMER = "Telemetry";
const char *CHIP = "gpiochip0";
const unsigned UP_LINE_NUM = 27;
const unsigned DOWN_LINE_NUM = 26;
const unsigned MENU_LINE_NUM = 23;
const unsigned CONFIG_LINE_NUM = 22;
const unsigned SET_LINE_NUM = 21;
const unsigned CANCEL_LINE_NUM = 20;

const unsigned INTERVAL = 10;
const unsigned INTERVALS_TO_PUSH = 3;
const unsigned INTERVALS_TO_LONG_PUSH = 150;

ButtonsThread::ButtonsThread(QObject *parent) : QThread(parent), code(0), threadState(true)
{
    chip = gpiod_chip_open_by_name(CHIP);
    upLine = gpiod_chip_get_line(chip, UP_LINE_NUM);
    downLine = gpiod_chip_get_line(chip, DOWN_LINE_NUM);
    menuLine = gpiod_chip_get_line(chip, MENU_LINE_NUM);
    configLine = gpiod_chip_get_line(chip, CONFIG_LINE_NUM);
    setLine = gpiod_chip_get_line(chip, SET_LINE_NUM);
    cancelLine = gpiod_chip_get_line(chip, CANCEL_LINE_NUM);
    if(!(upLine && downLine && menuLine && configLine && setLine && cancelLine))
    {
        qInfo("some line failed to open");
        code=ERROR_CODE;
        return;
    }

    if(gpiod_line_request_input(upLine, CONSUMER)|
       gpiod_line_request_input(downLine, CONSUMER)|
       gpiod_line_request_input(menuLine, CONSUMER)|
       gpiod_line_request_input(configLine, CONSUMER)|
       gpiod_line_request_input(setLine, CONSUMER)|
       gpiod_line_request_input(cancelLine, CONSUMER))
    {
        qInfo("line request failed");
        code=ERROR_CODE;
        return;
    }
}

ButtonsThread::~ButtonsThread()
{
    gpiod_chip_close(chip);
}

void ButtonsThread::stop()
{
    threadState = false;
}

void ButtonsThread::run()
{
    unsigned currentCode(0);
    int counter(0);
    while(threadState)
    {
        currentCode = (gpiod_line_get_value(upLine)<<UP_OFFSET)|
                (gpiod_line_get_value(downLine)<<DOWN_OFFSET)|
                (gpiod_line_get_value(menuLine)<<MENU_OFFSET)|
                (gpiod_line_get_value(configLine)<<CONFIG_OFFSET)|
                (gpiod_line_get_value(setLine)<<SET_OFFSET)|
                (gpiod_line_get_value(cancelLine)<<CANCEL_OFFSET);
        //qInfo("%d", currentCode);
        if(currentCode!=0)
        {
            if(counter==0)
            {
                code = currentCode;
                counter++;
            }
            else if((currentCode & code) == 0)
            {
                code = 0;
                counter = 0;
                currentCode = 0;
            }
            else
            {
                counter++;
            }
        }
        else if(counter!=0)
        {
            counter = 0;
            code = 0;
        }

        if(counter == INTERVALS_TO_PUSH)
            emit push();
        else if(counter == INTERVALS_TO_LONG_PUSH)
            emit longPush();
        QThread::msleep(INTERVAL);
    }
    exit();
}

unsigned ButtonsThread::getCode()
{
    return code;
}
