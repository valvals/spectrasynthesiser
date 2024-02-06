QT += core gui serialport printsupport widgets multimedia multimediawidgets

CONFIG += c++17

RC_FILE = recource.rc
TARGET = spectrasynthesizer

SOURCES += \
    CameraModule.cpp \
    DBJson.cpp \
    HamamatsuApi.cpp \
    OrminDevice.cpp \
    QrcFilesRestorer.cpp \
    debug_console.cpp \
    main.cpp \
    SpectraSynthesizer.cpp \
    qcustomplot.cpp \
    style_sheets.cpp

HEADERS += \
    CameraModule.h \
    DBJson.h \
    HamamatsuApi.h \
    OrminDevice.h \
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
