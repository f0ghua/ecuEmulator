#include "xbusframe.h"
#include "utils.h"

#include <QDebug>
#include <QDateTime>

static const char g_frameEndStr[2] = {(char)0xFF, (char)0x00};
static const char g_frameEscapeChar = 0xFF;

struct busType_t {
    quint8 value;
    int idx;
    QString description;
};

static const struct busType_t g_busTypes[] = {
    {PROTOCOL_ID_CAN1,  BUS_CAN1,   QStringLiteral("CAN1")},
    {PROTOCOL_ID_CAN2,  BUS_CAN2,   QStringLiteral("CAN2")},
    {PROTOCOL_ID_LIN,   BUS_LIN1,   QStringLiteral("LIN1")}
};

int XBusFrame::getBusTypeIndex(quint8 value)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_busTypes); i++) {
		if (g_busTypes[i].value == value)
			return g_busTypes[i].idx;
	}

    return -1;
}

quint8 XBusFrame::getBusTypeValue(int index)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_busTypes); i++) {
		if (g_busTypes[i].idx == index)
			return g_busTypes[i].value;
	}

    return 0;
}

QString XBusFrame::getBusTypeDescriptionByValue(quint8 value)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_busTypes); i++) {
		if (g_busTypes[i].value == value)
			return g_busTypes[i].description;
	}

    return QString();
}

QString XBusFrame::getBusTypeDescriptionByIndex(int index)
{
    for (quint8 i = 0; i < ARRAY_SIZE(g_busTypes); i++) {
		if (g_busTypes[i].idx == index)
			return g_busTypes[i].description;
	}

    return QString();
}

QStringList XBusFrame::getBusTypeDescriptions()
{
    QStringList l;
    
    for (quint8 i = 0; i < ARRAY_SIZE(g_busTypes); i++) {
        l.append(g_busTypes[i].description);
	}

    return l;
}

bool XBusFrame::isCommandFrame(const QByteArray &data)
{
    if (data.isEmpty())
        return false;
        
    quint8 header = data.at(0);
	quint8 protocol = header & PROTOCOL_ID_MASK;
    quint8 isCommand = IS_BIT_SET(header, 2);

    return ((protocol == PROTOCOL_ID_CONFIGURATION)||(isCommand)); 
}

bool XBusFrame::isUpgradeFrame(const QByteArray &data)
{
    if (data.isEmpty())
        return false;
        
    quint8 header = data.at(0);
	quint8 protocol = header & PROTOCOL_ID_MASK;
    //quint8 isCommand = IS_BIT_SET(header, 2);

    return ((protocol == PROTOCOL_ID_UPGRADE));//||(isCommand)); 
}

// data include id and payload
// cc: complete code include or not
// ck: LIN checksum include or not
XBusFrame::XBusFrame(int bus, bool tm, bool rx, bool cc, bool ck, const QByteArray &data)
{
    m_isValidFrame = false;
    
    m_protocolId = getBusTypeValue(bus);
    if (m_protocolId == 0)
        return;
    
    m_isTimeStampInc = tm;
    m_isTxFrame = !rx;

    // parse data frames
    switch (m_protocolId) {
        case PROTOCOL_ID_CAN1:
        case PROTOCOL_ID_CAN2:
			if (parseCanFrame(data, 0, cc) == -1)
				return;

            break;
        case PROTOCOL_ID_LIN:
			if (parseLinFrame(data, 0, cc, ck) == -1)
				return;

            break;          
        default:
            return;
    }

    m_isValidFrame = true;    
}

XBusFrame::XBusFrame(const QByteArray &data, int parseFlag)
{
	int i = 0;
    m_isValidFrame = false;

    if (data.isEmpty())
        return;

    quint8 header = data.at(i++);
	m_protocolId = header & PROTOCOL_ID_MASK;
    switch (m_protocolId) {
        case PROTOCOL_ID_CAN1:
            m_bus = BUS_CAN1;
            break;
        case PROTOCOL_ID_CAN2:
            m_bus = BUS_CAN2;
            break;
        case PROTOCOL_ID_LIN:
            m_bus = BUS_LIN1;
            break; 
        case PROTOCOL_ID_CONFIGURATION:
            break;
        default:
            return;
    }
    m_isCommand = IS_BIT_SET(header, 2);
    m_isTxFrame = IS_BIT_SET(header, 1);
    m_isTimeStampInc = IS_BIT_SET(header, 0);

    // parse command frames
    if (m_isCommand || (m_protocolId == PROTOCOL_ID_CONFIGURATION)) {
        if (data.size() < 2) {
            return;
        }
	    m_u.cmdId = data.at(i++);
        m_data = data.mid(i);

        m_isValidFrame = true;
        return;
	}

    if (parseFlag == ParseOnlyCommand)
        return;

    // parse data frames
    switch (m_protocolId) {
        case PROTOCOL_ID_CAN1:
        case PROTOCOL_ID_CAN2:
			if (parseCanFrame(data, i) == -1)
				return;

            break;
        case PROTOCOL_ID_LIN:
			if (parseLinFrame(data, i) == -1)
				return;

            break;          
        default:
            return;
    }

    m_isValidFrame = true;
}

int XBusFrame::parseCanFrame(const QByteArray &data, int offset, bool isCompleteCodeInc)
{
	int i = offset;

	if (data.count() <= i)
	    return -1;
	    
	quint8 c = data.at(i++);

	m_u.canInfo.isExtendedFrame = (c & 0x80) >> 7;
	if (m_u.canInfo.isExtendedFrame && (data.size() < 5))
	{
#ifndef F_NO_DEBUG
		qDebug() << "extended frame with size invalid";
#endif
		return -1;
	}

	if ((c & 0x20) >> 5)
		m_u.canInfo.type = FrameType::RemoteRequestFrame;
	else
		m_u.canInfo.type = FrameType::DataFrame;

	if (m_u.canInfo.type != FrameType::DataFrame)
	{
#ifndef F_NO_DEBUG
		qDebug() << "frame is not data frame";
#endif
		return -1;
	}

	if (!m_u.canInfo.isExtendedFrame)
	{
		m_u.canInfo.id = (c & 0x07) << 8;
		m_u.canInfo.id |= (data.at(i++) & 0xFF) ;
	}
	else
	{
		m_u.canInfo.id = (c & 0x1F) << 24;
		m_u.canInfo.id |= (data.at(i++) & 0xFF) << 16;
		m_u.canInfo.id |= (data.at(i++) & 0xFF) << 8;
		m_u.canInfo.id |= (data.at(i++) & 0xFF);
	}

	//qDebug() << "id = 0x" << QString::number(id, 16);
	int payloadLen = data.size() - i;
	if (isCompleteCodeInc) {
	    payloadLen -= COMPLETE_CODE_LEN;
	}
	if (m_isTimeStampInc) payloadLen -= TIME_STAMP_LEN;
	if (payloadLen < 0) return -1;

	m_data = data.mid(i, payloadLen);
	i += payloadLen;

    if (isCompleteCodeInc) {
	    m_completeCode = data.at(i++) & 0xFF;
    }
    
	if (m_isTimeStampInc) {
        m_timestamp = (data.at(i++) & 0xFF) << 24;
        m_timestamp |= (data.at(i++) & 0xFF) << 16;
        m_timestamp |= (data.at(i++) & 0xFF) << 8;
        m_timestamp |= (data.at(i) & 0xFF);
	}

	return 0;
}

int XBusFrame::parseLinFrame(const QByteArray &data, int offset, bool isCompleteCodeInc, bool isChecksumInc)
{
	int i = offset;
	
	if (data.count() <= i)
	    return -1;

	quint8 c = data.at(i++);

    m_u.linInfo.id = c & 0x3F;
    m_u.linInfo.parity = (c & 0xC0) >> 6;

	//qDebug() << "id = 0x" << QString::number(m_u.linInfo.id, 16);
	int payloadLen = data.size() - i;
	if (isChecksumInc) {
	    payloadLen -= LIN_CHECKSUM_LEN;
	}
	if (isCompleteCodeInc) {
	    payloadLen -= COMPLETE_CODE_LEN;
	}
	if (m_isTimeStampInc) payloadLen -= TIME_STAMP_LEN;
	if (payloadLen < 0) return -1;

	m_data = data.mid(i, payloadLen);
	i += payloadLen;

    if (isChecksumInc) {
        m_u.linInfo.cksum = data.at(i++) & 0xFF;
    }
    if (isCompleteCodeInc) {
	    m_completeCode = data.at(i++) & 0xFF;
    }
    
	if (m_isTimeStampInc) {
		m_timestamp = (data.at(i++) & 0xFF) << 8;
		m_timestamp |= (data.at(i) & 0xFF);
	}

	return 0;
}

static quint8 linCalculateChecksumInternal(const quint8 *pdata, quint8 len)
{
	quint16 sum = 0, i = 0;
	quint8 checksum = 0;
	quint16 tmp1 = 0 , tmp2 = 0;

	for(i = 0; i < len; i++){
		sum += pdata[i];
		tmp1 = sum >> 8;
		tmp2 = sum & 0xFF;
		sum = tmp1 + tmp2;

	}

	checksum = 0xFF - sum;
	return checksum;
}

quint8 XBusFrame::linCalculateChecksum(const QByteArray &data, quint8 id, bool isParityInc)
{
    quint8 checksum = 0;
    quint8 pid = id;

    if (!isParityInc) {
        pid = linCalculateIdParity(id);
    }
    
    if (Utils::Base::linVersion == LIN_VER_2_0) {
        QByteArray ba = data;
        ba.append(pid);
        checksum = linCalculateChecksumInternal((const quint8 *)ba.constData(), ba.count());
    } else {
        checksum = linCalculateChecksumInternal((const quint8 *)data.constData(), data.count());
    }
    
	return checksum;
}

quint8 XBusFrame::linCalculateIdParity(quint8 id)
{
	quint8 p0 = 0, p1 = 0;
	quint8 new_id = id;

	p0 = (IS_BIT_SET(id, 0) + IS_BIT_SET(id, 1) + IS_BIT_SET(id, 2) + IS_BIT_SET(id, 4))& 0x1;
	p1 = (IS_BIT_SET(id, 1) + IS_BIT_SET(id, 3) + IS_BIT_SET(id, 4) + IS_BIT_SET(id, 5) + 1)& 0x1;

	if(p0){
		SET_BIT(new_id, 6);
	}
	else{
		CLEAR_BIT(new_id, 6);
	}

	if(p1){
		SET_BIT(new_id, 7);
	}
	else{
		CLEAR_BIT(new_id, 7);
	}
	return new_id;
}

void XBusFrame::setLINId(quint8 id)
{
    quint8 c = linCalculateIdParity(id);
    m_u.linInfo.id = c & 0x3F;
    m_u.linInfo.parity = (c & 0xC0) >> 6;
}

void XBusFrame::packFrame(QByteArray &data)
{
    int pos, from = 0;

    // escape 0xFF
    while ((pos = data.indexOf(g_frameEscapeChar, from)) != -1) {
        data.insert(pos, g_frameEscapeChar);
        from = pos + 2; //jump 2 escape chars
    }

    // append the end
    data.append(QByteArray::fromRawData(g_frameEndStr, sizeof(g_frameEndStr)));
}

QByteArray XBusFrame::packFrame(const QByteArray &data)
{
    QByteArray buffer;

    // escape 0xFF
	for (int i = 0; i < data.count(); i++) {
		buffer.append(data.at(i));
		if (data.at(i) == g_frameEscapeChar) {
			buffer.append(g_frameEscapeChar);
		}
	}

    // append the end
    buffer.append(QByteArray::fromRawData(g_frameEndStr, sizeof(g_frameEndStr)));

    return buffer;
}

static QString cvtCompleCode2Str(quint8 ccode)
{
    switch (ccode) {
        case 0:
            return QStringLiteral("Reserved");
            break;
        case 0x01:
            return QStringLiteral("StuffError");
            break;
        case 0x02:
            return QStringLiteral("FormError");
            break;
        case 0x03:
            return QStringLiteral("AckError");
            break;
        case 0x04:
            return QStringLiteral("Bit1Error");
            break;
        case 0x05:
            return QStringLiteral("Bit0Error");
            break;
        case 0x06:
            return QStringLiteral("CRCError");
            break;
        case 0x07:
            return QStringLiteral("Reserved");
            break;
        case 0x08:
            return QStringLiteral("Success");
            break;
        case 0x09:
            return QStringLiteral("Reserved");
            break;
        case 0x10:
            return QStringLiteral("Success");
            break;
        case 0x20:
            return QStringLiteral("BufferOverrun");
            break;
        case 0x30:
            return QStringLiteral("OutofOrder");
            break;
   	 	default:
            return QStringLiteral("UnknowError");
            break;
    }

	return QStringLiteral("Reserved");
}

struct LinCompleteErrorMapping_t {
    int bitpos;
    QString description;
};

static const struct LinCompleteErrorMapping_t g_linCompleteErrorMapping[] = {
    {0,     QStringLiteral("SlaveNotResponding")},
    {1,     QStringLiteral("TframeError")},
    {2,     QStringLiteral("ChecksumError")},
    {3,     QStringLiteral("ParityError")},
    {4,     QStringLiteral("SyncError")},
    {5,     QStringLiteral("LoopBackError")},
    {6,     QStringLiteral("ReceiveFramingError")},
    {7,     QStringLiteral("RxBuffOverflow")}
};

static QString cvtLinCompleCode2Str(quint8 ccode)
{
    QString response = QString();

    for (quint8 i = 0; i < ARRAY_SIZE(g_linCompleteErrorMapping); i++) {
        if (ccode & (1 << g_linCompleteErrorMapping[i].bitpos)) {
            if (!response.isEmpty()) response.append(QChar('|'));
            response.append(g_linCompleteErrorMapping[i].description);
        }
	}

    if (response.isEmpty()) {
        response = QStringLiteral("Success");
    }
    
    return response;
}

QString XBusFrame::getCompleCodeString()
{
    QString response = QString();
    
    if (isCanFrame()) {
        response = cvtCompleCode2Str(m_completeCode);
    } else { // LIN frame
        response = cvtLinCompleCode2Str(m_completeCode);
    }
    
    return response;
}

bool XBusFrame::isFrameError(QString ccodeStr)
{
    if (ccodeStr.contains(QStringLiteral("Success"))) {
        return false;
    }
    return true;
}

QByteArray XBusFrame::getPureRawData()
{
	QByteArray ba;
	char protocol, c;

    c = (m_protocolId) + (m_isTxFrame?(1<<1):0) + (m_isTimeStampInc?1:0);
    ba.append(c);
    c = 0;
    switch (m_protocolId) {
        case PROTOCOL_ID_CAN1:
        case PROTOCOL_ID_CAN2:
        {
	        if (IS_EXTENDED(m_u.canInfo.id)) SET_BIT(c, 7);
	        if (m_u.canInfo.type != FrameType::DataFrame) SET_BIT(c, 5);
	        if (!IS_EXTENDED(m_u.canInfo.id)) {
		        c |= (m_u.canInfo.id >> 8)&0x07;
		        ba.append(c);
		        ba.append(m_u.canInfo.id & 0xFF);
	        }
	        else {
		        c |= (m_u.canInfo.id >> 24)&0x1F;
		        ba.append(c);
		        ba.append((m_u.canInfo.id >> 16) & 0xFF);
		        ba.append((m_u.canInfo.id >> 8) & 0xFF);
		        ba.append((m_u.canInfo.id) & 0xFF);
	        }            

	        ba.append(m_data);
	        
            break;
        }
        case PROTOCOL_ID_LIN:
        {
            c = linCalculateIdParity(m_u.linInfo.id);
            ba.append(c);

            ba.append(m_data);

            c = linCalculateChecksum(m_data, c);
            ba.append(c);
            break;
        }
    }

	return ba;
}

QByteArray XBusFrame::buildHeader(quint32 id, int bus, bool tm, bool rx, FrameType type)
{
	QByteArray ba;
	char protocol, c;

	protocol = ((bus == 0)?PROTOCOL_ID_CAN1:PROTOCOL_ID_CAN2);
	c = (protocol) + (rx?0:(1<<1)) + (tm?1:0);
	ba.append(c);
	c = 0;
	if (IS_EXTENDED(id)) SET_BIT(c, 7);
	if (type != FrameType::DataFrame) SET_BIT(c, 5);
	if (!IS_EXTENDED(id)) {
		c |= (id >> 8)&0x07;
		ba.append(c);
		ba.append(id & 0xFF);
	}
	else {
		c |= (id >> 24)&0x1F;
		ba.append(c);
		ba.append((id >> 16) & 0xFF);
		ba.append((id >> 8) & 0xFF);
		ba.append((id) & 0xFF);
	}

	return ba;
}

QByteArray XBusFrame::buildIdArray(quint32 id, int bus, FrameType type)
{
	QByteArray ba;
	quint8 protocol; 
	char c = 0;

    protocol = XBusFrame::getBusTypeValue(bus);
    if (protocol == 0)
        return ba;

    switch (protocol) {
        case PROTOCOL_ID_CAN1:
        case PROTOCOL_ID_CAN2:
        {
            if (IS_EXTENDED(id)) SET_BIT(c, 7);
            if (type != XBusFrame::FrameType::DataFrame) SET_BIT(c, 5);
            if (!IS_EXTENDED(id)) {
                c |= (id >> 8)&0x07;
                ba.append(c);
                ba.append(id & 0xFF);
            }
            else {
                c |= (id >> 24)&0x1F;
                ba.append(c);
                ba.append((id >> 16) & 0xFF);
                ba.append((id >> 8) & 0xFF);
                ba.append((id) & 0xFF);
            }

            break;
        }
        case PROTOCOL_ID_LIN:
        {
            c = linCalculateIdParity(id);
            ba.append(c);
            
            break;  
        }
        default:
            break;
    }

	return ba;
}

QByteArray XBusFrame::buildTailer(bool tm, quint16 timestamp)
{
	QByteArray ba;

	if (tm) {
		ba.append((timestamp >> 8) & 0xFF);
		ba.append(timestamp & 0xFF);
	}

	return ba;
}

QByteArray XBusFrame::buildLinFrame(quint8 id, QByteArray &data, quint8 chksum)
{
	QByteArray ba;
    quint8 c;
    
    c = linCalculateIdParity(m_u.linInfo.id);
    ba.append(c);
    
    ba.append(data);

    if (chksum) {
        c = chksum;
    } else {
        c = linCalculateChecksum(data, c);
    }
    
    ba.append(c);
    
	return ba;
}

QString XBusFrame::toString(qint64 baseTime) const
{
    qint64 elapsedMs = (m_timestamp >= (quint64)baseTime)?
        (m_timestamp - baseTime):
        (QDateTime::currentMSecsSinceEpoch() - baseTime);

#ifndef F_NO_DEBUG
    //qDebug() << QObject::tr("m_timestep = %1, baseTime = %2").arg(m_timestamp).arg(baseTime);
#endif
    QString ts = QString("%1.%2").arg(elapsedMs/1000).arg(elapsedMs%1000,3,10,QChar('0'));
    return QString("%1 %2 %3 %4 %5").\
        arg(ts, -16, QChar(' ')).\
        arg(isReceived()?QStringLiteral("Rx"):QStringLiteral("Tx")).\
        arg(getBusTypeDescriptionByIndex(m_bus)).\
        arg("0x" + QString::number(id(), 16).toUpper().rightJustified(3,'0')).\
        arg(m_data.toHex().constData());
}

