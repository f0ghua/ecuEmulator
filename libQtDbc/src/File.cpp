#include "File.h"

#include <QRegularExpression>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#define F_NO_DEBUG

namespace Vector {
namespace DBC {

File::File()
{
    /* nothing to do here */
    m_pNetHandle = new NetworkHandle();
    m_pNetwork = m_pNetHandle->getNetwork();
}

File::~File()
{
    /* nothing to do here */
    delete m_pNetHandle;
}

void File::readNodes(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;

    regex.setPattern("^BU\\_\\:(.*)");
    match = regex.match(line);
            //captured 1 = a list of node names separated by spaces. No idea how many yet
    if (match.hasMatch())
    {
        QStringList nodeStrings = match.captured(1).split(' ');
#ifndef F_NO_DEBUG
        qDebug() << "Found " << nodeStrings.count() << " node names";
#endif
        for (int i = 0; i < nodeStrings.count(); i++)
        {
                    //qDebug() << nodeStrings[i];
            if (nodeStrings[i].length() > 1)
            {
                Node node;
                node.name = nodeStrings[i];
                m_pNetwork->addNode(node);
            }
        }
    }
}

void File::readEnvironmentVariable(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;

    //EV_ IntVal2: 0 [0|0] "vol" 0 1 DUMMY_NODE_VECTOR1 Vector__XXX;
    regex.setPattern("^EV_ +(\\w+) *: *([01]) +\\[([\\d\\.\\+\\-eE]+)\\|([\\d\\.\\+\\-eE]+)\\] +\\\"(\\w*)\\\" +([\\d\\.\\+\\-eE]+) +(\\d+) +DUMMY_NODE_VECTOR(\\d+) +(.+) *;");
    match = regex.match(line);

    if (match.hasMatch())
    {
#if 0
        for (int i = 0; i <= match.lastCapturedIndex(); i++)
        {
            qDebug() << i << "<" << match.captured(i) << ">";
        }
#endif
    }
}

bool File::readValueDescriptionSignal(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;

    regex.setPattern("^VAL\\_ (\\d+) +(\\w+) +(.*) *;");
    match = regex.match(line);
    //captured 1 is the ID to match against
    //captured 2 is the signal name to match against
    //captured 3 is a series of values in the form (number "text") that is, all sep'd by spaces
    if (match.hasMatch())
    {
        Message *msg = m_pNetwork->findMsgByID(match.captured(1).toInt());
        if (msg != NULL)
        {
            Signal *sig = msg->findSignalByName(match.captured(2));
            if (sig != NULL)
            {
                QString tokenString = match.captured(3);

                while (tokenString.length() > 2)
                {
                    regex.setPattern("(\\d+) \\\"(.*?)\\\"(.*)");
                    match = regex.match(tokenString);
                    if (match.hasMatch())
                    {
                        sig->valueDescriptions.insert(
                            match.captured(1).toInt(),
                            match.captured(2));
#ifndef F_NO_DEBUG
                        qDebug() << "sig val " << match.captured(1).toInt() << ", desc " << match.captured(2);
#endif
                        int rightSize = tokenString.length() - match.captured(1).length() - match.captured(2).length() - 4;
                        if (rightSize > 0) tokenString = tokenString.right(rightSize);
                        else tokenString = "";
                                //qDebug() << "New token string: " << tokenString;
                    }
                    else tokenString = "";
                }

                return true;
            }
        }
    }

    return false;
}

bool File::readValueDescriptionEnvironmentVariable(QString &)
{
    return false;
}

void File::readValueDescription(QString &line)
{
    // for signal
    if (readValueDescriptionSignal(line)) {
        return;
    }

    // for environment variable
    if (readValueDescriptionEnvironmentVariable(line)) {
        return;
    }
}

void File::readMessageTransmitter(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;

    regex.setPattern("^BO\\_TX\\_BU\\_ (\\w+) *: (.*);");
    match = regex.match(line);

    if (match.hasMatch())
    {
        unsigned int id = match.captured(1).toUInt();
        QStringList nodeStrings = match.captured(2).split(',');
#ifndef F_NO_DEBUG
        qDebug() << "Found " << nodeStrings.count() << " tx node";
#endif
        Message *msg = m_pNetwork->findMsgByID(id);
        if (msg != NULL)
        {
            for (int i = 0; i < nodeStrings.count(); i++)
            {
                msg->transmitters.append(nodeStrings[i]);
            }
            // set the transmitter to empty
            msg->transmitter = QString();
        }
    }
}

void File::readAttributeDefinition(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;

    regex.setPattern("^BA_DEF_ ((?:BU_|BO_|SG_|EV_)?) *\\\"([A-Za-z0-9\\-_]+)\\\" +((?:INT|HEX|FLOAT|STRING|ENUM)) *(.+);");
    match = regex.match(line);
#if 0
    if (match.hasMatch())
    {
        for (int i = 0; i <= match.lastCapturedIndex(); i++)
        {
            qDebug() << i << "<" << match.captured(i) << ">";
        }
    }
#endif
    if (!match.hasMatch()) return;

    QString objectType = match.captured(1);
    QString attributeName = match.captured(2);
    QString attributeValueType = match.captured(3);
    QStringList valueStringList;
    if (attributeValueType != "ENUM") {
        valueStringList = match.captured(4).split(" ");
    }

    AttributeDefinition & ad = m_pNetwork->attributeDefinitions[attributeName];

    ad.name = attributeName;

    if (objectType == "") {
        ad.objectType = AttributeDefinition::ObjectType::Network;
    } else
    if (objectType == "BU_") {
        ad.objectType = AttributeDefinition::ObjectType::Node;
    } else
    if (objectType == "BO_") {
        ad.objectType = AttributeDefinition::ObjectType::Message;
    } else
    if (objectType == "SG_") {
        ad.objectType = AttributeDefinition::ObjectType::Signal;
    } else
    if (objectType == "EV_") {
        ad.objectType = AttributeDefinition::ObjectType::EnvironmentVariable;
    }

    if (attributeValueType == "INT") {
        ad.valueType = AttributeValueType::Int;
        ad.minimumIntegerValue = valueStringList[0].toInt();
        ad.maximumIntegerValue = valueStringList[1].toInt();
    } else
    if (attributeValueType == "HEX") {
        bool ok;
        ad.valueType = AttributeValueType::Hex;
        ad.minimumHexValue = valueStringList[0].toInt(&ok, 10);
        ad.maximumHexValue = valueStringList[1].toInt(&ok, 10);
    } else
    if (attributeValueType == "FLOAT") {
        ad.valueType = AttributeValueType::Float;
        ad.minimumFloatValue = valueStringList[0].toDouble();
        ad.maximumFloatValue = valueStringList[1].toDouble();
    } else
    if (attributeValueType == "STRING") {
        ad.valueType = AttributeValueType::String;
    } else
    if (attributeValueType == "ENUM") {
        ad.valueType = AttributeValueType::Enum;
        valueStringList = match.captured(4).split(",");
        for (int i = 0; i < valueStringList.size(); i++)
            ad.enumValues << valueStringList[i].split("\"", QString::SkipEmptyParts);
    }

}

void File::readAttributeDefault(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;
    bool ok;

    regex.setPattern("^BA_DEF_DEF_ +\\\"([A-Za-z0-9\\-_]+)\\\" +(.*) *;");
    match = regex.match(line);
#if 0
    if (match.hasMatch())
    {
        for (int i = 0; i <= match.lastCapturedIndex(); i++)
        {
            qDebug() << i << "<" << match.captured(i) << ">";
        }
        return;
    }
#endif
    if (!match.hasMatch()) return;

    QString attributeName = match.captured(1);
    QString attributeValue = match.captured(2);

    AttributeDefinition *pAdfn = m_pNetwork->findAttributeDefinitionByName(attributeName);
    if (pAdfn == NULL) return;

    Attribute &adft = m_pNetwork->attributeDefaults[attributeName];
    adft.name = attributeName;
    adft.valueType = pAdfn->valueType;
    adft.stringValue = attributeValue;
    switch (pAdfn->valueType)
    {
        // Integer
        case AttributeValueType::Int:
            adft.integerValue = attributeValue.toInt();
            break;

        // Hexadecimal
        case AttributeValueType::Hex:
            adft.hexValue = attributeValue.toInt(&ok, 10);
            break;

        // Float
        case AttributeValueType::Float:
            adft.floatValue = attributeValue.toDouble();
            break;

        // String
        case AttributeValueType::String:
            adft.stringValue = attributeValue.replace("\"", "");
            break;

        // Enumeration
        case AttributeValueType::Enum:
            adft.stringValue = attributeValue.replace("\"", "");
            break;
    }
}

/* Attribute Values (BA) for Node (BU) */
bool File::readAttributeValueNode(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;
    bool ok;

    regex.setPattern("^BA_ +\\\"([A-Za-z0-9\\-_]+)\\\" +BU_ +(\\w+) +(.*) *;");
    match = regex.match(line);
    if (!match.hasMatch()) return false;

    QString attributeName = match.captured(1);
    QString nodeName = match.captured(2);
    QString attributeValue = match.captured(3);

    AttributeDefinition *pAdfn = m_pNetwork->findAttributeDefinitionByName(attributeName);
    if (pAdfn == NULL) {
        qDebug() << "readAttributeValueNode: no definition found of " << attributeName;
        return false;
    }

    Node *pNode = m_pNetwork->findNodeByName(nodeName);
    if (pNode == NULL) {
        qDebug() << "readAttributeValueNode: no node found of " << nodeName;
        return false;
    }

    Attribute &attribute = pNode->attributeValues[attributeName];

    /* Name */
    attribute.name = attributeName;
    /* Value Type */
    attribute.valueType = pAdfn->valueType;
    /* Value */
    switch(attribute.valueType)
    {
        // Integer
        case AttributeValueType::Int:
            attribute.integerValue = attributeValue.toInt();
            break;

        // Hexadecimal
        case AttributeValueType::Hex:
            attribute.hexValue = attributeValue.toInt(&ok, 10);
            break;

        // Float
        case AttributeValueType::Float:
            attribute.floatValue = attributeValue.toDouble();
            break;

        // String
        case AttributeValueType::String:
            attribute.stringValue = attributeValue.replace("\"", "");
            break;

        // Enumeration
        case AttributeValueType::Enum:
            attribute.enumValue = attributeValue.toInt();
            break;
    }

    return true;
}

/* Attribute Values (BA) for Message (BO) */
bool File::readAttributeValueMessage(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;
    bool ok;

    regex.setPattern("^BA_ +\\\"([A-Za-z0-9\\-_]+)\\\" +BO_ +(\\d+) +(.*) *;");
    match = regex.match(line);
    if (!match.hasMatch()) return false;

    QString attributeName = match.captured(1);
    unsigned int id = match.captured(2).toUInt();
    QString attributeValue = match.captured(3);

    AttributeDefinition *pAdfn = m_pNetwork->findAttributeDefinitionByName(attributeName);
    if (pAdfn == NULL) return false;

    Message *pMsg = m_pNetwork->findMsgByID(id);
    if (pMsg == NULL) return false;

    Attribute &attribute = pMsg->attributeValues[attributeName];

    /* Name */
    attribute.name = attributeName;
    /* Value Type */
    attribute.valueType = pAdfn->valueType;
    /* Value */
    switch(attribute.valueType)
    {
        // Integer
        case AttributeValueType::Int:
            attribute.integerValue = attributeValue.toInt();
            break;

        // Hexadecimal
        case AttributeValueType::Hex:
            attribute.hexValue = attributeValue.toInt(&ok, 10);
            break;

        // Float
        case AttributeValueType::Float:
            attribute.floatValue = attributeValue.toDouble();
            break;

        // String
        case AttributeValueType::String:
            attribute.stringValue = attributeValue.replace("\"", "");
            break;

        // Enumeration
        case AttributeValueType::Enum:
            attribute.enumValue = attributeValue.toInt();
            break;
    }

    return true;
}

bool File::readAttributeValueSignal(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;
    bool ok;

    regex.setPattern("^BA_ +\\\"([A-Za-z0-9\\-_]+)\\\" +SG_ +(\\d+) +(\\w+) +(.*) *;");
    match = regex.match(line);
    if (!match.hasMatch()) return false;

    QString attributeName = match.captured(1);
    unsigned int id = match.captured(2).toUInt();
    QString signalName = match.captured(3);
    QString attributeValue = match.captured(4);

    AttributeDefinition *pAdfn = m_pNetwork->findAttributeDefinitionByName(attributeName);
    if (pAdfn == NULL) return false;

    Message *pMsg = m_pNetwork->findMsgByID(id);
    if (pMsg == NULL) return false;

    Signal *pSig = pMsg->findSignalByName(signalName);
    if (pSig == NULL) return false;
    Attribute &attribute = pSig->attributeValues[attributeName];

    /* Name */
    attribute.name = attributeName;
    /* Value Type */
    attribute.valueType = pAdfn->valueType;
    /* Value */
    switch(attribute.valueType)
    {
        // Integer
        case AttributeValueType::Int:
            attribute.integerValue = attributeValue.toInt();
            break;

        // Hexadecimal
        case AttributeValueType::Hex:
            attribute.hexValue = attributeValue.toInt(&ok, 10);
            break;

        // Float
        case AttributeValueType::Float:
            attribute.floatValue = attributeValue.toDouble();
            break;

        // String
        case AttributeValueType::String:
            attribute.stringValue = attributeValue.replace("\"", "");
            break;

        // Enumeration
        case AttributeValueType::Enum:
            attribute.enumValue = attributeValue.toInt();
            break;
    }

    return true;
}

/* Attribute Values (BA) for Environment Variable (EV) */
bool File::readAttributeValueEnvironmentVariable(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;
    bool ok;

    regex.setPattern("^BA_ +\\\"([A-Za-z0-9\\-_]+)\\\" +EV_ +(\\w+) +(.*) *;");
    match = regex.match(line);
    if (!match.hasMatch()) return false;

    QString attributeName = match.captured(1);
    QString envVarName = match.captured(2);
    QString attributeValue = match.captured(3);

    AttributeDefinition *pAdfn = m_pNetwork->findAttributeDefinitionByName(attributeName);
    if (pAdfn == NULL) return false;

    EnvironmentVariable *pEnv = m_pNetwork->findEnvrionmentVariablebyName(envVarName);
    if (pEnv == NULL) return false;

    Attribute &attribute = pEnv->attributeValues[attributeName];

    /* Name */
    attribute.name = attributeName;
    /* Value Type */
    attribute.valueType = pAdfn->valueType;
    /* Value */
    switch(attribute.valueType)
    {
        // Integer
        case AttributeValueType::Int:
            attribute.integerValue = attributeValue.toInt();
            break;

        // Hexadecimal
        case AttributeValueType::Hex:
            attribute.hexValue = attributeValue.toInt(&ok, 10);
            break;

        // Float
        case AttributeValueType::Float:
            attribute.floatValue = attributeValue.toDouble();
            break;

        // String
        case AttributeValueType::String:
            attribute.stringValue = attributeValue.replace("\"", "");
            break;

        // Enumeration
        case AttributeValueType::Enum:
            attribute.enumValue = attributeValue.toInt();
            break;
    }

    return true;
}

/* Attribute Values (BA) for Network */
bool File::readAttributeValueNetwork(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;
    bool ok;

    regex.setPattern("^BA_ +\\\"([A-Za-z0-9\\-_]+)\\\" +(.*) *;");
    match = regex.match(line);
    if (!match.hasMatch()) return false;

    QString attributeName = match.captured(1);
    QString attributeValue = match.captured(2);

    AttributeDefinition *pAdfn = m_pNetwork->findAttributeDefinitionByName(attributeName);
    if (pAdfn == NULL) return false;

    Attribute &attribute = m_pNetwork->attributeValues[attributeName];

    /* Name */
    attribute.name = attributeName;
    /* Value Type */
    attribute.valueType = pAdfn->valueType;
    /* Value */
    switch(attribute.valueType)
    {
        // Integer
        case AttributeValueType::Int:
            attribute.integerValue = attributeValue.toInt();
            break;

        // Hexadecimal
        case AttributeValueType::Hex:
            attribute.hexValue = attributeValue.toInt(&ok, 10);
            break;

        // Float
        case AttributeValueType::Float:
            attribute.floatValue = attributeValue.toDouble();
            break;

        // String
        case AttributeValueType::String:
            attribute.stringValue = attributeValue.replace("\"", "");
            if (attributeName == "DBName")
            {
                m_pNetwork->name = attribute.stringValue;
            }
            break;

        // Enumeration
        case AttributeValueType::Enum:
            attribute.enumValue = attributeValue.toInt();
            break;
    }

    return true;
}

void File::readAttributeValue(QString &line)
{
    // for nodes (BU)
    if (readAttributeValueNode(line)) {
        return;
    }

    // for messages (BO)
    if (readAttributeValueMessage(line)) {
        return;
    }

    // for signal (SG)
    if (readAttributeValueSignal(line)) {
        return;
    }

    // for environment variables (EV)
    if (readAttributeValueEnvironmentVariable(line)) {
        return;
    }

    // for network
    if (readAttributeValueNetwork(line)) {
        return;
    }
}

void File::readSignalGroup(QString &line)
{
    QRegularExpression regex;
    QRegularExpressionMatch match;

    regex.setPattern("^SIG_GROUP_ +(\\d+) +(\\w+) +(\\d+) *: *(.*) *;");
    match = regex.match(line);
    if (match.hasMatch())
    {
        unsigned int id = match.captured(1).toUInt();
        QString sigGroupName = match.captured(2);
        SignalGroup &signalGroup = m_pNetwork->messages[id].signalGroups[sigGroupName];

        /* Message Identifier */
        signalGroup.messageId = id;
        /* Name */
        signalGroup.name = sigGroupName;
        /* Repetitions */
        signalGroup.repetitions = match.captured(3).toUInt();
        /* Signals */
        signalGroup.signal = match.captured(4).split(" ");
    }
}

Status File::load(const QString &fileName)
{
    QFile *inFile = new QFile(fileName);
    QString line;
    QRegularExpression regex;
    QRegularExpressionMatch match;
    Message *currentMessage = NULL;
    int numSigFaults = 0, numMsgFaults = 0;

#ifndef F_NO_DEBUG
    qDebug() << "DBC File: " << fileName;
#endif

    if (!inFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        delete inFile;
        return Status::FileOpenError;
    }

    m_fileName = fileName;
#ifndef F_NO_DEBUG
    qDebug() << "Starting DBC load";
#endif
    m_pNetwork->nodes.clear();
    m_pNetwork->removeAllMessages();
    QFileInfo fileinfo = QFileInfo(fileName);
    m_pNetwork->name = fileinfo.baseName();

#if 0
    Node falseNode;
    falseNode.name = "Vector__XXX";
    falseNode.comment = "Default node if none specified";
    m_pNetwork->addNode(falseNode);
#endif

    while (!inFile->atEnd())
    {
        line = QString(inFile->readLine().simplified());
        if (line.startsWith("BO_ ")) //defines a message
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BO line";
#endif
            regex.setPattern("^BO\\_ (\\w+) (\\w+) *: (\\w+) (\\w+)");
            match = regex.match(line);
            // captured 1 = the ID in decimal
            // captured 2 = The message name
            // captured 3 = the message length
            // captured 4 = the NODE responsible for this message
            if (match.hasMatch())
            {
                Message msg;
                msg.id = match.captured(1).toUInt(); //the ID is always stored in decimal format
                msg.name = match.captured(2);
                msg.size = match.captured(3).toInt();
                msg.transmitter = match.captured(4);
#ifndef F_NO_DEBUG
                qDebug() << QString("id = %1, name = %2, size = %3, tr = %4").arg(msg.id).arg(msg.name).arg(msg.size).arg(msg.transmitter);
#endif
                m_pNetwork->addMessage(msg);
                currentMessage = m_pNetwork->findMsgByID(msg.id);
            }
            else numMsgFaults++;
        }
        if (line.startsWith("SG_ ")) //defines a signal
        {
            int offset = 0;
            //bool isMultiplexor = false;
            //bool isMultiplexed = false;
            Signal sig;

            sig.multiplexerSwitchValue = 0;
            sig.multiplexedSignal = false;
            sig.multiplexorSwitch = false;

#ifndef F_NO_DEBUG
            qDebug() << "Found a SG line";
#endif

#if 0
            regex.setPattern("^SG\\_ *(\\w+) *(\\(M|m\\)?) *: *(\\d+)\\|(\\d+)@(\\d+)([\\+|\\-]) \\(([0-9.+\\-eE]+),([0-9.+\\-eE]+)\\) \\[([0-9.+\\-eE]+)\\|([0-9.+\\-eE]+)\\] \\\"(.*)\\\" (.*)");
            match = regex.match(line);
            if (match.hasMatch())
            {
                if (currentMessage->id == 1300)
                for (int i = 0; i <= match.lastCapturedIndex(); i++)
                {
                    qDebug() << i << "<" << match.captured(i) << ">";
                }
            }
#endif
            regex.setPattern("^SG\\_ *(\\w+) +M *: *(\\d+)\\|(\\d+)@(\\d+)([\\+|\\-]) \\(([0-9.+\\-eE]+),[ ]*([0-9.+\\-eE]+)\\) \\[([0-9.+\\-eE]+)\\|([0-9.+\\-eE]+)\\] \\\"(.*)\\\" (.*)");

            match = regex.match(line);
            if (match.hasMatch())
            {
#ifndef F_NO_DEBUG
                qDebug() << "Multiplexor signal";
#endif
                //isMultiplexor = true;
                sig.multiplexorSwitch = true;
            }
            else
            {
                regex.setPattern("^SG\\_ *(\\w+) +m(\\d+) *: *(\\d+)\\|(\\d+)@(\\d+)([\\+|\\-]) \\(([0-9.+\\-eE]+),[ ]*([0-9.+\\-eE]+)\\) \\[([0-9.+\\-eE]+)\\|([0-9.+\\-eE]+)\\] \\\"(.*)\\\" (.*)");
                match = regex.match(line);
                if (match.hasMatch())
                {
#ifndef F_NO_DEBUG
                    qDebug() << "Multiplexed signal";
#endif
                    //isMultiplexed = true;
                    sig.multiplexedSignal = true;
                    sig.multiplexerSwitchValue = match.captured(2).toInt();
                    offset = 1;
                }
                else
                {
                    regex.setPattern("^SG\\_ *(\\w+) *: *(\\d+)\\|(\\d+)@(\\d+)([\\+|\\-]) \\(([0-9.+\\-eE]+),[ ]*([0-9.+\\-eE]+)\\) \\[([0-9.+\\-eE]+)\\|([0-9.+\\-eE]+)\\] \\\"(.*)\\\" (.*)");
                    match = regex.match(line);
                    sig.multiplexedSignal = false;
                    sig.multiplexorSwitch = false;
                }
            }

            //captured 1 is the signal name
            //captured 2 would be multiplex value if this is a multiplex signal. Then offset the rest of these by 1
            //captured 2 is the starting bit
            //captured 3 is the length in bits
            //captured 4 is the byte order / value type
            //captured 5 specifies signed/unsigned for ints
            //captured 6 is the scaling factor
            //captured 7 is the offset
            //captured 8 is the minimum value
            //captured 9 is the maximum value
            //captured 10 is the unit
            //captured 11 is the receiving node

            if (match.hasMatch())
            {
#ifndef F_NO_DEBUG
                qDebug() << "Normal signal, name " << match.captured(1);
#endif
                sig.name = match.captured(1);
                sig.startBit = match.captured(2 + offset).toInt();
                sig.bitSize = match.captured(3 + offset).toInt();
                int val = match.captured(4 + offset).toInt();
                if (val < 2)
                {
                    if (match.captured(5 + offset) == "+") sig.valueType = ValueType::Unsigned;
                    else sig.valueType = ValueType::Signed;
                }
                switch (val)
                {
                case 0: //big endian mode
                    sig.byteOrder = ByteOrder::Motorola;
                    break;
                case 1: //little endian mode
                    sig.byteOrder = ByteOrder::Intel;
                    break;
                case 2:
                    sig.valueType = ValueType::SPFloat;
                    break;
                case 3:
                    sig.valueType = ValueType::DPFloat;
                    break;
                case 4:
                    sig.valueType = ValueType::String;
                    break;
                }
                sig.factor = match.captured(6 + offset).toDouble();
                sig.offset = match.captured(7 + offset).toDouble();
                sig.minimumPhysicalValue = match.captured(8 + offset).toDouble();
                sig.maximumPhysicalValue = match.captured(9 + offset).toDouble();
                sig.unit = match.captured(10 + offset);
                //if (match.captured(11 + offset).contains(','))
                if (match.captured(11 + offset) != "Vector__XXX")
                {
                    sig.receivers = match.captured(11 + offset).split(',');
                }

                sig.parentMessage = currentMessage;
                currentMessage->addSignal(sig);
#ifdef SUPPORT_NETWORK_SIGNAL
                m_pNetwork->addSignalPointer(currentMessage->findSignalByName(sig.name));
#endif
                if (sig.multiplexorSwitch)
                    currentMessage->multiplexorSignal = currentMessage->findSignalByName(sig.name);
            }
            else numSigFaults++;

        }
        if (line.startsWith("BU_:")) //line specifies the nodes on this canbus
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BU line";
#endif
            readNodes(line);
        }
        if (line.startsWith("EV_")) /* Environment Variables (EV) */
        {
            readEnvironmentVariable(line);
        }
        if (line.startsWith("CM_ SG_ "))
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found an SG comment line";
#endif
            regex.setPattern("^CM\\_ SG\\_ *(\\w+) *(\\w+) *\\\"(.*)\\\";");
            match = regex.match(line);
            //captured 1 is the ID to match against to get to the message
            //captured 2 is the signal name from that message
            //captured 3 is the comment itself
            if (match.hasMatch())
            {
                //qDebug() << "Comment was: " << match.captured(3);
                Message *msg = m_pNetwork->findMsgByID(match.captured(1).toInt());
                if (msg != NULL)
                {
                    Signal *sig = msg->findSignalByName(match.captured(2));
                    if (sig != NULL)
                    {
                        sig->comment = match.captured(3);
                    }
                }
            }
        }
        if (line.startsWith("CM_ BO_ "))
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BO comment line";
#endif
            regex.setPattern("^CM\\_ BO\\_ *(\\w+) *\\\"(.*)\\\";");
            match = regex.match(line);
            //captured 1 is the ID to match against to get to the message
            //captured 2 is the comment itself
            if (match.hasMatch())
            {
                //qDebug() << "Comment was: " << match.captured(2);
                Message *msg = m_pNetwork->findMsgByID(match.captured(1).toInt());
                if (msg != NULL)
                {
                    msg->comment = match.captured(2);
                }
            }
        }
        if (line.startsWith("CM_ BU_ "))
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BU comment line";
#endif
            regex.setPattern("^CM\\_ BU\\_ *(\\w+) *\\\"(.*)\\\";");
            match = regex.match(line);
            //captured 1 is the Node name
            //captured 2 is the comment itself
            if (match.hasMatch())
            {
                //qDebug() << "Comment was: " << match.captured(2);
                Node *node = m_pNetwork->findNodeByName(match.captured(1));
                if (node != NULL)
                {
                    node->comment = match.captured(2);
                }
            }
        }
        if (line.startsWith("VAL_ "))
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a value description line";
#endif
            readValueDescription(line);
        }
        if (line.startsWith("BO_TX_BU_")) //
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BO_TX_BU_ line";
#endif
            readMessageTransmitter(line);
        }
#if 1 // later to move to phase 2 load
        if (line.startsWith("BA_DEF_")) //Attribute Definitions (BA_DEF)
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BA_DEF_ line";
#endif
            readAttributeDefinition(line);
        }
        if (line.startsWith("BA_DEF_DEF_")) //Attribute Defaults (BA_DEF_DEF)
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BA_DEF_DEF_ line";
#endif
            readAttributeDefault(line);
        }
        if (line.startsWith("BA_")) //Attribute Values (BA)
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BA_ line";
#endif
            readAttributeValue(line);
        }
#endif
        if (line.startsWith("SIG_GROUP_")) //Signal Groups (SIG_GROUP)
        {
#ifndef F_NO_DEBUG
            qDebug() << "Found a BA_ line";
#endif
            readSignalGroup(line);
        }
    }

#if 0
    if (numSigFaults > 0 || numMsgFaults > 0)
    {
        QMessageBox msgBox;
        QString msg = "DBC file loaded with errors!\n";
        msg += "Number of faulty message entries: " + QString::number(numMsgFaults) + "\n";
        msg += "Number of faulty signal entries: " + QString::number(numSigFaults) + "\n\n";
        msg += "Faulty entries have not been loaded.";
        msgBox.setText(msg);
        msgBox.exec();
    }
#endif

    inFile->close();
    delete inFile;

    return Status::Ok;
}




}
}
