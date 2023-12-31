QT += core gui serialport printsupport widgets

CONFIG += c++17

RC_FILE = recource.rc
TARGET = spectrasynthesizer

SOURCES += \
    DBJson.cpp \
    QrcFilesRestorer.cpp \
    debug_console.cpp \
    main.cpp \
    SpectraSynthesizer.cpp \
    qcustomplot.cpp \
    style_sheets.cpp

HEADERS += \
    DBJson.h \
    QrcFilesRestorer.h \
    SpectraSynthesizer.h \
    Version.h \
    debug_console.h \
    qcustomplot.h \
    style_sheets.h

FORMS += \
    SpectraSynthesizer.ui \
    debug_console.ui

RESOURCES += \
    res.qrc
