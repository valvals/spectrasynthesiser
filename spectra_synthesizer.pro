QT       += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

RC_FILE = recource.rc
TARGET = spectrasynthesizer

SOURCES += \
    DBJson.cpp \
    main.cpp \
    SpectraSynthesizer.cpp \
    style_sheets.cpp

HEADERS += \
    DBJson.h \
    SpectraSynthesizer.h \
    style_sheets.h

FORMS += \
    SpectraSynthesizer.ui
