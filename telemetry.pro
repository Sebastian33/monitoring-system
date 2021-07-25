QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += serialport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    GroupBoxes.cpp \
    buttonsthread.cpp \
    i2cconverter.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    GroupBoxes.h \
    buttonsthread.h \
    i2cconverter.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/../../raspi/sysroot/lib/arm-linux-gnueabihf/ -lgpiod

INCLUDEPATH += $$PWD/../../raspi/sysroot/usr/include
DEPENDPATH += $$PWD/../../raspi/sysroot/usr/include

unix|win32: LIBS += -lwiringPi
