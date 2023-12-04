#include "ComPortManager.h"
#include "QDebug"

ComPortManager::ComPortManager(QString comPortName, int baudRate) : QObject(), ComPortInterface()
{
    m_portTextName = comPortName;
    m_baudRate = baudRate;
    m_connectionState = false;
    m_serialPort = nullptr;
    setUpComPort();
}

ComPortManager::~ComPortManager()
{
    m_serialPort->close();
    if(m_serialPort != nullptr)
        delete m_serialPort;
}

void ComPortManager::setUpComPort()
{
    m_serialPort = new QSerialPort();
    m_serialPort->setPortName(m_portTextName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if(m_serialPort->open(QIODevice::ReadWrite)){
        m_connectionState = true;
        connect(m_serialPort, SIGNAL (readyRead()), SLOT(recieveSerialPortAnswer()));
    }else{
        m_connectionState = false;
    }
}

bool ComPortManager::writeCommand(QByteArray command, int msToWait)
{
    changeCurrentCommand(command);
    qDebug()<<"write"<<command;

    bool res = false;
    if(m_serialPort->isOpen()){
        m_serialPort->write(command);
        if(m_serialPort->waitForBytesWritten(msToWait))
            res = true;
    }
    return res;
}

void ComPortManager::recieveSerialPortAnswer()
{
    QByteArray data = m_serialPort->readAll();
    qDebug()<<"read bytes:"<<data.count();
    QByteArray dataToSend;
    if(appendDataToPacket(data, m_packetRecieved, dataToSend))
        emit dataIsReady(dataToSend);
}

bool ComPortManager::connectionState() const
{
    return m_connectionState;
}
