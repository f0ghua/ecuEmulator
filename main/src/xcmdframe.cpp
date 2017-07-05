#include "utils.h"
#include "xcmdframe.h"

XCmdFrame::XCmdFrame(const QByteArray &data):
    XBusFrame(data, XBusFrame::ParseOnlyCommand)
{

}

struct operationWarningCodeMapping_t {
	quint8 dataMin;
	quint8 dataMax;
	QString description;
};
static const struct operationWarningCodeMapping_t g_OWCMapping[] = {
	{0x01, 0x01, QStringLiteral("Loop overrun detected")},
	{0x02, 0x02, QStringLiteral("USB buffer full detected")},
    {0x03, 0x03, QStringLiteral("Escape sequence error")},
    {0x04, 0x04, QStringLiteral("Message Too Long")},
    {0x05, 0x05, QStringLiteral("Message Buffer Full")},
    {0x06, 0x06, QStringLiteral("Message Truncated")},
    {0x08, 0x0F, QStringLiteral("Configuration setting warning")},
    {0x10, 0x17, QStringLiteral("Keyword 82 operation warning")},
    {0x18, 0x1F, QStringLiteral("Keyword 71 operation warning")},
    {0x20, 0x27, QStringLiteral("IIC operation warning")},
    {0x28, 0x2F, QStringLiteral("Keyword 2000 operation warning")},
    {0x30, 0x37, QStringLiteral("IDB operation warning")},
    {0x38, 0x3F, QStringLiteral("ACP operation warning")},
    {0x40, 0x47, QStringLiteral("E&C operation warning")},
    {0x48, 0x4F, QStringLiteral("J1708 operation warning")},
    {0x50, 0x50, QStringLiteral("The CAN1 TX buffer has overrun and CAN frames have been discarded")},
    {0x51, 0x51, QStringLiteral("The CAN1 RX buffer has overrun and CAN frames have been discarded")},
    {0x52, 0x57, QStringLiteral("reserved for CAN 1")},
    {0x58, 0x58, QStringLiteral("The CAN2 TX buffer has overrun and CAN frames have been discarded")},
    {0x59, 0x59, QStringLiteral("The CAN2 RX buffer has overrun and CAN frames have been discarded")},
    {0x5A, 0x5F, QStringLiteral("reserved for CAN 2")},
    {0x60, 0x67, QStringLiteral("Class 2 operation warning")},
    {0x68, 0x6F, QStringLiteral("AOS operation warning")},
    {0x70, 0x77, QStringLiteral("SPI operation warning")},
    {0x78, 0x7F, QStringLiteral("Not Used")},
    {0x80, 0x87, QStringLiteral("UART operation warning")},
    {0x88, 0x8F, QStringLiteral("J1850 operation warning")},
    {0x90, 0x97, QStringLiteral("BEAN1 operation warning")},
    {0x98, 0x9F, QStringLiteral("BEAN2 operation warning")},
    {0xA0, 0xA7, QStringLiteral("Not Used")},
    {0xA8, 0xAF, QStringLiteral("Not Used")},
    {0xB0, 0xB7, QStringLiteral("IE bus operation warning")},
    {0xB8, 0xBF, QStringLiteral("LIN operation warning")},
    {0xC0, 0xC0, QStringLiteral("CAN packet buffer busy - rx message dropped (ISO15765-2)")},
    {0xC1, 0xC1, QStringLiteral("TX FIFO full - CAN packet message dropped (can't send FC - ISO15765-2)")},
    {0xC2, 0xC7, QStringLiteral("reserved")},
    {0xC8, 0xCF, QStringLiteral("reserved")},
    {0xD0, 0xD7, QStringLiteral("Not Used")},
    {0xD8, 0xDF, QStringLiteral("Not Used")},
    {0xE0, 0xE7, QStringLiteral("Not Used")},
    {0xE8, 0xEF, QStringLiteral("Not Used")},
    {0xF0, 0xF7, QStringLiteral("Not Used")},
	{0xF8, 0xFF, QStringLiteral("reserved")}
};

static QString getOperationWarningCodeStr(quint8 code)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_OWCMapping); i++) {
		if ((g_OWCMapping[i].dataMin <= code)
			&& (g_OWCMapping[i].dataMax >= code))
			return g_OWCMapping[i].description;
	}

    return QStringLiteral("Unknown");
}

struct crcErrorCodeMapping_t {
	quint8 error;
	QString description;
};

static const struct crcErrorCodeMapping_t g_crcECMapping[] = {
	{0x00, QStringLiteral("Emulation is executing or CRC calculation is executing ")},
	{0x01, QStringLiteral("Undefined SD card error")},
	{0x02, QStringLiteral("No such file or directory - SD card")},
	{0x05, QStringLiteral("I/O Error - SD card")},
	{0x09, QStringLiteral("Bad file number - SD card")},
	{0x0D, QStringLiteral("Permission denied - SD card")},
	{0x11, QStringLiteral("File Exists - SD card")},
	{0x13, QStringLiteral("No such device - SD card")},
	{0x16, QStringLiteral("Invalid Argument - SD card")},
	{0x18, QStringLiteral("Too many files open - SD card")},
	{0x1C, QStringLiteral("No space left on device - SD card")},
	{0x1E, QStringLiteral("Read only file system (Sharing error) - SD card")},
	{0x20, QStringLiteral("Buffer is busy")}
};

static QString getCRCErrorStr(const QByteArray &data)
{
	if ((data.count() != 2) || ((quint8)data.at(0) != 0xEE))
		return QString();

	quint8 error = data.at(1);
    for (quint8 i = 0; i < ARRAY_SIZE(g_crcECMapping); i++) {
		if (g_crcECMapping[i].error == error)
			return g_crcECMapping[i].description;
	}

    return QStringLiteral("Unknown Error");
}

struct baudRateMapping_t {
	int index;
	int baudrate;
	const char data[2];
};

static const struct baudRateMapping_t g_baudRateMapping[] = {
	{0, 33482, 	{(char)0x8D, (char)0xAF}},
	{1, 83333, 	{(char)0xB1, (char)0x2D}},
	{0, 95200, 	{(char)0xF0, (char)0x3A}},
	{1, 100000, {(char)0xF1, (char)0x39}},
	{0, 125000, {(char)0xDD, (char)0x3E}},
	{1, 200000, {(char)0xD8, (char)0x39}},
	{0, 250000, {(char)0xCE, (char)0x3E}},
	{1, 500000, {(char)0xC9, (char)0x39}}
};

QByteArray XCmdFrame::getBaudRateDataByValue(int baudrate)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_baudRateMapping); i++) {
		if (g_baudRateMapping[i].baudrate == baudrate)
			return QByteArray::fromRawData(g_baudRateMapping[i].data, 2);
	}

    return QByteArray();
}

int XCmdFrame::getBaudRateValueByData(quint16 data)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_baudRateMapping); i++) {
    	quint16 v = ((g_baudRateMapping[i].data[0])<<8) | (g_baudRateMapping[i].data[1]&0xFF);   	
		if (v == data)
			return g_baudRateMapping[i].baudrate;
	}

    return -1;
}

struct linVersionMapping_t {
	int version;
	QString description;
};

static const struct linVersionMapping_t g_linVersionMapping[] = {
	{0x13, QStringLiteral("LIN ver 1.3")},
    {0x20, QStringLiteral("LIN ver 2.0")},
    {0x80, QStringLiteral("SAE J2602 ver 0.0")},
    {0x92, QStringLiteral("SAE J2602 ver 1.2")}
};

QString XCmdFrame::getLinVersionDescription(quint8 version)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_linVersionMapping); i++) {
		if (version == g_linVersionMapping[i].version)
			return g_linVersionMapping[i].description;
	}

    return QString();
}

quint8 XCmdFrame::getLinVersionByDescription(QString s)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_linVersionMapping); i++) {
		if (s == g_linVersionMapping[i].description)
			return g_linVersionMapping[i].version;
	}

    return 0;
}

QString XCmdFrame::linVersion2String(quint8 version)
{
    QString s;

    s.clear();
    if ((version >> 7)&0x01) {
        s.append("SAE J2602");
    } else {
        s.append("LIN");
    }

    s += QString(" ver %1.%2").arg((version>>4)&0x07).arg(version&0x0F);

    return s;
}

int XCmdFrame::parseCmdResponseSSBusProtocol(quint8 *protocols, int maxNumber)
{
    if (m_data.isEmpty())
        return 0;

    int dataLen = m_data.count() - (hasTimeStamp()?2:0);

    for (int i = 0; i < maxNumber; ++i) {
        if (i < dataLen) {
            *protocols = (quint8)m_data[i];
        } else {
            *protocols = 0;
        }
        ++protocols;
    }

    return 0;
}

int XCmdFrame::parseCmdResponsePeriodicMessage(int *index, quint8 *enable, quint16 *period, 
    quint16 *delay, quint8 *header, QByteArray *data)
{
    // at least 5 bytes (1 byte status + 2 bytes time + 1 bytes header + 1 bytes m_data
    if (m_data.count() < 5) 
        return -1;

    int action = m_data.at(0);
    if ((action != 0x00) && (action != 0x01))
        return -1;
    
    *index = cmdId() - 0x70;
    *enable = m_data.at(0)?0:1;
    *period = ((m_data.at(1)&0xFF)<<8) + (m_data.at(2)&0xFF);
    *delay = 0;
    *header = m_data.at(3);
    *data = m_data.mid(4);

    return 0;
}

int XCmdFrame::parseCmdResponseGetFrequency(quint8 *busType, quint8 *busIndex, quint16 *data)
{
    if (m_data.count() < 2) 
        return -1;    

    *busType = m_protocolId;
    *busIndex = m_bus;
    *data = ((m_data.at(0)&0xFF)<<8) + (m_data.at(1)&0xFF);

    return 0;
}

int XCmdFrame::parseCmdResponseGetCanTransCtrl(quint8 *busIndex, quint8 *type, quint8 *mode)
{
    if (m_data.count() < 2) 
        return -1;    

    *busIndex = m_bus;
    *type = m_data.at(0);
    *mode = m_data.at(1);

    return 0;
}

int XCmdFrame::parseCmdResponseGetLinVersion(quint8 *version)
{
    int minLen = hasTimeStamp()?3:1; // 1 data + 2 timestamp
    if (m_data.count() < minLen) 
        return -1;    
        
    *version = m_data.at(0);
    return 0;
}

int XCmdFrame::parseCmdResponseGetBaudrate(quint32 *baudrate)
{
    quint32 v = 0;
    int minLen = hasTimeStamp()?3:1;
    if (m_data.count() < minLen) {
       v = 19200;
    } else if (m_data.count() >= (minLen+1)) { // 2 bytes data
        QByteArray ba = m_data.mid(0, 2);
        v = Utils::Base::bcd2Dec(ba);
    } else { // 1 byte
        QByteArray ba = m_data.mid(0, 1);
        v = Utils::Base::bcd2Dec(ba);
    }

    *baudrate = v * 100;
    
    return 0;
}

int XCmdFrame::parseCmdResponseGetAutoChecksum(quint8 *enable)
{
    int minLen = hasTimeStamp()?3:1;
    if (m_data.count() < minLen)
        return -1;    
        
    *enable = m_data.at(0);

    return 0;
}

int XCmdFrame::parseCmdResponseGetSSTConfig(quint8 *number, quint8 *enable, quint8 *id, 
    QByteArray *data, quint8 *chksum)
{
    int minLen = hasTimeStamp()?3:1; // at least 1 bytes (1 byte number)
    if (m_data.count() < minLen) 
        return -1;

    *number = m_data.at(0);
    if (*number == 0xFF)
        return 0;

    if (m_data.count() < (minLen+2))
        return -1;

    *enable = m_data.at(1);
    *id = m_data.at(2);

    if (*id == 0) // id invalid
        return 0;

    // at least (1 byte number, 1 byte enable, 1 byte id, 1 byte data, 1 byte checksum, 2 bytes timestamp)
    if (m_data.count() < (minLen+4))
        return -1;

    int dataLen;
    dataLen = m_data.count() - 3 - 1; // remove number,id,enable and checksum
    if (hasTimeStamp()) {
        dataLen -= 2;
    }

    *data = m_data.mid(3, dataLen);
    *chksum = m_data.at(3 + dataLen);

    return 0;
}

static QString parseDataSetPeriodicMessage(quint8 id, const QByteArray &data)
{
    QString response = QString();

    if (data.count() < 1)
        return response;

    int cmd = data.at(0);
    int number = id - 0x70;

    if (((cmd == 0x00)||(cmd == 0x01))
        && (data.count() > 4)) {
        // at least 5 bytes (1 byte status + 2 bytes time + 1 bytes header + 1 bytes data
        QByteArray payload = data.mid(3);
        return QStringLiteral("Set Up Periodic Message: #%1 - Status: %2, Rate: %3ms, Data: %4").\
            arg(number).\
            arg(data.at(0)?"Off":"On").\
            arg(((data.at(1)&0xFF)<<8) + (data.at(2)&0xFF)).\
            arg(Utils::Base::formatByteArray(&payload));
    } else if (data.count() != 1) {
        return response;
    }

    // only 1 byte data len
    switch (cmd) {
        case 0x00:
            return QStringLiteral("Turn periodic message #%1 ON").arg(number);
        case 0x01:
            return QStringLiteral("Turn periodic message #%1 OFF").arg(number);
        case 0x10:
            return QStringLiteral("Delete periodic message #%1").arg(number);
        case 0x20:
            return QStringLiteral("Delete all periodic messages that have been turned OFF");
        case 0x30:
            return QStringLiteral("Delete all periodic messages");
        case 0x80:
            return QStringLiteral("Turn ON all periodic messages that are OFF");
        case 0x90:
            return QStringLiteral("Turn OFF all periodic messages that are ON");
        default:
            break;
    }

    return response;
}

bool XCmdFrame::isCmdType(quint8 cmdId, XCmdFrame::CommandType cmdType)
{
    switch (cmdType) {
        case XCmdFrame::CMD_CFG_REQSWVER:
            return (cmdId == 0x92);
        case XCmdFrame::CMD_CFG_PMSG:
            return ((cmdId >= 0x70) && (cmdId <= 0x7F));
            break;
        default:
            break;
    }
    
    return false;
}

bool XCmdFrame::isCmdType(XCmdFrame::CommandType cmdType)
{
    return isCmdType(m_u.cmdId, cmdType);
}

static inline XCmdFrame::CommandType getCmdCfgType(quint8 cmdId)
{
    XCmdFrame::CommandType type = XCmdFrame::CMD_UNKNOW;
    
    switch (cmdId) {
    	case 0x51: // Request an SD card file's CRC
    		type = XCmdFrame::CMD_CFG_REQSDFCRC;
    		break;
        case 0x80:
            type = XCmdFrame::CMD_CFG_RESET;
            break;
    	case 0x86: // turn timestamp information off
    		type = XCmdFrame::CMD_CFG_TMOFF;
    		break;
    	case 0x87: // turn timestamp information on
    		type = XCmdFrame::CMD_CFG_TMON;
    		break;
        case 0x92: // software version request
            type = XCmdFrame::CMD_CFG_REQSWVER;
            break;
        case 0xA1: // retrieve and report operation warning code
            type = XCmdFrame::CMD_CFG_REQOWCODE;
            break;
        case 0xA3:
            type = XCmdFrame::CMD_CFG_REQSSBUS;
            break;
        case 0xA5: // software version request
            type = XCmdFrame::CMD_CFG_REQSN;
            break;
        default:
            if ((cmdId >= 0x70) && (cmdId <= 0x7F)) {
                type = XCmdFrame::CMD_CFG_PMSG;
            }
            break;
   	}

    return type;
}

static inline XCmdFrame::CommandType getCmdCanType(quint8 cmdId)
{
    XCmdFrame::CommandType type = XCmdFrame::CMD_UNKNOW;
    
    switch (cmdId) {
        case 0x01:
            type = XCmdFrame::CMD_CAN_SETFREQ;
            break;
        case 0x04:
            type = XCmdFrame::CMD_CAN_SETTRMODE;
            break;
        default:
            break;
    }

    return type;
}

static inline XCmdFrame::CommandType getCmdLinType(quint8 cmdId)
{
    XCmdFrame::CommandType type = XCmdFrame::CMD_UNKNOW;
    
    switch (cmdId) {
        case 0x01:
            type = XCmdFrame::CMD_LIN_SETVERSION;
            break;
        case 0x02:
            type = XCmdFrame::CMD_LIN_SETBAUDRATE;
            break;
        case 0x03:
            type = XCmdFrame::CMD_LIN_SETAUTOCKSUM;
            break;
        case 0x13:
            type = XCmdFrame::CMD_LIN_SSTREPORTCFG;
            break;
        default:
            break;
    }

    return type;
}

XCmdFrame::CommandType XCmdFrame::getCmdType()
{
    CommandType type = CMD_UNKNOW;
    quint8 cmd = cmdId();

    if (isConfigCommandFrame()) {
        type = getCmdCfgType(cmd);
        return type;
    }

    if (!isProtocolCommandFrame())
        return type;

    if (isCanFrame()) {
        type = getCmdCanType(cmd);
    } else if (isLinFrame()) {
        type = getCmdLinType(cmd);
    }
    
    return type;
}

QString XCmdFrame::handleConfigCmdResponse()
{
    QString response = QString();
    const QByteArray &ba = payload();
    
    switch ((quint8)cmdId()) {
    	case 0x51: // Request an SD card file's CRC
    		if (ba.count() == 5) {
    			// 4 bytes crc included
    			response = QString("CRC data %1").arg(ba.mid(1).toHex().constData());
    		} else if (ba.count() == 2) {
    			response = QString("Error: ") + getCRCErrorStr(ba);
    		}
    		break;
        case 0x80:
            response = QStringLiteral("Device is going to be reset");
            break;
    	case 0x86: // turn timestamp information off
    		response = QStringLiteral("Turn timestamp information OFF");
    		break;
    	case 0x87: // turn timestamp information on
    		response = QStringLiteral("Turn timestamp information ON");
    		break;
        case 0x92: // software version request
            response = QString(ba); //QString::fromLatin1(ba.constData());
            break;
        case 0xA5: // serial number request
            response = QString(ba); //QString::fromLatin1(ba.constData());
            break;
        case 0xA1: // retrieve and report operation warning code
            if (ba.count() == 1) {
                response = QString("OperationWarningCode(0x%1) - %2").\
                    	arg(quint8(ba.at(0)), 2, 16, QChar('0')).\
						arg(getOperationWarningCodeStr((quint8)ba[0]));
            }
            break;
        default:
            if ((cmdId() >= 0x70) && (cmdId() <= 0x7F)) {
                response = parseDataSetPeriodicMessage(cmdId(), ba);
            }
            break;
   	}

    return response;
}

QString XCmdFrame::handleCanCmdResponse()
{
    QString response = QString();
    const QByteArray &ba = payload();
    
    switch ((quint8)cmdId()) {
    	case 0x01:
    		response = QStringLiteral("to be added");
            break;
        default:
            if ((cmdId() >= 0x70) && (cmdId() <= 0x7F)) {
                response = parseDataSetPeriodicMessage(cmdId(), ba);
            }
            break;
   	}

    return response;
}

QString XCmdFrame::handleLinCmdResponse()
{
    QString response = QString();
    const QByteArray &ba = payload();
    
    switch ((quint8)cmdId()) {
    	case 0x01:
    		response = QStringLiteral("to be added");
            break;
        default:
            if ((cmdId() >= 0x70) && (cmdId() <= 0x7F)) {
                response = parseDataSetPeriodicMessage(cmdId(), ba);
            }
            break;
   	}

    return response;
}


QString XCmdFrame::handleCmdResponse(CommandType cmdType)
{
    QString response = QString();

#ifndef F_NO_DEBUG
    //qDebug() << QObject::tr("m_isValidFrame = %1, isCommandFrame = %2").arg(m_isValidFrame).arg(isCommandFrame());
#endif

    if ((!m_isValidFrame)||(!isCommandFrame()))
        return response;

    if ((cmdType != CMD_IGNORE) && (cmdType != getCmdType()))
        return response;
    
#ifndef F_NO_DEBUG
    //qDebug() << QObject::tr("cmdId = %1, data = %2").arg((quint8)cmdId(), 2, 16).arg(QString::fromLatin1(m_data.toHex().constData()));
#endif

    if (isConfigCommandFrame()) {
        response = handleConfigCmdResponse();
    } else if (isCanFrame()) {
        response = handleCanCmdResponse();
    } else if (isLinFrame()) {
        response = handleLinCmdResponse();
    }
   
/*    
    switch ((quint8)cmdId()) {
    	case 0x51: // Request an SD card file's CRC
    		if (m_data.count() == 5) {
    			// 4 bytes crc included
    			response = QString("CRC data %1").arg(m_data.mid(1).toHex().constData());
    		} else if (m_data.count() == 2) {
    			response = QString("Error: ") + getCRCErrorStr(m_data);
    		}
    		break;
    	case 0x86: // turn timestamp information off
    		response = QStringLiteral("Turn timestamp information OFF");
    		break;
    	case 0x87: // turn timestamp information on
    		response = QStringLiteral("Turn timestamp information ON");
    		break;
        case 0x92: // software version request
            response = QString(m_data); //QString::fromLatin1(m_data.constData());
            break;
        case 0xA1: // retrieve and report operation warning code
            if (m_data.count() == 1) {
                response = QString("OperationWarningCode(0x%1) - %2").\
                    	arg(quint8(m_data.at(0)), 2, 16, QChar('0')).\
						arg(getOperationWarningCodeStr((quint8)m_data[0]));
            }
            break;
        default:
            if ((cmdId() >= 0x70) && (cmdId() <= 0x7F)) {
                response = parseDataSetPeriodicMessage(cmdId(), m_data);
            }
            break;
   	}
*/
    return response;
}

QByteArray XCmdFrame::buildCfgCmdSaveConfig()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0xC0;

    raw.append(protocol);
    raw.append(id);

    return raw;
}

// isLong == true, 4 bytes timestamp; or 2 bytes
QByteArray XCmdFrame::buildCfgCmdTimpStampMode(bool isLong)
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0x94;

    raw.append(protocol);
    raw.append(id);
    if (isLong) {
        raw.append((char)0x01);
    } else {
        raw.append((char)0x00);
    }

    return raw;
}

QByteArray XCmdFrame::buildCfgCmdEnableInterfaces()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0xA3;

    raw.append(protocol);
    raw.append(id);
    raw.append(getBusTypeValue(BUS_CAN1));
    raw.append(getBusTypeValue(BUS_CAN2));
    raw.append(getBusTypeValue(BUS_LIN1));
    
    return raw;
}

QByteArray XCmdFrame::buildCfgCmdGetSSBusProtocols()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0xA3;

    raw.append(protocol);
    raw.append(id);
    
    return raw;
}

QByteArray XCmdFrame::buildCfgCmdGetVersion()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0x92;

    raw.append(protocol);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildCfgCmdGetSerialNumber()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0xA5;

    raw.append(protocol);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildCfgCmdPeriodicMessageGetAll()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0x70; // request periodic message
    char function = 0x70;
    
    raw.append(protocol);
    raw.append(id);
    raw.append(function);
    
    return raw;
}

QByteArray XCmdFrame::buildCfgCmdSetPeriodicMessage(int index, quint8 enable,
	quint16 period, quint16 delay, quint8 header, const QByteArray &data)
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0x70 + index;
    char function = enable?0:1;
    
    raw.append(protocol);
    raw.append(id);
    raw.append(function);
    raw.append((period>>8)&0xFF);
    raw.append(period&0xFF);
    raw.append(getBusTypeValue(header));
    raw.append(data);

	return raw;
}

QByteArray XCmdFrame::buildCfgCmdPeriodicMessageEnable(int index, quint8 enable)
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0x70 + index;
    char function = enable?0:1;
    
    raw.append(protocol);
    raw.append(id);
    raw.append(function);

	return raw;
}

QByteArray XCmdFrame::buildCfgCmdPeriodicMessageDelete(int index)
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0x70 + index;
    char function = 0x10;
    
    raw.append(protocol);
    raw.append(id);
    raw.append(function);
    
	return raw;
}

QByteArray XCmdFrame::buildCfgCmdReset()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0x80;

    raw.append(protocol);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildCfgCmdRestoreConfig()
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0xC1;

    raw.append(protocol);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildCfgCmdSetSSBusProtocols(QByteArray &ba)
{
    QByteArray raw;
    char protocol = 0x08;
    char id = 0xA3;

    raw.append(protocol);
    raw.append(id);
    if (!ba.isEmpty()) raw.append(ba);

    return raw;
}

QByteArray XCmdFrame::buildCanCmdGetFrequency(int bus)
{
    QByteArray raw;
    char header = (bus == 0)?0x54:0x5C;
    char id = 0x01;
    
    raw.append(header);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildCanCmdSetFrequency(int bus, int frequencey)
{
    QByteArray raw;
    char header = (bus == 0)?0x54:0x5C;
    char id = 0x01;
    QByteArray data = getBaudRateDataByValue(frequencey);
    
    raw.append(header);
    raw.append(id);
    raw.append(data);

    return raw;
}

QByteArray XCmdFrame::buildCanCmdSetFrequency(int bus, const QByteArray &data)
{
    QByteArray raw;
    char header = (bus == 0)?0x54:0x5C;
    char id = 0x01;
    
    raw.append(header);
    raw.append(id);
    raw.append(data);

    return raw;
}

/**
 * mode: 0 - sleep mode, 1 - high speed, 2 - wake up, 3 - normal
 */
QByteArray XCmdFrame::buildCanCmdSetSWMode(int bus, char mode)
{
    QByteArray raw;
    char header = (bus == 0)?0x54:0x5C;
    char id = 0x02;
    
    raw.append(header);
    raw.append(id);
    raw.append(mode);

    return raw;
}

QByteArray XCmdFrame::buildCanCmdGetCanTransceiverCtl(int bus)
{
    QByteArray raw;
    char header = (bus == 0)?0x54:0x5C;
    char id = 0x04;
    
    raw.append(header);
    raw.append(id);

    return raw;
}

/**
 * txvr: 0 - high speed, 1 - fault tolerant, 2 - sigle wired
 * mode: 
 * 	txvr == 0; 0 - normal, 1 - standby
 *	txvr == 1; 1 - sleep, 3 - normal
 *  txvr == 2; 0 - sleep, 1 - hight speed, 2 - hight voltage, 3 - normal
 */
QByteArray XCmdFrame::buildCanCmdSetCanTransceiverCtl(int bus, char txvr, char mode)
{
    QByteArray raw;
    char header = (bus == 0)?0x54:0x5C;
    char id = 0x04;

    raw.append(header);
    raw.append(id);
    switch (txvr) {
		case 0x00:
			raw.append(txvr);
			if ((mode >= 0)&&(mode <= 1))
				raw.append(mode);
			break;
		case 0x01:
			raw.append(txvr);
			if ((mode == 1)||(mode == 3))
				raw.append(mode);
			break;	
		case 0x02:
			raw.append(txvr);
			if ((mode >= 0)&&(mode <= 3))
				raw.append(mode);
			break;
		default:
			break;
    }

    return raw;
}

QByteArray XCmdFrame::buildLinCmdGetVersion(int bus)
{
    Q_UNUSED(bus);
    
    QByteArray raw;
    char header = 0xBC;
    char id = 0x01;
    
    raw.append(header);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildLinCmdSetVersion(int bus, quint8 version)
{
    Q_UNUSED(bus);
    
    QByteArray raw;
    char header = 0xBC;
    char id = 0x01;
    
    raw.append(header);
    raw.append(id);
    raw.append(version);

    return raw;
}


QByteArray XCmdFrame::buildLinCmdGetBaudrate(int bus)
{
    Q_UNUSED(bus);

    QByteArray raw;
    char header = 0xBC;
    char id = 0x02;
    
    raw.append(header);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildLinCmdSetBaudrate(int bus, quint32 baudrate)
{
    Q_UNUSED(bus);

    QByteArray raw;
    char header = 0xBC;
    char id = 0x02;
    
    raw.append(header);
    raw.append(id);
    QByteArray ba = Utils::Base::dec2Bcd(baudrate/100);
    raw.append(ba);
    
    return raw;
}

QByteArray XCmdFrame::buildLinCmdGetAutoChecksum(int bus)
{
    Q_UNUSED(bus);

    QByteArray raw;
    char header = 0xBC;
    char id = 0x03;
    
    raw.append(header);
    raw.append(id);

    return raw;
}

QByteArray XCmdFrame::buildLinCmdSetAutoChecksum(int bus, quint8 enable)
{
    Q_UNUSED(bus);

    QByteArray raw;
    char header = 0xBC;
    char id = 0x03;
    
    raw.append(header);
    raw.append(id);
    raw.append(enable);

    return raw;
}

QByteArray XCmdFrame::buildLinCmdGetSSTAll(int bus)
{
    Q_UNUSED(bus);

    QByteArray raw;
    char header = 0xBC;
    char id = 0x13;
    QByteArray data = "\xFF"; 
    
    raw.append(header);
    raw.append(id);
    raw.append(data);
    
    return raw;
}

QByteArray XCmdFrame::buildLinCmdSSTAddModify(quint8 id, const QByteArray &data)
{
    QByteArray raw;
    char header = 0xBC;
    char action = 0x10;
    quint8 chksum = 0;
    
    raw.append(header);
    raw.append(action);
    raw.append(id);
    raw.append(data);

    quint8 len = data.count();
    int start = 3;
    QByteArray ckdata = raw.mid(start, len);
    chksum = XBusFrame::linCalculateChecksum(ckdata, id, false);
    raw.append(chksum);
    
    return raw;
}

QByteArray XCmdFrame::buildLinCmdSSTEnable(quint8 id, quint8 enable)
{
    QByteArray raw;
    char header = 0xBC;
    char action = 0x12;
    
    raw.append(header);
    raw.append(action);
    raw.append(id);
    raw.append(enable);
    
    return raw;
}

QByteArray XCmdFrame::buildLinCmdSSTEnableAll(bool enable)
{
    QByteArray raw;
    char header = 0xBC;
    char action = 0x12;
    char id = 0xFF;
    
    raw.append(header);
    raw.append(action);
    raw.append(id);
    if (enable) raw.append(0x01);
    
    return raw;
}

QByteArray XCmdFrame::buildLinCmdSSTDelete(quint8 id)
{
    QByteArray raw;
    char header = 0xBC;
    char action = 0x11;
    
    raw.append(header);
    raw.append(action);
    raw.append(id);
    
    return raw;
}

QByteArray XCmdFrame::buildLinCmdSSTDeleteAll()
{
    QByteArray raw;
    char header = 0xBC;
    char action = 0x11;
    char id = 0xFF;
    
    raw.append(header);
    raw.append(action);
    raw.append(id);
    
    return raw;
}

