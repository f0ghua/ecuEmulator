#include "DbcHandle.h"

#include <QDebug>

namespace Vector {
namespace DBC {

QString NetworkHandle::buildAttributeString(const Attribute &attribute)
{
    QString tempString;

    tempString.append("Attr:" + attribute.name);
    tempString.append(" || ");
    tempString.append("valueType:");
    switch(attribute.valueType)
    {
        // Integer
        case AttributeValueType::Int:
            tempString.append("Int");
            tempString.append(QString("<%1>").arg(attribute.integerValue));
            break;

        // Hexadecimal
        case AttributeValueType::Hex:
            tempString.append("Hex");
            tempString.append(QString("<0x%1>").arg(attribute.hexValue, 0, 16));
            break;

        // Float
        case AttributeValueType::Float:
            tempString.append("Float");
            tempString.append(QString("<%1>").arg(attribute.floatValue));
            break;

        // String
        case AttributeValueType::String:
            tempString.append("String");
            tempString.append(QString("<%1>").arg(attribute.stringValue));
            break;

        // Enumeration
        case AttributeValueType::Enum:
            tempString.append("Enum");
            tempString.append(QString("<%1>").arg(attribute.enumValue));
            break;
    }

    return tempString;
}

void NetworkHandle::dumpMessages()
{
    int count = 0;

    //QMap<unsigned int, Message>::const_iterator ci;
    for(auto ci = network.messages.constBegin();
        ci != network.messages.constEnd();
        ci++)
    {
        qDebug() << ci.key() << " : " << ci.value().name;
        count++;
    }

    qDebug() << "total message count " << count;
}

void NetworkHandle::dumpMessagesTransmitters()
{
    int count = 0;
    QString tempString;

    //QMap<unsigned int, Message>::const_iterator ci;
    for(auto ci = network.messages.constBegin();
        ci != network.messages.constEnd();
        ci++)
    {
        tempString.clear();
        tempString.append(QString("Message(%1)[%2].transmitters: ").arg(ci.key()).arg(ci.value().name));

        if (!ci.value().transmitter.isEmpty())
        {
            tempString.append(ci.value().transmitter);
        }
        else
        {
            for (int i = 0; i < ci.value().transmitters.length(); i++)
            {
                tempString.append(ci.value().transmitters[i]);
                tempString.append(" ");
            }
        }

        qDebug() << tempString;
        count++;
    }

    qDebug() << "total message count " << count;
}

void NetworkHandle::dumpMessagesSignalGroups()
{
    QString tempString;

    //QMap<unsigned int, Message>::const_iterator ci;
    for(auto ci = network.messages.constBegin();
        ci != network.messages.constEnd();
        ci++)
    {
        qDebug() << QString("Message(%1)[%2].signalGroup: ").arg(ci.key()).arg(ci.value().name);

        for (auto sg : ci.value().signalGroups)
        {
            tempString.clear();
            tempString.append("\t" + sg.name + " : ");
            for (int i = 0; i < sg.signal.size(); i++)
            {
                tempString.append(sg.signal[i]);
                if (i != (sg.signal.size()-1))
                    tempString.append(" ");
            }

            qDebug() << tempString;
        }
    }
}

void NetworkHandle::dumpSignals()
{
    QMap<unsigned int, Message>::const_iterator ciMsg;
    QMap<QString, Signal>::const_iterator ciSig;
    int count = 0, subCount = 0;

    for (ciMsg = network.messages.constBegin();
        ciMsg != network.messages.constEnd();
        ciMsg++)
    {
        qDebug() << "Message : " << ciMsg.value().id << " " << ciMsg.value().name;
        subCount = 0;

        for (ciSig = ciMsg.value().m_signals.constBegin();
            ciSig != ciMsg.value().m_signals.constEnd();
            ciSig++)
        {
            qDebug() << "    Signal : " << ciSig.value().name;
            subCount++;
            count++;
        }
        qDebug() << "    subCount " << subCount;
    }

    qDebug() << "total signal count " << count;
}

void NetworkHandle::dumpSignalsReceivers()
{
    QMap<unsigned int, Message>::const_iterator ciMsg;
    QMap<QString, Signal>::const_iterator ciSig;
    int count = 0, subCount = 0;

    for (ciMsg = network.messages.constBegin();
        ciMsg != network.messages.constEnd();
        ciMsg++)
    {
        qDebug() << "Message : " << ciMsg.value().id << " " << ciMsg.value().name;
        subCount = 0;

        for (ciSig = ciMsg.value().m_signals.constBegin();
            ciSig != ciMsg.value().m_signals.constEnd();
            ciSig++)
        {
            qDebug() << "    Signal : " << ciSig.value().name;

            QString tempString;
            for (int i = 0; i < ciSig.value().receivers.size(); i++)
            {
                tempString.append(ciSig.value().receivers.at(i));
                if (i < ciSig.value().receivers.size()-1)
                    tempString.append(",");
            }
            qDebug() << "      RCV(" << ciSig.value().receivers.size() << "): " << tempString;

            subCount++;
            count++;
        }
        qDebug() << "    subCount " << subCount;
    }

    qDebug() << "total signal count " << count;
}

void NetworkHandle::dumpMappedSignalsbyMsgId(int id)
{
    Message *msg = network.findMsgByID(id);

    if (msg == NULL)
    {
        qDebug() << QString("Message(id=%1) is not found").arg(id, 0, 16);
        return;
    }
    //QMap<unsigned int, Message>::const_iterator ci;
    for (auto ci = msg->m_signals.constBegin();
        ci != msg->m_signals.constEnd();
        ci++)
    {
        qDebug() << ci.value().name << ", receivers: " << ci.value().receivers;
    }
}

void NetworkHandle::dumpSignalValueDecriptionByName(QString name)
{
    QMap<unsigned int, Message>::const_iterator ciMsg;
    QMap<QString, Signal>::const_iterator ciSig;

    for (ciMsg = network.messages.constBegin();
        ciMsg != network.messages.constEnd();
        ciMsg++)
    {
        qDebug() << "Message : " << ciMsg.value().id;
        for (ciSig = ciMsg.value().m_signals.constBegin();
            ciSig != ciMsg.value().m_signals.constEnd();
            ciSig++)
        {
            qDebug() << "    Signal : " << ciSig.value().name;
            if (name == ciSig.value().name)
            {
                qDebug() << "find signal " << ciSig.value().name;
                break;
            }
        }
        if(ciSig != ciMsg.value().m_signals.constEnd())
            break;
    }
    if (ciMsg == network.messages.constEnd())
    {
        qDebug() << QString("The signal(%1) has not found").arg(name);
        return;
    }
    for (auto ci = ciSig.value().valueDescriptions.constBegin();
        ci != ciSig.value().valueDescriptions.constEnd();
        ci++)
    {
        qDebug() << QString("sig(%1) val ").arg(ciSig.value().name) << ci.key() << ", desc " << ci.value();
    }
}

void NetworkHandle::dumpAttributeDefinitions()
{
    QString tempString;

    for (auto ci = network.attributeDefinitions.constBegin();
        ci != network.attributeDefinitions.constEnd();
        ci++)
    {
        tempString.clear();

        qDebug() << "Attr name: " << ci.key();

        if (ci.value().valueType == AttributeValueType::Int)
        {
            tempString.append(QString::number(ci.value().minimumIntegerValue));
            tempString.append(" ~ ");
            tempString.append(QString::number(ci.value().maximumIntegerValue));
        } else
        if (ci.value().valueType == AttributeValueType::Enum)
        {
            for (int i = 0; i < ci.value().enumValues.size(); i++)
            {
                tempString.append(ci.value().enumValues[i]);
                if (i != (ci.value().enumValues.size()-1))
                    tempString.append(" ");
            }
        }

        qDebug() << "  Range : " << tempString;
    }
}

void NetworkHandle::dumpAttributeDefaults()
{
    QString tempString;

    for (auto ci = network.attributeDefaults.constBegin();
        ci != network.attributeDefaults.constEnd();
        ci++)
    {
        tempString.clear();

        qDebug() << "Attr name: " << ci.key();

        if (ci.value().valueType == AttributeValueType::Int)
        {
            tempString.append(QString::number(ci.value().integerValue));
        } else
        if (ci.value().valueType == AttributeValueType::Enum)
        {
            tempString.append(ci.value().stringValue);
        }

        qDebug() << "  Value : " << tempString;
    }
}

void NetworkHandle::dumpNodesAttributes()
{
    QString tempString;
    QMap<QString, Node>::const_iterator ciNode;

    for (ciNode = network.nodes.constBegin();
        ciNode != network.nodes.constEnd();
        ciNode++)
    {
        qDebug() << "Node: " << ciNode.value().name;
        for (auto ci = ciNode.value().attributeValues.constBegin();
            ci != ciNode.value().attributeValues.constEnd();
            ci++)
        {
            tempString = buildAttributeString(ci.value());
            qDebug() << "\t" << tempString;
        }
    }
}

void NetworkHandle::dumpMessagesAttributes()
{
    QString tempString;
    QMap<unsigned int, Message>::const_iterator ciObject;

    for (ciObject = network.messages.constBegin();
        ciObject != network.messages.constEnd();
        ciObject++)
    {
        qDebug() << QString("Message: [%1]%2").arg(ciObject.value().id).arg(ciObject.value().name);
        for (auto ci = ciObject.value().attributeValues.constBegin();
            ci != ciObject.value().attributeValues.constEnd();
            ci++)
        {
            tempString = buildAttributeString(ci.value());
            qDebug() << "\t" << tempString;
        }
    }
}

void NetworkHandle::dumpSignalsAttributes()
{
    QString tempString;
    QMap<unsigned int, Message>::const_iterator ciMsg;

    for (ciMsg = network.messages.constBegin();
        ciMsg != network.messages.constEnd();
        ciMsg++)
    {
        qDebug() << QString("Message: [%1]%2").arg(ciMsg.value().id).arg(ciMsg.value().name);

        for (auto ciObject = ciMsg.value().m_signals.constBegin();
            ciObject != ciMsg.value().m_signals.constEnd();
            ciObject++)
        {
            qDebug() << "\tSignal : " << ciObject.value().name;
            for (auto ci = ciObject.value().attributeValues.constBegin();
                ci != ciObject.value().attributeValues.constEnd();
                ci++)
            {
                tempString = buildAttributeString(ci.value());
                qDebug() << "\t\t" << tempString;
            }
        }
    }
}

void NetworkHandle::dumpNetworkAttributes()
{
    QString tempString;

    qDebug() << "Network: ";
    for (auto ci = network.attributeValues.constBegin();
        ci != network.attributeValues.constEnd();
        ci++)
    {
        tempString = buildAttributeString(ci.value());
        qDebug() << "\t" << tempString;
    }

}

void NetworkHandle::dumpRawBits(QByteArray &canData)
{
    QString tempString;

    tempString.clear();
    tempString.append("msg value raw = <");
    for (int i = 0; i < canData.length(); i++)
    {
        tempString.append("0x" + QString::number(canData.at(i)&0xff, 16).toUpper());
        if (i < (canData.length()-1)) tempString.append(" ");
    }
    tempString.append(">");
    qDebug() << tempString;

    tempString.clear();
    for (int i = 7; i >= 0; i--)
    {
        tempString.append(QString("%1 ").arg(i, 5, 10, QChar(' ')));
    }
    qDebug() << "\t" << tempString;

    tempString.clear();
    for (int i = 7; i >= 0; i--)
    {
        tempString.append(QString("------"));
    }
    qDebug() << "\t" <<tempString;

    for(int j = 0; j < 8; j++)
    {
        uint8_t ch = 0;

        if (j < canData.length()) ch = canData.at(j);

        tempString.clear();
        for (int i = 7; i >= 0; i--)
        {
            tempString.append(QString("%1(%2) ").arg(i+(j*8), 2, 10, QChar(' ')).arg(((ch & (1 << i))?1:0)));
        }

        qDebug() << QString::number(j) << "< \t" << tempString;
    }
}

void NetworkHandle::dumpMessageSampleData()
{
    unsigned int canIdentifier = 837;
    QString tempString;

    Message *msg = network.findMsgByID(canIdentifier);
    if (msg == NULL) {
        qDebug() << "msg not found";
        return;
    }
    for (auto ci = msg->m_signals.constBegin();
        ci != msg->m_signals.constEnd();
        ci++)
    {
        qDebug() << ci.value().name << ", startBit: " << ci.value().startBit << ", bitSize: " << ci.value().bitSize;
    }

    QByteArray canData;
    canData.reserve(msg->size);
    canData.resize(msg->size);

    qDebug() << "msg->size = " << msg->size << ", canData.length = " << canData.length();
    //network.messages[canIdentifier].signals["multiplexor"].encode(canData, 0);
    network.messages[canIdentifier].m_signals["RrWndDfgInhRq"].encode((uint8_t *)canData.data(), 0x01);
    network.messages[canIdentifier].m_signals["RrWndDfgSwAtv"].encode((uint8_t *)canData.data(), 0x01);

    qDebug() << "msg id = 0x" + QString::number(canIdentifier, 16).toUpper() + ", msg name = " + network.messages[canIdentifier].name;
    qDebug() << "msg id raw = 0x" + QString::number(((canIdentifier >> 8)& 0x7), 16).toUpper() + " 0x" + QString::number(((canIdentifier)& 0xff), 16).toUpper();

    dumpRawBits(canData);
}

void NetworkHandle::dumpMessageSampleData_2()
{
    unsigned int canIdentifier = 400;
    QString tempString;

    Message *msg = network.findMsgByID(canIdentifier);
    if (msg == NULL) {
        qDebug() << "msg not found";
        return;
    }
    for (auto ci = msg->m_signals.constBegin();
        ci != msg->m_signals.constEnd();
        ci++)
    {
        qDebug() << ci.value().name << ", startBit: " << ci.value().startBit << ", bitSize: " << ci.value().bitSize;
    }

    QByteArray canData;
    canData.reserve(msg->size);
    canData.resize(msg->size);

    qDebug() << "msg->size = " << msg->size << ", canData.length = " << canData.length();
    //network.messages[canIdentifier].signals["multiplexor"].encode(canData, 0);
    network.messages[canIdentifier].m_signals["SIRDpl"].encode((uint8_t *)canData.data(), 0x01);
    network.messages[canIdentifier].m_signals["NotDrvSeatStat"].encode((uint8_t *)canData.data(), 0x03);
    network.messages[canIdentifier].m_signals["NotiEventCount"].encode((uint8_t *)canData.data(), 0x10);
    network.messages[canIdentifier].m_signals["NotSndRwLtSeatStat"].encode((uint8_t *)canData.data(), 0x02);

    qDebug() << "msg id = 0x" + QString::number(canIdentifier, 16).toUpper() + ", msg name = " + network.messages[canIdentifier].name;
    qDebug() << "msg id raw = 0x" + QString::number(((canIdentifier >> 8)& 0x7), 16).toUpper() + " 0x" + QString::number(((canIdentifier)& 0xff), 16).toUpper();

    dumpRawBits(canData);
}

void NetworkHandle::dumpMessageSampleData_3()
{
    unsigned int canIdentifier = 0x632;
    QString tempString;

    Message *msg = network.findMsgByID(canIdentifier);
    if (msg == NULL) {
        qDebug() << "msg not found";
        return;
    }
    for (auto ci = msg->m_signals.constBegin();
        ci != msg->m_signals.constEnd();
        ci++)
    {
        qDebug() << ci.value().name << ", startBit: " << ci.value().startBit << ", bitSize: " << ci.value().bitSize;
    }

    QByteArray canData;
    canData.reserve(msg->size);
    canData.resize(msg->size);

    qDebug() << "msg->size = " << msg->size << ", canData.length = " << canData.length();
    //network.messages[canIdentifier].signals["multiplexor"].encode(canData, 0);
    network.messages[canIdentifier].m_signals["SrcNodeID"].encode((uint8_t *)canData.data(), 0x25);
    network.messages[canIdentifier].m_signals["UDat"].encode((uint8_t *)canData.data(), 0xFFFFFFFF);

    qDebug() << "msg id = 0x" + QString::number(canIdentifier, 16).toUpper() + ", msg name = " + network.messages[canIdentifier].name;
    qDebug() << "msg id raw = 0x" + QString::number(((canIdentifier >> 8)& 0x7), 16).toUpper() + " 0x" + QString::number(((canIdentifier)& 0xff), 16).toUpper();

    dumpRawBits(canData);
}



}
}
