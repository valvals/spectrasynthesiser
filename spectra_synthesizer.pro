QT       += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

RC_FILE = recource.rc
TARGET = spectrasynthesizer

SOURCES += \
    DBJson.cpp \
    QrcFilesRestorer.cpp \
    main.cpp \
    SpectraSynthesizer.cpp \
    style_sheets.cpp

HEADERS += \
    DBJson.h \
    QrcFilesRestorer.h \
    SpectraSynthesizer.h \
    style_sheets.h

FORMS += \
    SpectraSynthesizer.ui

RESOURCES += \
    res.qrc
