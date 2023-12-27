#ifndef COMPORTINTERFACE_H
#define COMPORTINTERFACE_H

#include <QByteArray>
#include <common/CommonTypes.h>
#include <QList>

enum CurrentCommandType {
  NO_COMMAND, CHANGING_EXPOSITION, GETTING_SPECTRUM
};

class ComPortInterface {
  CurrentCommandType m_currentCommandType;    //!< Current Command Type
  QList<QByteArray> m_nextCommands;           //!< List of queed commands

 public:
  /**
   * @brief ComPortInterface  Constructor
   */
  ComPortInterface();

  /**
   * @brief currentCommandType    Finction to get current command type
   * @return Current command type
   */
  CurrentCommandType currentCommandType() const;

 protected:
  /**
   * @brief changeCurrentCommand  Function to change current command type
   * @param command   Current command
   */
  void changeCurrentCommand(QByteArray command);

  /**
   * @brief appendDataToPacket    Function to append new data to packet recieved before
   * @param data  New recieved data
   * @param packet    Packet recieved before
   * @param dataToSend    Ready packet to send (if the function return true)
   * @return  Is all packet recieved
   */
  bool appendDataToPacket(QByteArray data, QByteArray& packet, QByteArray& dataToSend);
};

#endif // COMPORTINTERFACE_H
