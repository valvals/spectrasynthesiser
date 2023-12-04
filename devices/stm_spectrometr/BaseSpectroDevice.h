#ifndef ABSTRACTSPECTRODEVICE_H
#define ABSTRACTSPECTRODEVICE_H

#include <QObject>
#include "Math/CalculationModule.h"
#include "SpectrDataSaver.h"
#include "BaseSpectrometer.h"

/**
 * @brief The BaseSpectroDevice class
 * Class needed for universal using of spectral devices
 */
class BaseSpectroDevice : public BaseSpectrometer
{
    Q_OBJECT
public:
    BaseSpectroDevice();

    /**
     * @brief setSpectrMode Function to set spectrometer shooting mode
     * @param spectrMode    Needed spectrometer shooting mode
     */
    void setSpectrMode(const SpectrMode &spectrMode) override;

    /**
     * @brief setIsNeed2SaveNow Function to set variable 'm_isNeedToSaveNow'
     * @param isNeed2SaveNow    Needed value
     */
    void setIsNeedToSaveNow(bool isNeed2SaveNow) override;

    /**
     * @brief setBands      Function to set bands vector
     * @param bands         Vector to set
     * @param bandsValues   Bands Values (BAND_NUMBERS/WAVELENGTH)
     */
    void setBands(const QVector<double> &bands, BandsValues bandsValues) override;

    /**
     * @brief setSpectrDataSaver    Function to set Spectr Data Saver
     * @param spectrDataSaver   Spectrum Data Saver Object
     */
    void setSpectrDataSaver(SpectrDataSaver *spectrDataSaver);

    /**
     * @brief setCalculationModule    Function to set Calculation Module
     * @param calculationModule Calculation Module Object
     */
    void setCalculationModule(CalculationModule *calculationModule);

public slots:
    /**
     * @brief changeSettings    Slot needed to change Base Spectrometer Settings
     * @param settingsType  Spectrometer Settings type
     * @param var   Needed value
     */
    virtual void changeSettings(SpectrometerSettingsType settingsType, QVariant var) override;

    /**
     * @brief captureSpectr Slot needed to capture spectrum
     */
    virtual void captureSpectr() override;

    /**
     * @brief stopAll   Slot needed to stop application work
     */
    virtual void stopAll() override;

    /**
     * @brief setExpositionValue    Function to set spectrometer Exposition value
     * @param expositionValue   Needed value
     */
    virtual void setExpositionValue(double value) override;

private slots:
    /**
     * @brief deviceErrorrRecived   Slot to recieve device error
     * @param recivedError  Error text
     */
    void deviceErrorrRecived(QString recivedError);

signals:
    /**
     * @brief sendSpectrumToSaver   Signal to send spectrum data to Saver object
     * @param baseFileName  Base File Name to save
     * @param strData   Spectrum data as text string
     */
    void sendSpectrumToSaver(QString baseFileName, QString strData);

protected:    
    /**
     * @brief saveSpectrum  Function to save spectrum data using Spectrum Data Saver
     */
    void saveSpectrum();

    /**
     * @brief addSolarStatesToString    Function to add current Solar states to string
     */
    void addSpectrometerStatesToString();


    QVector <double> m_spectrData;  //!< Spectrum data vector
    QVector <double> m_blackPixels; //!< Black pixels vector
    QString m_specString;           //!< Spectrum data as text string

    double m_maxValueInSpectrum;    //!< Maximum value in spectrum data

    CalculationModule *m_calculationModule; //!< Calculation Module object
    SpectrDataSaver *m_spectrDataSaver;     //!< Spectrum Data Saver Object
};

#endif // ABSTRACTSPECTRODEVICE_H
