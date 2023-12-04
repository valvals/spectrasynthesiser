#pragma once
#ifndef COMMONTYPES_H
#define COMMONTYPES_H
#include <QString>
#include <QVector>
#include <QMetaType>

/**
 * @brief The SpectrometerSettingsType enum
 * Enum of base spectrometer settings types
 *
 * EXPOSITION               Exposition settings
 * COUNT_IN_SERIES          Count of spectrums while series shooting is chosen
 * CALIBRATION_FILE_PATH    Path to calibration file (actual for FTDI device)
 * SAVING_FILE_PATH         Base directory to save spectrums
 * IS_NEED2SAVE_MEAN        Is need to calculate and save mean spectrums
 * IS_NEED2SAVE_DARK        Is need to save dark spectrums (Actual for Solar device)
 */
enum SpectrometerSettingsType {

    EXPOSITION, COUNT_IN_SERIES, CALIBRATION_FILE_PATH, SAVING_FILE_PATH, IS_NEED2SAVE_MEAN,
    IS_NEED2SAVE_DARK

};
Q_DECLARE_METATYPE(SpectrometerSettingsType)

/**
 * @brief The SpectrMode enum
 * Enum with spectrometer shooting modes
 *
 * SINGLE_MODE      Single spectrum mode
 * SERIES_MODE      Series spectrum mode
 * CONTINUOUS_MODE  Continuous mode (needed for continuous shooting, saving is made in separate thread)
 * COMBO_MODE       Mode needed to make series of dark and pattern spectrums and calculate afterall
 */
enum SpectrMode {

    SINGLE_MODE, SERIES_MODE, CONTINUOUS_MODE, COMBO_MODE

};

/**
 * @brief The BandsValues enum
 * Enum of bands values in spectrum
 *
 * BAND_NUMBERS     Numbers of bands
 * WAVELENGTH       Waves, nm
 */
enum BandsValues{

    BAND_NUMBERS, WAVELENGTH
};
Q_DECLARE_METATYPE(BandsValues)

/**
 * @brief The GraphValues enum
 * Enum of graph values in spectrum
 *
 * ADC_VALUES       ADC units
 * RFL_VALUES       Reflectance (from 0 to 1)
 */
enum GraphValues{

    ADC_VALUES, RFL_VALUES
};
Q_DECLARE_METATYPE(GraphValues)

enum CommonAngleType{
    SUN_ANGLE, SPECTROMETER_ANGLE, PATTERN_ANGLE
};
Q_DECLARE_METATYPE(CommonAngleType)

#pragma pack(push,1)
struct SpectrumData
{
    unsigned short  dummy1[14];     // 32 байта (нужно 16, заменено для STM)
    short int       black1[13];     // 26 байт
    unsigned short  dummy2[3];      // 6 байт
    short int       spectrum[3648]; // 7296 байт
    unsigned short  dummy[14];      // 28 байт
};  //!< Spectrum data structure
Q_DECLARE_METATYPE(SpectrumData)
#pragma pack(pop)

/**
 * @brief The Spectrum struct
 * Structure of spectrum data
 */
struct Spectrum{

    QString name;               //!< Spectrum name
    double exposition;          //!< Exposition in spectrum
    BandsValues bandsValues;    //!< Bands values
    GraphValues graphValues;    //!< Graph values
    QVector<double> wavesSpectrum;  //!< Vector of wavelength
    QVector<double> dataSpectrum;   //!< Vector of Spectrum data

};
Q_DECLARE_METATYPE(Spectrum)

/**
 * @brief The DarkSpectrum struct
 * Structure of dark spectrum data (actual for solar device)
 */
struct DarkSpectrum{

    int expositionIndex;            //!< Exposition index in list of expositions
    QVector<double> dataSpectrum;   //!< Dark spectrum data

};

/**
 * @brief The SunCoordinates struct
 * Sctructure needed to contain sun coordinates
 */
struct SunCoordinates
{
    double zenithAngle;     //!< Zenith angle
    double azimuth;         //!< azimuth
};

#endif // COMMONTYPES_H
