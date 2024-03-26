#include "power_supply_manager.h"

#include <QFile>
#include <QDir>
#include "Windows.h"
#include "QTimer"
#include <QtGlobal>
#include "json_utils.h"

#define DECREASING_INTERVAL 100
#define DECREASING_DELTA 0.05

PowerSupplyManager::PowerSupplyManager() {
  m_socket = new QTcpSocket;
  m_powerSupplyIndex = 0; 
  m_hostPort  = 9221;
  loadJsonConfig();
  m_hostAddress.setAddress(m_powers["lamps"].toArray()[m_powerSupplyIndex].toObject()["ip"].toString());

  connect(m_socket, SIGNAL(connected()), this, SLOT(connectedCase()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnectedCase()));
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
    qDebug()<<m_powers;
    QString ip;
    int out;
    getIpAndOutForIndex(0,ip,out);
    m_hostAddress.setAddress(ip);
    m_socket->connectToHost(m_hostAddress,m_hostPort);
    m_socket->waitForConnected(3000);
}

void PowerSupplyManager::getVoltage(quint16 unit) {
  m_socket->write(m_cb.makeGetVcommand(unit));
  m_socket->waitForBytesWritten();
}

void PowerSupplyManager::setVoltage(quint16 unit, double value) {
  //qDebug()<<"setVoltage function...."<<value;
  m_socket->write(m_cb.makeSetVcommand(unit, value));
}

void PowerSupplyManager::setCurrentLimit(quint16 unit, double value) {
  //qDebug()<<"Set current limit";
  m_socket->write(m_cb.makeSetCurrentLimitCommand(unit, value));
}

void PowerSupplyManager::getCurrentLimit(quint16 unit) {
  timeOutTimer.start();
  //qDebug()<<"get Current limit function....";
  if (unit == 1)
    parametr_state = parametrRead::LI1;
  if (unit == 2)
    parametr_state = parametrRead::LI2;
  m_socket->write(m_cb.makeGetCurrentLimitCommand(unit));
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

void PowerSupplyManager::getPowerStatus(quint16 unit) {
  timeOutTimer.start();
  //qDebug()<<"get Switch State for unit"<< unit;
  if (unit == 1)
    parametr_state = parametrRead::S1;
  if (unit == 2)
    parametr_state = parametrRead::S2;
  m_socket->write(m_cb.makeGetSwitchStateCommand(unit));
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

void PowerSupplyManager::startToReachTargetCurrent() {
  m_target_I = getCurrentLimitForCurrentPowerSupplyAndOut();
  qDebug() << "Target I: " << m_target_I;
  m_accuracy_target_I = 0.01;
  m_last_V_mult = 20.0;
  m_acrcy_V = 0.05;
  m_load_I  = 0.03;
  m_delta_V = 0.2;
  m_check_I = 2.0;
  m_isLastV = false;
  m_V_fail_counter = 10;
  m_V = m_delta_V;
  m_increaseV_interval = 300;
  increaseVoltageTimer.setInterval(m_increaseV_interval);
  operation_variant = operation::increaseVoltage;
  timeOutTimer.start();
  increaseVoltageTimer.start();
  qDebug() << "Switch on all lamps * * * * * *  ";

}

void PowerSupplyManager::increasingVoltage() {
  increaseVoltageTimer.stop();
  setVoltage(m_outNumber, m_V);
  getVoltage(m_outNumber);
}

void PowerSupplyManager::timeOutCase() {
  m_isTimeOut = true;
  qDebug() << "Timeout";
  timeOutTimer.stop();
  //m_sounder.playSound("timeOutCase.mp3");
}

void PowerSupplyManager::increasingVoltageFinished() {
  qDebug() << "Increasing voltage process is finished. +++++++++++++++++++++++";
  increaseVoltageTimer.stop();

  if (m_outNumber == 2 && m_powerSupplyIndex == m_IpAddresses.size() - 1) {
    //m_sounder.playSound("lamps_is_switched_on.mp3");
    emit allLampsSwitchedOn();
    return;
  }
  //qDebug("It's ok");return;
  if (m_outNumber == 1) {
    m_outNumber = 2;
    startToReachTargetCurrent();
  } else {
    m_outNumber = 1;
    qDebug() << "Disconnecting...";
    m_socket->disconnectFromHost();

  }
}

void PowerSupplyManager::decreasingProcess() {
  qDebug() << "Decreasing process in progress...";
  decreasingVoltageTimer.stop();
  if (m_decreaseV > 0.005) {
    getVoltage(m_outNumber);
  } else {
    qDebug() << "Decreasing process has stopped...";
    if (m_outNumber == 2) {
      m_outNumber = 1;
      emit oneLampWasSwitchedOff();
    } else {
      m_socket->disconnectFromHost();
    }
  }
}

void PowerSupplyManager::testChangeRandomAllParams() {

  //setParamsForPwrSupply(powerSupply_1);
  //setParamsForPwrSupply(powerSupply_2);
  //setParamsForPwrSupply(powerSupply_3);
  //setParamsForPwrSupply(powerSupply_4);

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
  if (V >= m_check_I) {

    if (I < m_load_I) {
      powerSupplyIsNotLoaded();
      return;
    }

  } else {

    if (I < m_target_I) { //Accuracy I
      m_V += m_delta_V;
      increaseVoltageTimer.start();
      return;
    }
  }

  if (m_isLastV) {

    increasingVoltageFinished();
    return;
  }

  if (I >= m_target_I - m_accuracy_target_I) {
    qDebug() << "My last V multiplication.................................";
    m_V += m_delta_V * m_last_V_mult;
    m_isLastV = true;
    increaseVoltageTimer.start();
    return;
  } else {

    m_V += m_delta_V;
    increaseVoltageTimer.start();
    return;

  }


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

void PowerSupplyManager::connectedCase() {

}

void PowerSupplyManager::disconnectedCase() {


}
