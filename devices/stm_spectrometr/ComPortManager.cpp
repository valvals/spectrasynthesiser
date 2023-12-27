#include "ComPortManager.h"
#include "QDebug"
#include "QSerialPortInfo"

ComPortManager::ComPortManager(QString comPortName, int baudRate) : QObject(), ComPortInterface() {
  m_portTextName = comPortName;
  m_baudRate = baudRate;
  m_connectionState = false;
  m_serialPort = nullptr;
  setUpComPort();
}

ComPortManager::~ComPortManager() {
  m_serialPort->close();
  if (m_serialPort != nullptr)
    delete m_serialPort;
}

void ComPortManager::setUpComPort() {
  m_serialPort = new QSerialPort();
  m_serialPort->setPortName("COM9");
  m_serialPort->setBaudRate(115200);
  m_serialPort->setDataBits(QSerialPort::Data8);
  m_serialPort->setParity(QSerialPort::NoParity);
  m_serialPort->setStopBits(QSerialPort::OneStop);
  m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
  m_serialPort->open(QIODevice::ReadWrite);
  connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(recieveSerialPortAnswer()));
  /*QSerialPortInfo serial_port_info;
  auto available_ports = serial_port_info.availablePorts();

  for(int i=0;i<available_ports.size();++i){
      qDebug()<<available_ports[i].serialNumber()
              <<available_ports[i].portName();
      if(available_ports[i].serialNumber()=="2089358E5748"){
          m_serialPort->setPort(available_ports[i]);
          m_serialPort->open(QIODevice::ReadWrite);
          connect(m_serialPort,SIGNAL(readyRead()),this,SLOT(recieveSerialPortAnswer()));
          m_connectionState = true;
          qDebug()<<"Stm spectrometr.....is connected";
          break;
      }
      m_connectionState = false;
  }*/

}

bool ComPortManager::writeCommand(QByteArray command, int msToWait) {
  changeCurrentCommand(command);
  qDebug() << "write" << command;

  bool res = false;
  if (m_serialPort->isOpen()) {
    m_serialPort->write(command);
    if (m_serialPort->waitForBytesWritten(msToWait))
      res = true;
  }
  qDebug() << "write" << res;
  return res;
}

void ComPortManager::recieveSerialPortAnswer() {
  QByteArray data = m_serialPort->readAll();
  qDebug() << "read bytes:" << data.count();
  QByteArray dataToSend;
  if (appendDataToPacket(data, m_packetRecieved, dataToSend))
    emit dataIsReady(dataToSend);
}

bool ComPortManager::connectionState() const {
  return m_connectionState;
}
