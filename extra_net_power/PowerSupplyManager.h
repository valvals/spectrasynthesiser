#ifndef POWERSUPPLYMANAGER_H
#define POWERSUPPLYMANAGER_H
#include <QTcpSocket>
#include <QHostAddress>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include "CommandsBuilder.h"
#include "Sounder.h"
#include <QTimer>
#include "Types.h"
#include <QHash>

struct testResults {

  bool power1 = false;
  bool power2 = false;
  bool power3 = false;
  bool power4 = false;

};


enum class  operation : uint8_t        {
  increaseVoltage,
  singleOperation,
  switchOff_All,
  monitoring,
  test,
  decreasingVoltageAndSwitchOff
};


class PowerSupplyManager: public QObject {
  Q_OBJECT

 public:
  PowerSupplyManager();
  ~PowerSupplyManager();
  void getID();
  void getVoltage(quint16 unit);
  void setVoltage(quint16 unit, double value);
  void setCurrentLimit(quint16 unit, double value);
  void getCurrentLimit(quint16 unit);
  void getCurrentValue(quint16 unit);
  void getPowerStatus(quint16 unit);
  void switchOnAllUnits();
  void switchOnUnit(quint16 unit);
  void switchOffAllUnits();
  void switchOffUnit(quint16 unit);
  void connectToHost();
  void makeSoundNotification(const QString sample);
  void setOperation_variant(operation newOperation_variant);
  void switchOffOneLamp();
  const testResults& results() const;

 private:
  enum class  powerManagerState : uint8_t {
    NO_POWER_SUPPLIES,
    POWER_SUPPLY_1,
    POWER_SUPPLY_2,
    POWER_SUPPLY_3,
    POWER_SUPPLY_4
  } power_state;

  operation  operation_variant;

  enum class  out : uint8_t               {
    out1,
    out2
  };

  enum class  parametrRead : uint8_t     {
    V1, V2, I1, I2, LI1, LI2, S1, S2, ID
  }  parametr_state;


  testResults m_results;

  QStringList    m_IpAddresses;
  QTcpSocket*     m_socket;
  QHostAddress    m_hostAddress;
  quint16         m_hostPort;
  QString         answer;
  Sounder         m_sounder;

  double          m_target_I;
  double          m_accuracy_target_I;
  double          m_load_I;
  double          m_delta_V;
  double          m_acrcy_V;
  double          m_check_I;
  double          m_last_V_mult;         //Это множитель для последнего прибавления  m_delta_V * m_last_V_mult
  bool            m_isLastV;
  int             m_V_fail_counter;
  double          m_V;
  double          m_decreaseV;
  int             m_increaseV_interval;
  int             m_powerSupplyIndex;
  int             m_outNumber;
  QMap<PowerOuts, PowerUnitParams> powers_params;

  QJsonObject powerSupply_1;
  QJsonObject powerSupply_2;
  QJsonObject powerSupply_3;
  QJsonObject powerSupply_4;
  QHash<QPair<QString, int>, double> m_powerLimits;
  CommandsBuilder m_cb;

  bool   m_isTimeOut;
  QTimer increaseVoltageTimer;
  QTimer decreasingVoltageTimer;
  QTimer timeOutTimer;
  QTimer accrcyTimer;
  QTimer testDisplayTimer;

  void initializeJSonObjects();
  void messageWasRecievedAfterTimeout();
  void replaceUselessGetV(double& V, QString& msg);
  void replaceUselessGetI(double& I, QString& msg);
  void checkIfVoltageWasSetted(const double& Vps);
  void cantSetV_Case();
  void powerSupplyIsNotLoaded();
  void checkIfPwrSupplyIsLoaded(const double& V, const double& I);

  void setParamsForPwrSupply(QJsonObject& json);
  void setParamsForPwrSupply(out out, parametrRead pr, const double& value);
  void sendUpdateSignal();
  PowerOuts getPowerAndOut();
  double getCurrentLimitForCurrentPowerSupplyAndOut();



 public slots:
  void startToReachTargetCurrent();

 private slots:
  void connectedCase();
  void disconnectedCase();
  void recieveData();
  void errorInSocket(QAbstractSocket::SocketError error);
  void checkPowerUnitsConnection();

  void increasingVoltage();
  void timeOutCase();
  void increasingVoltageFinished();
  void decreasingProcess();
  void testChangeRandomAllParams();

 signals:
  void paramsPowerChanged(PowerOuts);
  void allLampsSwitchedOn();
  void oneLampWasSwitchedOff();
};

#endif // POWERSUPPLYMANAGER_H
