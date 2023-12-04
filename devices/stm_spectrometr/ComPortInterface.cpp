#include "ComPortInterface.h"

ComPortInterface::ComPortInterface()
{
    m_currentCommandType = NO_COMMAND;
}

CurrentCommandType ComPortInterface::currentCommandType() const
{
    return m_currentCommandType;
}

void ComPortInterface::changeCurrentCommand(QByteArray command)
{
    if(command.contains('e'))
        m_currentCommandType = CHANGING_EXPOSITION;
    else
        m_currentCommandType = GETTING_SPECTRUM;
}

bool ComPortInterface::appendDataToPacket(QByteArray data, QByteArray &packet, QByteArray &dataToSend)
{
//    qDebug()<<"appending data..."<<static_cast<int>(sizeof(SpectrumData))<<packet.count();
    bool res = false;
    if(m_currentCommandType == CHANGING_EXPOSITION){
        if(data.count() == 4){
            dataToSend = data;
            packet.clear();
            res = true;
        }
    }else if(m_currentCommandType == GETTING_SPECTRUM){
        packet.append(data);
        if(packet.count() > static_cast<int>(sizeof(SpectrumData))){
            dataToSend = packet.mid(0, sizeof(SpectrumData));
            packet = packet.mid(sizeof(SpectrumData) - 1, packet.count() - static_cast<int>(sizeof(SpectrumData)));
            res = true;
        }else if(packet.count() == static_cast<int>(sizeof(SpectrumData))){
            dataToSend = packet;
            packet.clear();
            res = true;
        }
    }
    return res;
}
