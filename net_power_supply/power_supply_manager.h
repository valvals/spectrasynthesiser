#ifndef POWER_SUPPLY_MANAGER_H
#define POWER_SUPPLY_MANAGER_H

#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include "commands_builder.h"



class PowerSupplyManager: public QObject {
  Q_OBJECT

 public:
  PowerSupplyManager();
  ~PowerSupplyManager();
  void getID();
  void getPowerStatus(const quint16 index);

  void getVoltage(const quint16 index);
  void setVoltage(const quint16 index,
                  double value);

  void setCurrentLimit(const quint16 index,
                       double value);
  void getCurrentLimit(const quint16 index);
  void getCurrentValue(const quint16 index);


  void switchOnUnit(const quint16 index);
  void switchOffUnit(const quint16 unit);

  void switchOnAllUnits();
  void switchOffAllUnits();

 private:
  QTcpSocket*     m_socket;
  QHostAddress    m_hostAddress;
  QString         answer;

  QJsonObject m_powers;
  CommandsBuilder m_cb;

  void loadJsonConfig();
  void replaceUselessGetV(double& V, QString& msg);
  void replaceUselessGetI(double& I, QString& msg);

  void getIpAndOutForIndex(const int index,
                           QString& ip,
                           int& out);
  int maybeReconnectHost(const int index);
  void checkPowersConection();

 private slots:
  void recieveData();
  void errorInSocket(QAbstractSocket::SocketError error);

};

#endif // POWER_SUPPLY_MANAGER_H
