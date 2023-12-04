#ifndef TEENSY_H
#define TEENSY_H

#include "SpectroDevices/BaseSpectroDevice.h"
#include "ExtraDevices/ComPortManager.h"
#include <QObject>

class Teensy : public BaseSpectroDevice
{
    Q_OBJECT

    ComPortManager* m_comPortManager;   //!< Com Port Manager object
    bool m_isNeedToUpdate;    //!< Is need to update spectrum
    bool m_isSpectrumRequested;     //!< Is spectrum requested
    bool m_isExpositionChanging;    //!< Is exposition changing
    double m_tempExpositionValue;   //!< Spectrometer exposition value (prepared to changing)

    qint64 m_experimentTimeMs;

public:
    Teensy(QString comPortName, int baudRate);
    ~Teensy() override;
    bool isConnected() const;

signals:
    /**
     * @brief spectrReadyToShow Signal about ready to show spectrum data in plotter
     * @param data  Data vector
     * @param max   Maximum value in spectrum
     * @param isNeedToUpdate    Is need to update spectrum in plotter
     */
    void spectrReadyToShow(QVector<double> data, double max, bool isNeedToUpdate);

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
     * @brief writeCommandToComPort Function to write command trought serial port
     * @param command   Command to write
     */
    void writeCommandToComPort(QByteArray command);

public slots:
    /**
     * @brief captureSpectr Slot needed to capture spectrum
     */
    void captureSpectr() override;

    /**
     * @brief setExpositionValue    Function to set spectrometer Exposition value
     * @param expositionValue   Needed value
     */
    void setExpositionValue(double expositionValue) override;

private slots:
    /**
     * @brief parseComPortData  Function to take date from Com port and parse it
     * @param comAnswer Array from port
     */
    void parseComPortData(QByteArray comAnswer);

private:
    /**
     * @brief minusDarkPixelsAndSend    Function to minus dark pixels from data and send to GUI
     * @param spectrumData  Data recieved
     */
    void minusDarkPixelsAndSend(SpectrumData &spectrumData);

    /**
     * @brief requestExpositionChanging Function to request exposition changing
     */
    void requestExpositionChanging();
};

#endif // TEENSY_H
