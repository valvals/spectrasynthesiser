#ifndef COMPORTMANAGER_H
#define COMPORTMANAGER_H

#include "QtSerialPort/QSerialPort"
#include "qbytearray.h"
#include "windows.h"
#include "ComPortInterface.h"

/**
 * @brief The ComPortManager class
 * Base class for COM-port using
 */
class ComPortManager : public QObject, public ComPortInterface
{
    Q_OBJECT

    QString m_portTextName;         //!< Com Port name
    int m_baudRate;                 //!< Baudrate
    QSerialPort *m_serialPort;      //!< Serial port
    bool m_connectionState;         //!< Manager state (0 - not connected, 1 - connected)
    QByteArray m_packetRecieved;

public:
    /**
     * @brief ComPortManager    Constructor
     * @param comPortName       Com Port Name
     * @param baudRate          Com Port Baud Rate
     */
    explicit ComPortManager(QString comPortName = "", int baudRate  = 0);
    ~ComPortManager() override;

    /**
     * @brief managerState  Function returns COM-port manager state
     * @return COM-port manager state (0 - not connected, 1 - connected)
     */
    bool connectionState() const;

public slots:
    /**
     * @brief writeCommand  Function writes command to device and recieves answer from device if it exists
     * @param command       Command as QByteArray
     * @param msToWait      Waiting time for writing
     * @return Is writing OK
     */
    bool writeCommand(QByteArray command, int msToWait = 30);

private:
    /**
     * @brief setUpComPort  Function makes COM-port setup and opens device
     */
    void setUpComPort();

private slots:
    /**
     * @brief recieveSerialPortAnswer   Slot for Serial port signal 'readyRead()'
     */
    void recieveSerialPortAnswer();

signals:
    /**
     * @brief dataIsReady   Signal about data recieved
     * @param recievedData  Data packet recieved throught serial port
     */
    void dataIsReady(QByteArray recievedData);
};

#endif // COMPORTMANAGER_H
