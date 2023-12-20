QT += core gui serialport printsupport widgets

CONFIG += c++17

RC_FILE = recource.rc
TARGET = spectrasynthesizer

SOURCES += \
    DBJson.cpp \
    QrcFilesRestorer.cpp \
    devices/stm_spectrometr/BaseSpectroDevice.cpp \
    devices/stm_spectrometr/BaseSpectrometer.cpp \
    devices/stm_spectrometr/CalibrationModule.cpp \
    devices/stm_spectrometr/ComPortInterface.cpp \
    devices/stm_spectrometr/ComPortManager.cpp \
    devices/stm_spectrometr/SpectrDataSaver.cpp \
    devices/stm_spectrometr/Teensy.cpp \
    main.cpp \
    SpectraSynthesizer.cpp \
    math/CalculationModule.cpp \
    math/CubSpline.cpp \
    math/RelaxMediumAlg.cpp \
    qcustomplot.cpp \
    style_sheets.cpp

HEADERS += \
    DBJson.h \
    QrcFilesRestorer.h \
    SpectraSynthesizer.h \
    devices/stm_spectrometr/BaseSpectroDevice.h \
    devices/stm_spectrometr/BaseSpectrometer.h \
    devices/stm_spectrometr/CalibrationModule.h \
    devices/stm_spectrometr/ComPortInterface.h \
    devices/stm_spectrometr/ComPortManager.h \
    devices/stm_spectrometr/SpectrDataSaver.h \
    devices/stm_spectrometr/Teensy.h \
    math/CalculationModule.h \
    math/CubSpline.h \
    math/RelaxMediumAlg.h \
    qcustomplot.h \
    style_sheets.h

FORMS += \
    SpectraSynthesizer.ui

RESOURCES += \
    res.qrc
