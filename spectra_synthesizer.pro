QT += core gui serialport printsupport widgets multimedia multimediawidgets

CONFIG += c++17

RC_FILE = recource.rc
TARGET = spectrasynthesizer

SOURCES += \
    CameraModule.cpp \
    HamamatsuApi.cpp \
    OrminDevice.cpp \
    debug_console.cpp \
    fitting/fitSpectr.cpp \
    fitting/mpfit.c \
    fitting/relaxFilter.cpp \
    json_utils.cpp \
    main.cpp \
    SpectraSynthesizer.cpp \
    qcustomplot.cpp \
    qrc_files_restorer.cpp \
    style_sheets.cpp \
    voice_informator.cpp

HEADERS += \
    CameraModule.h \
    HamamatsuApi.h \
    OrminDevice.h \
    SpectraSynthesizer.h \
    debug_console.h \
    fitting/dataStructs.h \
    fitting/fitSpectr.h \
    fitting/mpfit.h \
    fitting/relaxFilter.h \
    json_utils.h \
    qcustomplot.h \
    qrc_files_restorer.h \
    style_sheets.h \
    version.h \
    voice_informator.h

FORMS += \
    SpectraSynthesizer.ui \
    debug_console.ui

RESOURCES += \
    res.qrc
