#include "power_supply_manager.h"

#include <QFile>
#include <QDir>
#include "Windows.h"
#include "QTimer"
#include <QtGlobal>
#include "json_utils.h"

constexpr int host_port = 9221;

PowerSupplyManager::PowerSupplyManager() {
  m_socket = new QTcpSocket;
  loadJsonConfig();
  QString ip;
  int out;
  getIpAndOutForIndex(0, ip, out);
  m_hostAddress.setAddress(ip);
  m_socket->connectToHost(m_hostAddress, host_port);
  m_socket->waitForConnected(1000);
  checkPowersConection();
  connect(m_socket, SIGNAL(readyRead()), this, SLOT(recieveData()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(errorInSocket(QAbstractSocket::SocketError)));

}

PowerSupplyManager::~PowerSupplyManager() {
  if (m_socket->state() == QAbstractSocket::ConnectedState)
    m_socket->disconnectFromHost();
}

void PowerSupplyManager::getID() {
  m_socket->write(m_cb.makeGetDeviceID_Command());
  m_socket->waitForReadyRead();
}

void PowerSupplyManager::loadJsonConfig() {
  jsn::getJsonObjectFromFile("ir_lamps.json", m_powers);
}

void PowerSupplyManager::getVoltage(const quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeGetVcommand(out));
  m_socket->waitForReadyRead();
}

void PowerSupplyManager::setVoltage(const quint16 index,
                                    double value) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeSetVcommand(out, value));
  m_socket->waitForBytesWritten();
}

void PowerSupplyManager::setCurrentLimit(const quint16 index,
                                         double value) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeSetCurrentLimitCommand(out, value));
  m_socket->waitForBytesWritten();
}

void PowerSupplyManager::getCurrentLimit(const quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeGetCurrentLimitCommand(out));
  m_socket->waitForReadyRead();
}

void PowerSupplyManager::getCurrentValue(const quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeGetCurrentValueCommand(out));
  m_socket->waitForReadyRead();
}

void PowerSupplyManager::getPowerStatus(const quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeGetSwitchStateCommand(out));
  m_socket->waitForReadyRead();
}

void PowerSupplyManager::switchOnUnit(const quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeSwitchOnUnitCommand(out));
  m_socket->waitForBytesWritten();
}

void PowerSupplyManager::switchOffUnit(const quint16 index) {
  int out = maybeReconnectHost(index);
  m_socket->write(m_cb.makeSwitchOffUnitCommand(out));
  m_socket->waitForBytesWritten();
}

void PowerSupplyManager::switchOnAllUnits() {
  auto indexes = m_powers["lamps"].toArray().size();
  for (int i = 0; i < indexes; ++i) {
    switchOnUnit(i);
  };
}

void PowerSupplyManager::switchOffAllUnits() {
  auto indexes = m_powers["lamps"].toArray().size();
  for (int i = 0; i < indexes; ++i) {
    switchOffUnit(i);
  };
}

void PowerSupplyManager::recieveData() {
  answer = QString::fromLocal8Bit(m_socket->readAll());
  qDebug() << answer;
}

void PowerSupplyManager::errorInSocket(QAbstractSocket::SocketError error) {
  qDebug() << error;
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

void PowerSupplyManager::replaceUselessGetV(double& V, QString& msg) {
  if (msg.contains("V1"))
    msg.remove('\r').remove('\n').replace("V1 ", "");
  if (msg.contains("V2"))
    msg.remove('\r').remove('\n').replace("V2 ", "");
  bool isParsing = false;
  V = msg.toDouble(&isParsing);
}

void PowerSupplyManager::replaceUselessGetI(double& I, QString& msg) {
  msg.remove('\r').remove('\n').replace("A", "");
  bool isParsing = false;
  I = msg.toDouble(&isParsing);
}

void PowerSupplyManager::getIpAndOutForIndex(const int index,
                                             QString& ip,
                                             int& out) {

  QJsonArray lamps = m_powers["lamps"].toArray();
  ip = lamps[index].toObject()["ip"].toString();
  out = lamps[index].toObject()["out"].toInt();
}

int PowerSupplyManager::maybeReconnectHost(const int index) {
  int out;
  QString new_host;
  getIpAndOutForIndex(index, new_host, out);
  QString current_host = m_hostAddress.toString();
  if (current_host == new_host)
    return out;
  m_socket->disconnectFromHost();
  m_hostAddress.setAddress(new_host);
  m_socket->connectToHost(m_hostAddress, host_port, QIODevice::ReadWrite);
  m_socket->waitForConnected(1000);
  return out;
}

void PowerSupplyManager::checkPowersConection() {
  QJsonArray lamps = m_powers["lamps"].toArray();
  auto indexes = lamps.size();

  for (int i = 0; i < indexes; ++i) {
    auto object = lamps[i].toObject();
    maybeReconnectHost(i);
    auto state = m_socket->state();
    switch (state) {
      case QTcpSocket::UnconnectedState:
        //qDebug()<<"no connection for index: "<<i;
        object["conection_state"] = false;
        break;
      case QTcpSocket::ConnectedState:
        //qDebug()<<"connection for index: "<<i<<" --> OK";
        object["conection_state"] = true;
        break;
    }
    lamps[i] = object;
  };
  m_powers["lamps"] = lamps;
  qDebug() << m_powers;
}
