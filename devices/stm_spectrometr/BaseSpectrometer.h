#ifndef BASESPECTROMETER_H
#define BASESPECTROMETER_H

#include <QObject>
#include <QVariant>
#include "common/CommonTypes.h"

/**
 * @brief The BaseSpectrometer class
 * Class for using universal spectrometer object for any real spectrometer
 */
class BaseSpectrometer : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief BaseSpectrometer  Constructor
     * @param spectrometerType  Type of spectrometer
     */
    BaseSpectrometer();

    /**
     * @brief setIsNeedToSaveMean    Function to set variable 'm_isNeedToSaveMeen'
     * @param isNeed2SaveMean   Needed value
     */
    virtual void setIsNeedToSaveMean(bool isNeed2SaveMean);

    /**
     * @brief setIsNeed2SaveNow Function to set variable 'm_isNeedToSaveNow'
     * @param isNeed2SaveNow    Needed value
     */
    virtual void setIsNeedToSaveNow(bool isNeed2SaveNow);

    /**
     * @brief getBandsValues    Function to get bands values in spectrometer
     * @return  Bands Values (BAND_NUMBERS/WAVELENGTH)
     */
    BandsValues getBandsValues() const;

    /**
     * @brief getBands  Function to get bands vector
     * @return Bands vector
     */
    QVector<double> getBands() const;

    /**
     * @brief setBands      Function to set bands vector
     * @param bands         Vector to set
     * @param bandsValues   Bands Values (BAND_NUMBERS/WAVELENGTH)
     */
    virtual void setBands(const QVector<double> &bands, BandsValues bandsValues);

    /**
     * @brief getExpositionValue    Function to get spectrometer Exposition value
     * @return Spectrometer Exposition value
     */
    double getExpositionValue() const;

    /**
     * @brief getBandsCount Function to get spectrometer bands count
     * @return  Spectrometer bands count
     */
    int getBandsCount() const;

    /**
     * @brief setSpectrMode Function to set spectrometer shooting mode
     * @param spectrMode    Needed spectrometer shooting mode
     */
    virtual void setSpectrMode(const SpectrMode &spectrMode);

    /**
     * @brief setIsConfigurationMode    Function to set is it Configuration mode now (actual only for Solar spectrometer)
     * @param isConfigurationMode   Needed value
     */
    void setIsConfigurationMode(bool isConfigurationMode);

    /**
     * @brief setIsScannerMode  Function to set Scanner spectrometer mode (used only for Solar)
     * @param isScannerMode Is need scanner mode
     */
    virtual void setIsScannerMode(bool isScannerMode);

    /**
     * @brief getIsNeedToSaveMean   Function to get is it need to save mean spectrum now
     * @return  Is it need to save mean spectrum now
     */
    bool getIsNeedToSaveMean() const;

    /**
     * @brief getSpectrMode Function to get current spectrum shooting mode
     * @return Current spectrum shooting mode
     */
    SpectrMode getSpectrMode() const;

    /**
     * @brief getExpositionIndex    Function to get current exposition index (actual only for Solar spectrometer)
     * @return  Current exposition index
     */
    int getExpositionIndex() const;

    void setIsDarkSpectrumsNow(bool isDarkSpectrumsNow);

public slots:
    /**
     * @brief changeSettings    Slot needed to change Base Spectrometer Settings
     * @param settingsType  Spectrometer Settings type
     * @param var   Needed value
     */
    virtual void changeSettings(SpectrometerSettingsType settingsType, QVariant var);

    /**
     * @brief stopAll   Slot needed to stop application work
     */
    virtual void stopAll();

    /**
     * @brief captureSpectr Slot needed to capture spectrum
     */
    virtual void captureSpectr();

    /**
     * @brief setSpectrometerElevationAngle Slot needed to set spectrometer elevation angle
     * @param elevationAngle    Spectrometer elevation angle
     */
    void setSpectrometerElevationAngle(int elevationAngle);

    /**
     * @brief setSpectrometerShootingSurface    Slot needed to set spectrometer shooting surface
     * @param shootingSurface   Spectrometer shooting surface (0 - sun vertical, 1 - perpendicular to sun vertical)
     */
    void setSpectrometerShootingSurface(bool shootingSurface);

    /**
     * @brief setExpositionValue    Function to set spectrometer Exposition value
     * @param expositionValue   Needed value
     */
    virtual void setExpositionValue(double expositionValue);

    void changeShutterState(bool isOpen);

    void changeFilterState(bool doesExist);

signals:
    /**
     * @brief spectrReadyToShow Signal about ready to show spectrum data in plotter
     * @param data  Data vector
     * @param max   Maximum value in spectrum
     * @param isNeedToUpdate    Is need to update spectrum in plotter
     */
    //void spectrReadyToShow(QVector<double> data, double max, bool isNeedToUpdate);

    /**
     * @brief showMessage   Signal to show to user text message in message box
     * @param message   Needed information
     */
    void showMessage(QString message);

    /**
     * @brief showProgressText  Signal to show to user text message in progress dialog
     * @param text  text to show
     */
    void showProgressText(QString text);

    /**
     * @brief bandsChanged  Signal about bands vector changing
     * @param bands New bands vector
     * @param bandsValues   New bands values
     */
    void bandsChanged(const QVector<double> &bands, BandsValues bandsValues);

    /**
     * @brief configurationComplete Signal about spectrometer configuration finish (actual only for Solar device)
     */
    void configurationComplete();

    /**
     * @brief expositionWasChanged  Signal about exposition changing in spectrometer (needed to display in GUI)
     * @param value New exposition value
     */
    void expositionWasChanged(double value);

    /**
     * @brief seriesCompleteFor Signal about series shooting process
     * @param percent   Complete percent
     */
    void seriesCompleteFor(int percent);

    /**
     * @brief scannerCompleteFor    Signal about scanner shooting process
     * @param percent   Complete percent
     */
    void scannerCompleteFor(int percent);

protected:
    SpectrMode m_spectrMode;    //!< Spectrum shooting mode

    QVector <double> m_bands;   //!< Spectrum bands vector
    BandsValues m_bandsValues;  //!< Spectrum bands values (BAND_NUMBERS/WAVELENGTH)
    int m_bandsCount;           //!< Spectrum bands count

    bool m_isNeedToSaveMean;    //!< Is need to save mean spectrum while shooting
    bool m_isNeedToSaveNow;     //!< Is need to save currently got spectrum
    bool m_isConfigurationMode; //!< Is spectrometer in configuration mode
    bool m_isScannerMode;       //!< Is spectrometer in scanner mode
    bool m_isDarkSpectrumsNow;  //!< Is shooting dark spectrums now

    double m_expositionValue;   //!< Spectrometer exposition value (microseconds)
    int m_expositionIndex;      //!< Spectrometer exposition index
    int m_spectrometerElevationAngle;   //!< Spectrometer elevation angle (angle between horizon plane and spectrometer axe)
    bool m_spectrometerShootingSurface; //!< Spectrometer Shooting Surface (0 - sun vertical, 1 - perpendicular to sun vertical)
    bool m_shutterState;        //!< Spectrometer shutter state
    bool m_filterState;         //!< Spectrometer filter state
};

#endif // BASESPECTROMETER_H
