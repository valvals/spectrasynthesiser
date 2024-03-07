QT += core gui serialport printsupport widgets multimedia multimediawidgets

CONFIG += c++17

RC_FILE = recource.rc
TARGET = spectrasynthesizer

SOURCES += \
    camera_module.cpp \
    debug_console.cpp \
    fitting/fitSpectr.cpp \
    fitting/mpfit.c \
    fitting/relaxFilter.cpp \
    hamamatsu_api.cpp \
    json_utils.cpp \
    main.cpp \
    net_power_supply/power_supply_manager.cpp \
    ormin_device.cpp \
    qcustomplot.cpp \
    qrc_files_restorer.cpp \
    spectra_synthesizer.cpp \
    style_sheets.cpp \
    voice_informator.cpp

HEADERS += \
    camera_module.h \
    debug_console.h \
    fitting/dataStructs.h \
    fitting/fitSpectr.h \
    fitting/mpfit.h \
    fitting/relaxFilter.h \
    hamamatsu_api.h \
    json_utils.h \
    net_power_supply/commands_builder.h \
    net_power_supply/power_supply_manager.h \
    ormin_device.h \
    qcustomplot.h \
    qrc_files_restorer.h \
    spectra_synthesizer.h \
    style_sheets.h \
    version.h \
    voice_informator.h

FORMS += \
    debug_console.ui \
    spectra_synthesizer.ui

RESOURCES += \
    res.qrc
