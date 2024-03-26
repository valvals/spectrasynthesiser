#include "power_supply_manager.h"

#include <QFile>
#include <QDir>
#include "Windows.h"
#include "QTimer"
#include <QtGlobal>
#include "json_utils.h"


PowerSupplyManager::PowerSupplyManager() {
  m_socket = new QTcpSocket;
  m_powerSupplyIndex = 0; 
  m_hostPort  = 9221;
  loadJsonConfig();
  QString ip;
  int out;
  getIpAndOutForIndex(0,ip,out);
  m_hostAddress.setAddress(ip);
  m_socket->connectToHost(m_hostAddress,m_hostPort);
  m_socket->waitForConnected(3000);

  connect(m_socket, SIGNAL(readyRead()), this, SLOT(recieveData()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(errorInSocket(QAbstractSocket::SocketError)));

}

PowerSupplyManager::~PowerSupplyManager() {
  if (m_socket->state() == QAbstractSocket::ConnectedState)
    m_socket->disconnectFromHost();
}

void PowerSupplyManager::connectToHost() {
  m_socket->connectToHost(m_hostAddress, m_hostPort, QIODevice::ReadWrite);
}

void PowerSupplyManager::makeSoundNotification(const QString sample) {
  std::ignore = sample;
  //m_sounder.playSound(sample);
}

void PowerSupplyManager::setOperation_variant(operation newOperation_variant) {
  operation_variant = newOperation_variant;
}

void PowerSupplyManager::switchOffOneLamp() {
  operation_variant = operation::decreasingVoltageAndSwitchOff;
  getVoltage(m_outNumber);

}

const testResults& PowerSupplyManager::results() const {
  return m_results;
}

void PowerSupplyManager::getID() {
  parametr_state = parametrRead::ID;
  m_socket->write(m_cb.makeGetDeviceID_Command());
}

void PowerSupplyManager::loadJsonConfig() {
    jsn::getJsonObjectFromFile("ir_lamps.json",m_powers);
}

void PowerSupplyManager::getVoltage(quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeGetVcommand(out));
  m_socket->waitForReadyRead();
}

void PowerSupplyManager::setVoltage(quint16 unit, double value) {
  //qDebug()<<"setVoltage function...."<<value;
  m_socket->write(m_cb.makeSetVcommand(unit, value));
}

void PowerSupplyManager::setCurrentLimit(quint16 index, double value) {
  //qDebug()<<"Set current limit";
  m_socket->write(m_cb.makeSetCurrentLimitCommand(index, value));
}

void PowerSupplyManager::getCurrentLimit(quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeGetCurrentLimitCommand(out));
  m_socket->waitForReadyRead();
}

void PowerSupplyManager::getCurrentValue(quint16 unit) {
  timeOutTimer.start();
  //qDebug()<<"get Current value function....";
  if (unit == 1)
    parametr_state = parametrRead::I1;
  if (unit == 2)
    parametr_state = parametrRead::I2;
  m_socket->write(m_cb.makeGetCurrentValueCommand(unit));
}

void PowerSupplyManager::getPowerStatus(quint16 index) {
  m_socket->write(m_cb.makeGetSwitchStateCommand(index));
}

void PowerSupplyManager::switchOnAllUnits() {
  m_socket->write(m_cb.makeSwitchOnAllunitsCommand());
}

void PowerSupplyManager::switchOnUnit(quint16 unit) {
  m_socket->write(m_cb.makeSwitchOnUnitCommand(unit));
}

void PowerSupplyManager::switchOffAllUnits() {
  m_socket->write(m_cb.makeSwitchOffAllunitsCommand());
}

void PowerSupplyManager::switchOffUnit(quint16 unit) {
  m_socket->write(m_cb.makeSwitchOffUnitCommand(unit));
}

void PowerSupplyManager::recieveData() {

  answer = QString::fromLocal8Bit(m_socket->readAll());
  qDebug()<<answer;
}

void PowerSupplyManager::errorInSocket(QAbstractSocket::SocketError error) {
  qDebug()<<error;
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
      break;
    case QAbstractSocket::RemoteHostClosedError:
      break;
    case QAbstractSocket::HostNotFoundError:
      break;
    case QAbstractSocket::SocketAccessError:
      break;
    case QAbstractSocket::SocketResourceError:
      break;
    case QAbstractSocket::SocketTimeoutError:
      break;
    case QAbstractSocket::DatagramTooLargeError:
      break;
    case QAbstractSocket::NetworkError:
      //m_sounder.playSound("network_error.mp3");
      qDebug() << "No connection error.";
      break;
    case QAbstractSocket::AddressInUseError:
      break;
    case QAbstractSocket::SocketAddressNotAvailableError:
      break;
    case QAbstractSocket::UnsupportedSocketOperationError:
      break;
    case QAbstractSocket::UnfinishedSocketOperationError:
      break;
    case QAbstractSocket::ProxyAuthenticationRequiredError:
      break;
    case QAbstractSocket::SslHandshakeFailedError:
      break;
    case QAbstractSocket::ProxyConnectionRefusedError:
      break;
    case QAbstractSocket::ProxyConnectionClosedError:
      break;
    case QAbstractSocket::ProxyConnectionTimeoutError:
      break;
    case QAbstractSocket::ProxyNotFoundError:
      break;
    case QAbstractSocket::ProxyProtocolError:
      break;
    case QAbstractSocket::OperationError:
      break;
    case QAbstractSocket::SslInternalError:
      break;
    case QAbstractSocket::SslInvalidUserDataError:
      break;
    case QAbstractSocket::TemporaryError:
      break;
    case QAbstractSocket::UnknownSocketError:
      break;

  }
}

void PowerSupplyManager::checkPowerUnitsConnection() {

  //testChangeRandomAllParams();
  //m_sounder.playSound("welcome.mp3");
  //getPowerStatus(1);
  //switchOnAllUnits();
  //startToReachTargetCurrent();
}

void PowerSupplyManager::timeOutCase() {
  m_isTimeOut = true;
  qDebug() << "Timeout";
  timeOutTimer.stop();
  //m_sounder.playSound("timeOutCase.mp3");
}

void PowerSupplyManager::messageWasRecievedAfterTimeout() {
  qDebug() << "Message was recieved after timeout!";
}

void PowerSupplyManager::replaceUselessGetV(double& V, QString& msg) {
  if (msg.contains("V1"))
    msg.remove('\r').remove('\n').replace("V1 ", "");
  if (msg.contains("V2"))
    msg.remove('\r').remove('\n').replace("V2 ", "");
  bool isParsing = false;
  V = msg.toDouble(&isParsing);
  //qDebug()<<"V:"<<V;
}

void PowerSupplyManager::replaceUselessGetI(double& I, QString& msg) {
  msg.remove('\r').remove('\n').replace("A", "");
  bool isParsing = false;
  I = msg.toDouble(&isParsing);
  //qDebug()<<"I:"<<I;
}

void PowerSupplyManager::checkIfVoltageWasSetted(const double& Vps) {
  //qDebug()<<"accuracy"<<Vps;
  increaseVoltageTimer.stop();
  if (m_V >= Vps - m_acrcy_V && m_V <= Vps + m_acrcy_V) {
    getCurrentValue(m_outNumber);
  } else {
    --m_V_fail_counter;
    if (m_V_fail_counter < 0) {
      cantSetV_Case();
      return;
    }
    //qDebug()<<"I try to set V again...accuracy case"<<m_V_fail_counter;
    increaseVoltageTimer.start();
  }
}

void PowerSupplyManager::cantSetV_Case() {
  qDebug() << "Can't set V case";
  increaseVoltageTimer.stop();
}

void PowerSupplyManager::powerSupplyIsNotLoaded() {
  qDebug() << "Power supply is not loaded case....";
  //m_sounder.playSound("notLoaded.mp3");
}

void PowerSupplyManager::checkIfPwrSupplyIsLoaded(const double& V, const double& I) {


}

void PowerSupplyManager::setParamsForPwrSupply(out out, parametrRead pr, const double& value) {
  QString outStr;
  switch (out) {
    case PowerSupplyManager::out::out1: outStr = "out1";
      break;
    case PowerSupplyManager::out::out2: outStr = "out2";
      break;

  }
  QJsonObject power;
  if (m_powerSupplyIndex == 0)
    power = powerSupply_1.value(outStr).toObject();
  if (m_powerSupplyIndex == 1)
    power = powerSupply_2.value(outStr).toObject();
  if (m_powerSupplyIndex == 2)
    power = powerSupply_3.value(outStr).toObject();
  if (m_powerSupplyIndex == 3)
    power = powerSupply_4.value(outStr).toObject();

  switch (pr) {

    case PowerSupplyManager::parametrRead::ID:
      break;
    case PowerSupplyManager::parametrRead::V1: power["Voltage"] = value;
      break;
    case PowerSupplyManager::parametrRead::V2: power["Voltage"] = value;
      break;
    case PowerSupplyManager::parametrRead::I1: power["Current"] = value;
      break;
    case PowerSupplyManager::parametrRead::I2: power["Current"] = value;
      break;
    case PowerSupplyManager::parametrRead::LI1: power["LimitCurrent"] = value;
      break;
    case PowerSupplyManager::parametrRead::LI2: power["LimitCurrent"] = value;
      break;
    case PowerSupplyManager::parametrRead::S1: power["isOn"] = value;
      break;
    case PowerSupplyManager::parametrRead::S2: power["isOn"] = value;
      break;

  }
  if (m_powerSupplyIndex == 0) {
    powerSupply_1[outStr] = power;
    if (m_outNumber == 1)
      emit paramsPowerChanged(PowerOuts::POW1_OUT1);
    else
      emit paramsPowerChanged(PowerOuts::POW1_OUT2);
  }

  if (m_powerSupplyIndex == 1) {
    powerSupply_2[outStr] = power;

  }
  if (m_powerSupplyIndex == 2) {
    powerSupply_3[outStr] = power;

  }
  if (m_powerSupplyIndex == 3) {
    powerSupply_4[outStr] = power;
  }
  sendUpdateSignal();
}

void PowerSupplyManager::sendUpdateSignal() {
  if (m_powerSupplyIndex == 0) {

    if (m_outNumber == 1)
      emit paramsPowerChanged(PowerOuts::POW1_OUT1);
    else
      emit paramsPowerChanged(PowerOuts::POW1_OUT2);
  }

  if (m_powerSupplyIndex == 1) {

    if (m_outNumber == 1)
      emit paramsPowerChanged(PowerOuts::POW2_OUT1);
    else
      emit paramsPowerChanged(PowerOuts::POW2_OUT2);
  }
  if (m_powerSupplyIndex == 2) {

    if (m_outNumber == 1)
      emit paramsPowerChanged(PowerOuts::POW3_OUT1);
    else
      emit paramsPowerChanged(PowerOuts::POW3_OUT2);
  }
  if (m_powerSupplyIndex == 3) {

    if (m_outNumber == 1)
      emit paramsPowerChanged(PowerOuts::POW4_OUT1);
    else
      emit paramsPowerChanged(PowerOuts::POW4_OUT2);
  }
}

PowerOuts PowerSupplyManager::getPowerAndOut() {
  if (m_powerSupplyIndex == 0) {
    if (m_outNumber == 1)
      return PowerOuts::POW1_OUT1;
    else
      return PowerOuts::POW1_OUT2;
  };
  if (m_powerSupplyIndex == 1) {
    if (m_outNumber == 1)
      return PowerOuts::POW2_OUT1;
    else
      return PowerOuts::POW2_OUT2;
  };
  if (m_powerSupplyIndex == 2) {
    if (m_outNumber == 1)
      return PowerOuts::POW3_OUT1;
    else
      return PowerOuts::POW3_OUT2;
  };
  if (m_powerSupplyIndex == 3) {
    if (m_outNumber == 1)
      return PowerOuts::POW4_OUT1;
    else
      return PowerOuts::POW4_OUT2;
  };

  return PowerOuts::UKNOWN;
}

double PowerSupplyManager::getCurrentLimitForCurrentPowerSupplyAndOut() {
    return m_powerLimits.value(QPair<QString, int>(m_IpAddresses.at(m_powerSupplyIndex), m_outNumber));
}

void PowerSupplyManager::getIpAndOutForIndex(const int index,
                                             QString &ip,
                                             int &out)
{

    QJsonArray lamps = m_powers["lamps"].toArray();
    ip = lamps[index].toObject()["ip"].toString();
    out = lamps[index].toObject()["out"].toInt();
}

int PowerSupplyManager::maybeReconnectHost(const int index)
{
    int out;
    QString new_host;
    getIpAndOutForIndex(index,new_host,out);
    QString current_host = m_hostAddress.toString();
    if(current_host == new_host)return out;
    m_socket->disconnectFromHost();
    m_hostAddress.setAddress(new_host);
    m_socket->connectToHost(m_hostAddress,m_hostPort,QIODevice::ReadWrite);
    m_socket->waitForConnected();
    return out;
}

