QT       += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

TARGET = spectrasynthesizer

SOURCES += \
    DBJson.cpp \
    main.cpp \
    SpectraSynthesizer.cpp

HEADERS += \
    DBJson.h \
    SpectraSynthesizer.h

FORMS += \
    SpectraSynthesizer.ui
