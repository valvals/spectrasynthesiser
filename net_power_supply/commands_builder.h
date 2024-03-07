#ifndef COMMANDS_BUILDER_H
#define COMMANDS_BUILDER_H

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <cstdint>

class CommandsBuilder {

 public:
  QByteArray makeSwitchOnAllunitsCommand() {
    return m_commands.switchOnAll.toUtf8();
  }

  QByteArray makeSwitchOffAllunitsCommand() {
    return m_commands.switchOffAll.toUtf8();
  }

  QByteArray makeSwitchOnUnitCommand(const uint16_t& unit) {
    return makeCommand(m_commands.switchUnit, unit, 1);
  }

  QByteArray makeSwitchOffUnitCommand(const uint16_t& unit) {
    return makeCommand(m_commands.switchUnit, unit, 0);
  }

  QByteArray makeSetCurrentLimitCommand(const uint16_t& unit,
                                        const float& value) {
    return makeCommand(m_commands.current, unit, value);
  }

  QByteArray makeGetCurrentLimitCommand(const uint16_t& unit) {
    return makeCommand(m_commands.current, unit);
  }

  QByteArray makeGetCurrentValueCommand(const uint16_t& unit) {
    return makeReadBackCommand(m_commands.current, unit);
  }

  QByteArray makeGetVoltageValueCommand(const uint16_t& unit) {
    return makeReadBackCommand(m_commands.voltage, unit);
  }

  QByteArray makeSetVcommand(const uint16_t& unit, const float& value) {
    return makeCommand(m_commands.voltage, unit, value);
  }

  QByteArray makeGetVcommand(const uint16_t& unit) {
    return makeCommand(m_commands.voltage, unit);
  }

  QByteArray makeGetSwitchStateCommand(const uint16_t& unit) {
    return makeCommand(m_commands.switchUnit, unit);
  }

  QByteArray makeGetDeviceID_Command() { return m_commands.deviceID.toUtf8(); };

 private:
  struct Commands {
    const QString deviceID = "*IDN?";
    const QString switchOnAll = "OPALL 1";
    const QString switchOffAll = "OPALL 0";
    const QString switchUnit = "OP";
    const QString voltage = "V";
    const QString current = "I";
    const QString deltaI = "DELTAI";
    const QString deltaV = "DELTAV";
    const QString incrementI = "INCI";
    const QString incrementV = "INCV";
    const QString decrementI = "DECI";
    const QString decrementV = "DECV";
  } m_commands;


  inline QByteArray makeCommand(const QString& command, const quint16& unit,
                                const float& value) {
    QString unitStr = QString::number(unit);
    QString valueStr = QString::number(value);
    QString commandStr = QString("%1%2 %3\n").arg(command, unitStr, valueStr);
    return commandStr.toUtf8();
  };

  inline QByteArray makeCommand(const QString& command, const quint16& unit) {
    QString unitStr = QString::number(unit);
    QString commandStr = QString("%1%2?\n").arg(command, unitStr);
    return commandStr.toUtf8();
  };

  inline QByteArray makeReadBackCommand(const QString& command,
                                        const quint16& unit) {
    QString unitStr = QString::number(unit);
    QString commandStr = QString("%1%2O?\n").arg(command, unitStr);
    return commandStr.toUtf8();
  };
};

#endif // COMMANDS_BUILDER_H
