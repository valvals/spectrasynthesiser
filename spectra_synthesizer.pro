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
    fitting/fitSpectr.cpp \
    fitting/mpfit.c \
    main.cpp \
    SpectraSynthesizer.cpp \
    qcustomplot.cpp \
    style_sheets.cpp \
    voice_informator.cpp

HEADERS += \
    CameraModule.h \
    DBJson.h \
    HamamatsuApi.h \
    OrminDevice.h \
    QrcFilesRestorer.h \
    SpectraSynthesizer.h \
    Version.h \
    debug_console.h \
    fitting/dataStructs.h \
    fitting/fitSpectr.h \
    fitting/mpfit.h \
    qcustomplot.h \
    style_sheets.h \
    voice_informator.h

FORMS += \
    SpectraSynthesizer.ui \
    debug_console.ui

RESOURCES += \
    res.qrc
