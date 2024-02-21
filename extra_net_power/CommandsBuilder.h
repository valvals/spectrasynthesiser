#ifndef COMMANDSBUILDER_H
#define COMMANDSBUILDER_H
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <cstdint>

class CommandsBuilder {

public:
  //!
  //! \brief  Создаёт комманду для включения всех модулей блока
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeSwitchOnAllunitsCommand() {
    return m_commands.switchOnAll.toUtf8();
  }

  //!
  //! \brief  Создаёт комманду для выключения всех модулей блока
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeSwitchOffAllunitsCommand() {
    return m_commands.switchOffAll.toUtf8();
  }

  //!
  //! \brief  Создаёт комманду для включения модуля блока
  //! \param  unit номер модуля блока питания
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeSwitchOnUnitCommand(const uint16_t &unit) {
    return makeCommand(m_commands.switchUnit, unit, 1);
  }

  //!
  //! \brief  Создаёт комманду для выключения модуля блока
  //! \param  unit номер модуля блока питания
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeSwitchOffUnitCommand(const uint16_t &unit) {
    return makeCommand(m_commands.switchUnit, unit, 0);
  }

  //!
  //! \brief  Создаёт комманду для установки значения ограничения по току
  //! \param  unit номер модуля блока питания
  //! \param  value  значения тока ограничения (ампер)
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeSetCurrentLimitCommand(const uint16_t &unit,
                                        const float &value) {
    return makeCommand(m_commands.current, unit, value);
  }

  //!
  //! \brief  Создаёт комманду для получения значения ограничения по току
  //! \param  unit номер модуля блока питания
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeGetCurrentLimitCommand(const uint16_t &unit) {
    return makeCommand(m_commands.current, unit);
  }

  //!
  //! \brief  Создаёт комманду для получения текущего значения тока
  //! \param  unit номер модуля блока питания
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeGetCurrentValueCommand(const uint16_t &unit) {
    return makeReadBackCommand(m_commands.current, unit);
  }

  //!
  //! \brief  Создаёт комманду для получения текущего значения напряжения
  //! \param  unit номер модуля блока питания
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeGetVoltageValueCommand(const uint16_t &unit) {
    return makeReadBackCommand(m_commands.voltage, unit);
  }

  //!
  //! \brief  makeSetVcommand
  //! \param  unit
  //! \param  value
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeSetVcommand(const uint16_t &unit, const float &value) {
    return makeCommand(m_commands.voltage, unit, value);
  }

  //!
  //! \brief  makeGetVcommand
  //! \param  unit
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeGetVcommand(const uint16_t &unit) {
    return makeCommand(m_commands.voltage, unit);
  }
  //!
  //! \brief  makeGetSwitchStateCommand
  //! \param  unit
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeGetSwitchStateCommand(const uint16_t &unit) {
    return makeCommand(m_commands.switchUnit, unit);
  }

  //!
  //! \brief  makeGetDeviceID_Command
  //! \return QByteArray Комманда в виде байтового массива
  //!
  QByteArray makeGetDeviceID_Command() { return m_commands.deviceID.toUtf8(); };

private:
  struct Commands {
    const QString deviceID = "*IDN?"; //!< Возвращает идентификатор БП
    const QString switchOnAll =
        "OPALL 1"; //!< Одновременное включение всех выходов на БП
    const QString switchOffAll =
        "OPALL 0"; //!< Одновременное выключение всех выходов на БП
    const QString switchUnit =
        "OP"; //!< Получение статуса выхода на БП (0 - выключен, 1 - включён)
    const QString voltage = "V";
    const QString current = "I";
    const QString deltaI = "DELTAI";
    const QString deltaV = "DELTAV";
    const QString incrementI = "INCI";
    const QString incrementV = "INCV";
    const QString decrementI = "DECI";
    const QString decrementV = "DECV";
  } m_commands;

  //!
  //! \brief  Общая функция для формирования комманды состоящей из трёх
  //! элементов \param  Строковая константа обозначающая комманду \param  Номер
  //! модуля для блока \param  Значение параметра \return QByteArray Комманда в
  //! виде байтового массива
  //!
  inline QByteArray makeCommand(const QString &command, const quint16 &unit,
                                const float &value) {

    QString unitStr = QString::number(unit);
    QString valueStr = QString::number(value);
    QString commandStr = QString("%1%2 %3\n").arg(command, unitStr, valueStr);
    // qDebug()<<commandStr<<"  *******************************";
    return commandStr.toUtf8();
  };

  //!
  //! \brief  Общая функция для формирования комманды состоящей из двух
  //! элементов \param  Строковая константа обозначающая комманду \param  Номер
  //! модуля для блока \return QByteArray Комманда в виде байтового массива
  //!
  inline QByteArray makeCommand(const QString &command, const quint16 &unit) {

    QString unitStr = QString::number(unit);
    QString commandStr = QString("%1%2?\n").arg(command, unitStr);
    return commandStr.toUtf8();
  };

  //!
  //! \brief  Общая функция для формирования комманды получения текущего
  //! состояния параметров U,I \param  Строковая константа обозначающая комманду
  //! \param  Номер модуля для блока
  //! \return QByteArray Комманда в виде байтового массива
  //!
  inline QByteArray makeReadBackCommand(const QString &command,
                                        const quint16 &unit) {

    QString unitStr = QString::number(unit);
    QString commandStr = QString("%1%2O?\n").arg(command, unitStr);
    return commandStr.toUtf8();
  };
};
#endif // COMMANDSBUILDER_H
