#include "PowerSupplyManager.h"
#include <QFile>
#include <QDir>
#include "Windows.h"
#include "QTimer"
#include <random>
#include <QtGlobal>

#define DECREASING_INTERVAL 100
#define DECREASING_DELTA 0.05

PowerSupplyManager::PowerSupplyManager() {
  m_socket = new QTcpSocket;
  m_powerSupplyIndex = 0;
  m_outNumber = 1;
  operation_variant = operation::test;
  m_IpAddresses << "192.168.1.24" << "192.168.1.23" << "192.168.1.22"; //<<"192.168.1.25";
  m_hostAddress.setAddress(m_IpAddresses.at(m_powerSupplyIndex));
  m_hostPort  = 9221;
  m_isTimeOut = false;
  timeOutTimer.setInterval(5000);
  initializeJSonObjects();
  powers_params = {
    {PowerOuts::POW1_OUT1, {0, 0, 0, false}},
    {PowerOuts::POW1_OUT2, {0, 0, 0, false}},
    {PowerOuts::POW2_OUT1, {0, 0, 0, false}},
    {PowerOuts::POW2_OUT2, {0, 0, 0, false}},
    {PowerOuts::POW3_OUT1, {0, 0, 0, false}},
    {PowerOuts::POW3_OUT2, {0, 0, 0, false}},
    {PowerOuts::POW4_OUT1, {0, 0, 0, false}},
    {PowerOuts::POW4_OUT2, {0, 0, 0, false}}
  };
  //connectToHost();
  connect(m_socket, SIGNAL(connected()), this, SLOT(connectedCase()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnectedCase()));
  connect(m_socket, SIGNAL(readyRead()), this, SLOT(recieveData()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(errorInSocket(QAbstractSocket::SocketError)));
  connect(&timeOutTimer, SIGNAL(timeout()), this, SLOT(timeOutCase()));
  connect(&increaseVoltageTimer, SIGNAL(timeout()), this, SLOT(increasingVoltage()));
  connect(&testDisplayTimer, SIGNAL(timeout()), SLOT(testChangeRandomAllParams()));
  connect(&decreasingVoltageTimer, SIGNAL(timeout()), this, SLOT(decreasingProcess()));
  //QTimer::singleShot(1000,this,SLOT(checkPowerUnitsConnection()));
  //testDisplayTimer.start(1000);
}

PowerSupplyManager::~PowerSupplyManager() {
  if (m_socket->state() == QAbstractSocket::ConnectedState)
    m_socket->disconnectFromHost();
}

void PowerSupplyManager::connectToHost() {
  m_socket->connectToHost(m_hostAddress, m_hostPort, QIODevice::ReadWrite);
}

void PowerSupplyManager::makeSoundNotification(const QString sample) {
  m_sounder.playSound(sample);
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

void PowerSupplyManager::initializeJSonObjects() {
  QFile file(QDir::currentPath() + "/PowerSupplyList.json");
  if (!file.exists()) {
    QJsonObject outObj{
      {"isOn", false},
      {"Voltage", 0.0},
      {"Current_T", 0.0},
      {"LimitCurrent", 0.0},
      {"LimitVoltage", 0.0},
      {"Power", 0.0},
      {"Delta_V", 0.0},
      {"Delta_t", 0.0},
      {"TargetName", "Lamp X"},
    };
    powerSupply_1.insert("PowerSupplyName", "");
    powerSupply_1.insert("IP", "0.0.0.0");
    powerSupply_1.insert("MAC", "00-00-00-00-00-00");
    powerSupply_1.insert("isConnected", false);
    powerSupply_1.insert("out1", outObj);
    powerSupply_1.insert("out2", outObj);
    powerSupply_2 = powerSupply_1;
    powerSupply_3 = powerSupply_1;
    powerSupply_4 = powerSupply_1;

    QJsonArray jArray = {powerSupply_1, powerSupply_2, powerSupply_3, powerSupply_4};

    file.open(QIODevice::WriteOnly);
    file.write(QJsonDocument(jArray).toJson(QJsonDocument::Indented));
  } else {
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    QJsonParseError errorPtr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &errorPtr);
    if (doc.isNull()) {
      qDebug() << "Ошибка разбора JSON!";
    }
    powerSupply_1 = doc.array().at(0).toObject();
    powerSupply_2 = doc.array().at(1).toObject();
    powerSupply_3 = doc.array().at(2).toObject();
    //powerSupply_4 = doc.array().at(3).toObject();
    qDebug() << "Check ps1 property: " << powerSupply_1.value("PowerSupplyName").toString();
    qDebug() << "Check ps2 property: " << powerSupply_2.value("PowerSupplyName").toString();
    qDebug() << "Check ps3 property: " << powerSupply_3.value("PowerSupplyName").toString();
    //qDebug()<<"Check ps4 property: "<<powerSupply_4.value("PowerSupplyName").toString();

    for (int i = 0; i < m_IpAddresses.count(); ++i) {
      double pl1 = doc.array().at(i).toObject().value("out1").toObject().value("Current_target").toDouble();
      double pl2 = doc.array().at(i).toObject().value("out2").toObject().value("Current_target").toDouble();
      m_powerLimits.insert(QPair<QString, int>(m_IpAddresses.at(i), 1), pl1);
      m_powerLimits.insert(QPair<QString, int>(m_IpAddresses.at(i), 2), pl2);
      qDebug() << "-----> pL: " << pl1 << pl2;
    }
  }
  file.close();
}

void PowerSupplyManager::getVoltage(quint16 unit) {
  timeOutTimer.start();
  //qDebug()<<"getVoltage function....";
  if (unit == 1)
    parametr_state = parametrRead::V1;
  if (unit == 2)
    parametr_state = parametrRead::V2;
  m_socket->write(m_cb.makeGetVcommand(unit));
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
  qDebug() << "Switch on all units....";
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
  //qDebug()<<"Recieve data function..."<<(int)operation_variant;
  timeOutTimer.stop();
  answer = QString::fromLocal8Bit(m_socket->readAll());
  if (!m_isTimeOut) {

    if (operation_variant == operation::increaseVoltage) {

      double psV = -1;
      double psI = -1;

      switch (parametr_state) {
        case PowerSupplyManager::parametrRead::ID: break;
        case parametrRead::S1: qDebug() << "Power out 1 state: " << answer; break;
        case parametrRead::S2: qDebug() << "Power out 2 state: " << answer; break;
        case parametrRead::V1: {
          //qDebug()<<"Voltage out 1: "<<answer;
          replaceUselessGetV(psV, answer);
          checkIfVoltageWasSetted(psV);
          powers_params[getPowerAndOut()].V = psV;
          break;
        }
        case parametrRead::V2: {
          //qDebug()<<"Voltage out 2: "<<answer;
          replaceUselessGetV(psV, answer);
          checkIfVoltageWasSetted(psV);
          powers_params[getPowerAndOut()].V = psV;
          break;
        }
        case parametrRead::I1: {
          //qDebug()<<"Current out 1: "<<answer;
          replaceUselessGetI(psI, answer);
          checkIfPwrSupplyIsLoaded(m_V, psI);
          powers_params[getPowerAndOut()].I = psI;
          break;
        }
        case parametrRead::I2: {
          //qDebug()<<"Current out 2: "<<answer;
          replaceUselessGetI(psI, answer);
          checkIfPwrSupplyIsLoaded(m_V, psI);
          powers_params[getPowerAndOut()].I = psI;
          break;
        }
        case parametrRead::LI1: qDebug() << "Current out 1: " << answer; break;
        case parametrRead::LI2: qDebug() << "Current out 2: " << answer; break;
      }
    }
    if (operation_variant == operation::test) {
      qDebug() << "Test power supplyies cases:";
      switch (parametr_state) {
        case PowerSupplyManager::parametrRead::ID:
          break;
        case PowerSupplyManager::parametrRead::V1:
          qDebug() << "V1: " << answer;
          setParamsForPwrSupply(out::out1, parametrRead::V1, answer.toDouble());
          getVoltage(2);
          break;
        case PowerSupplyManager::parametrRead::V2:
          qDebug() << "V2: " << answer;
          setParamsForPwrSupply(out::out2, parametrRead::V2, answer.toDouble());
          getCurrentValue(1);
          break;
        case PowerSupplyManager::parametrRead::I1:
          qDebug() << "I1: " << answer;
          setParamsForPwrSupply(out::out1, parametrRead::I1, answer.toDouble());
          getCurrentValue(2);
          break;
        case PowerSupplyManager::parametrRead::I2:
          qDebug() << "I2: " << answer;
          setParamsForPwrSupply(out::out2, parametrRead::I2, answer.toDouble());
          getCurrentLimit(1);
          break;
        case PowerSupplyManager::parametrRead::LI1:
          qDebug() << "LI1: " << answer;
          setParamsForPwrSupply(out::out1, parametrRead::LI1, answer.toDouble());
          getCurrentLimit(2);
          break;
        case PowerSupplyManager::parametrRead::LI2:
          qDebug() << "LI2: " << answer;
          setParamsForPwrSupply(out::out2, parametrRead::LI2, answer.toDouble());
          getPowerStatus(1);
          break;
        case PowerSupplyManager::parametrRead::S1:
          qDebug() << "PS1: " << answer;
          setParamsForPwrSupply(out::out1, parametrRead::S1, answer.toDouble());
          getPowerStatus(2);
          break;
        case PowerSupplyManager::parametrRead::S2:
          qDebug() << "PS2: " << answer;
          setParamsForPwrSupply(out::out2, parametrRead::S2, answer.toDouble());
          qDebug() << "********************************\n\n";
          switchOnAllUnits();
          m_socket->disconnectFromHost();
          break;

      }
    }
    if (operation_variant == operation::decreasingVoltageAndSwitchOff) {
      switch (parametr_state) {
        case parametrRead::V1: {

          qDebug() << "Voltage decreasing out 1: " << answer;
          double psV = -1;
          replaceUselessGetV(psV, answer);
          if (psV > 0.005) {
            m_decreaseV = psV - DECREASING_DELTA;
            if (m_decreaseV < 0)
              m_decreaseV = 0;
            setVoltage(m_outNumber, m_decreaseV);
            decreasingVoltageTimer.start(DECREASING_INTERVAL);
            break;
          } else {
            //Добавить команду выключения выхода
            qDebug() << "Decreasing is finished...";
          }
          break;

        }
        case parametrRead::V2: {

          qDebug() << "Voltage decreasing out 2: " << answer;
          double psV = -1;
          replaceUselessGetV(psV, answer);
          if (psV > 0.005) {
            m_decreaseV = psV - 0.1;
            if (m_decreaseV < 0)
              m_decreaseV = 0;
            setVoltage(m_outNumber, m_decreaseV);
            decreasingVoltageTimer.start(DECREASING_INTERVAL);
            break;
          } else {
            //Добавить команду выключения выхода
            qDebug() << "Decreasing is finished...";
            switchOffUnit(m_outNumber);//NOT TESTED
          }
          break;
        }
      }
    }


  }
}

void PowerSupplyManager::errorInSocket(QAbstractSocket::SocketError error) {
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
      m_sounder.playSound("network_error.mp3");
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
  m_sounder.playSound("timeOutCase.mp3");
}

void PowerSupplyManager::increasingVoltageFinished() {
  qDebug() << "Increasing voltage process is finished. +++++++++++++++++++++++";
  increaseVoltageTimer.stop();

  if (m_outNumber == 2 && m_powerSupplyIndex == m_IpAddresses.size() - 1) {
    m_sounder.playSound("lamps_is_switched_on.mp3");
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

  setParamsForPwrSupply(powerSupply_1);
  setParamsForPwrSupply(powerSupply_2);
  setParamsForPwrSupply(powerSupply_3);
  setParamsForPwrSupply(powerSupply_4);

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
  m_sounder.playSound("notLoaded.mp3");
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

void PowerSupplyManager::setParamsForPwrSupply(QJsonObject& json) {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.00, 20.00);
  qDebug() << dis(gen);
  QJsonObject power1 =  json.value("out1").toObject();
  power1["Voltage"] = dis(gen);
  power1["Current"] = dis(gen);
  QJsonObject power2 =  json.value("out1").toObject();
  power2["Voltage"] = dis(gen);
  power2["Current"] = dis(gen);
  json["out1"] = power1;
  json["out2"] = power2;
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

void PowerSupplyManager::connectedCase() {
  if (m_powerSupplyIndex == 0) {
    powerSupply_1.value("isConnected") = true;
    sendUpdateSignal();
    m_results.power1 = true;
  }
  if (m_powerSupplyIndex == 1) {
    powerSupply_2.value("isConnected") = true;
    sendUpdateSignal();
    m_results.power2 = true;
  }
  if (m_powerSupplyIndex == 2) {
    powerSupply_3.value("isConnected") = true;
    sendUpdateSignal();
    m_results.power3 = true;
  }
  if (m_powerSupplyIndex == 3) {
    powerSupply_4.value("isConnected") = true;
    sendUpdateSignal();
    m_results.power4 = true;
  }

  qDebug() << "Power supply " << m_IpAddresses.at(m_powerSupplyIndex) << " is connected.";

  if (operation_variant == operation::test) {
    setVoltage(1, 0);
    setVoltage(2, 0);
    setCurrentLimit(1, m_powerLimits.value(QPair<QString, int>(m_IpAddresses.at(m_powerSupplyIndex), 1)));
    setCurrentLimit(2, m_powerLimits.value(QPair<QString, int>(m_IpAddresses.at(m_powerSupplyIndex), 2)));
    getVoltage(m_outNumber);

  }
  if (operation_variant == operation::increaseVoltage) {

    qDebug() << "increasing starts **************************************************";
    startToReachTargetCurrent();
  }
  if (operation_variant == operation::decreasingVoltageAndSwitchOff) {

    qDebug() << "Change network interface after lamp was switched off" << m_powerSupplyIndex << m_outNumber;
    emit oneLampWasSwitchedOff();
  }

}

void PowerSupplyManager::disconnectedCase() {

  if (m_powerSupplyIndex == 0) {
    powerSupply_1.value("isConnected") = false;
    sendUpdateSignal();
  }
  if (m_powerSupplyIndex == 1) {
    powerSupply_2.value("isConnected") = false;
    sendUpdateSignal();
  }
  if (m_powerSupplyIndex == 2) {
    powerSupply_3.value("isConnected") = false;
    sendUpdateSignal();
  }
  if (m_powerSupplyIndex == 3) {
    powerSupply_4.value("isConnected") = false;
    sendUpdateSignal();
  }

  if (operation_variant == operation::test) {
    if (m_powerSupplyIndex < m_IpAddresses.size() - 1) {
      ++m_powerSupplyIndex;
      qDebug() << "Index: " << m_powerSupplyIndex;
      m_hostAddress.setAddress(m_IpAddresses.at(m_powerSupplyIndex));
      connectToHost();
    } else {
      qDebug() << "All power supplyies are tested.";
      operation_variant = operation::singleOperation;
      m_powerSupplyIndex = 0;
      m_powerSupplyIndex = 0;
      timeOutTimer.stop();
      m_hostAddress.setAddress(m_IpAddresses.at(m_powerSupplyIndex));
      connectToHost();
      return;
    }
  }
  if (operation_variant == operation::increaseVoltage) {

    //if(m_outNumber == 1)m_outNumber = 2;
    //if(m_outNumber == 2)m_outNumber = 1;

    if (m_powerSupplyIndex < m_IpAddresses.size() - 1) {

      ++m_powerSupplyIndex;
      m_hostAddress.setAddress(m_IpAddresses.at(m_powerSupplyIndex));
      qDebug() << "Connect to " << m_powerSupplyIndex << ")))))))))))))))))))))))))))))))))))))))))))))))";
      connectToHost();


    } else {

      qDebug() << "All lamps are ready.";
    }
  }
  if (operation_variant == operation::decreasingVoltageAndSwitchOff) {

    if (m_powerSupplyIndex > 0) {

      -- m_powerSupplyIndex;
      m_outNumber = 2;
      m_hostAddress.setAddress(m_IpAddresses.at(m_powerSupplyIndex));
      connectToHost();
    } else {

      //if all lamps were switched off some code
      emit oneLampWasSwitchedOff();
    }

  }

}
