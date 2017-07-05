#ifndef XBUSFRAME_H
#define XBUSFRAME_H

#include <QByteArray>
#include <QVariant>

#include "xcommdefine.h"

class XBusFrame
{
public:
	enum FrameType {
		UnknownFrame		= 0x0,
		DataFrame			= 0x1,
		ErrorFrame			= 0x2,
		RemoteRequestFrame	= 0x3,
		InvalidFrame		= 0x4
	};

    enum ParseOption {
        ParseAll            = 0,
        ParseOnlyCommand    = 1
    };

	XBusFrame(const QByteArray &data = QByteArray(), int parseFlag = ParseAll);
    XBusFrame(int bus, bool tm = false, bool rx = false, bool cc = false, bool ck = false, const QByteArray &data = QByteArray());
    QByteArray getPureRawData();
    QByteArray buildLinFrame(quint8 id, QByteArray &data, quint8 chksum = 0);
    QString getCompleCodeString();
    
    static int getBusTypeIndex(quint8 value);
    static quint8 getBusTypeValue(int index);
    static QString getBusTypeDescriptionByValue(quint8 value);
    static QString getBusTypeDescriptionByIndex(int index);
    static QStringList getBusTypeDescriptions();
    static bool isCommandFrame(const QByteArray &data);
	static bool isUpgradeFrame(const QByteArray &data);
	static void packFrame(QByteArray &data);
	static QByteArray packFrame(const QByteArray &data);
    static bool isFrameError(QString ccodeStr);
    static QByteArray buildHeader(quint32 id, int bus, bool tm = false, bool rx = false, FrameType type = XBusFrame::FrameType::DataFrame);
    static QByteArray buildTailer(bool tm, quint16 timestamp);
    static QByteArray buildIdArray(quint32 id, int bus, FrameType type = XBusFrame::FrameType::DataFrame);
    static quint8 linCalculateChecksum(const QByteArray &data, quint8 id, bool isParityInc = true);
    static quint8 linCalculateIdParity(quint8 id);
    
    quint8 protocolId() const { return m_protocolId; }
	int bus() const { return m_bus; }
    int bus() {return m_bus;}
	void setBus(int i) { m_bus = i; }
	int dlc() const { return m_data.count(); }
	void setDlc(int dlc)
	{
		if (dlc > m_data.count()) {
			QByteArray ba((dlc-m_data.count()), 0);
			m_data.append(ba);
		} else {
			m_data.truncate(dlc);
		}
	}
	
	quint32 id() const 
	{ 
	    if (isCanFrame()) 
	        return m_u.canInfo.id; 
	    else 
	        return m_u.linInfo.id; 
	}
	
	void setId(quint32 v) 
	{ 
	    if (isCanFrame())
	        m_u.canInfo.id = v;
	    else 
	        setLINId((quint8)v); 
	}
	const QByteArray payload() const { return m_data; }
	QByteArray &payload() { return m_data; }
	void setPayload(const QByteArray &data) { m_data = data; }
	void setPayload(int i, char ch)
	{
		if (i < m_data.count()) {
			m_data[i] = ch;
		}
	}
	quint64 timestamp() const { return m_timestamp; }
	bool isValid() const { return m_isValidFrame; }
	bool isReceived() const { return !m_isTxFrame; }
	bool hasTimeStamp() { return m_isTimeStampInc; }
	bool isCanFrame() { return ((m_protocolId == PROTOCOL_ID_CAN1)||(m_protocolId == PROTOCOL_ID_CAN2)); }
	bool isCanFrame() const { return ((m_protocolId == PROTOCOL_ID_CAN1)||(m_protocolId == PROTOCOL_ID_CAN2)); }
	bool isLinFrame() { return (m_protocolId == PROTOCOL_ID_LIN);}
	quint8 completeCode() const { return m_completeCode; }
	quint8 cmdId() const { return m_u.cmdId; }
	bool isCommandFrame() { return ((m_protocolId == PROTOCOL_ID_CONFIGURATION)||(m_isCommand)); }
	bool isConfigCommandFrame() { return (m_protocolId == PROTOCOL_ID_CONFIGURATION); }
    bool isProtocolCommandFrame() { return m_isCommand; }
    QString toString(qint64 baseTime) const;
    
protected:

	// header information
    quint8 m_protocolId;
    quint8 m_isCommand:1;	// is CAN command
    quint8 m_isTxFrame:1;
    quint8 m_isTimeStampInc:1;
	quint8 m_isValidFrame:1;
	quint8 m_bus:4;

    union {
        quint8 cmdId;
        
        struct CANInfo {
	        // frame properties
            quint8 isExtendedFrame:1;
            quint8 version:5;
            quint8 isValidFrameId:1;
            quint8 isFlexibleDataRate:1;

	        // frame content
            quint32 id:29; // acts as container for error codes too
            quint8 	type:3; // max of 8 frame types
        } canInfo;
        
        struct LINInfo {
	        // frame properties

	        // frame content
            quint8  id:6;
            quint8 	parity:2;
            quint8  cksum;
        } linInfo;
    } m_u;
    
	quint8 reserved[3];
	QByteArray m_data; // msg data, no include header and tailer
	quint8 m_completeCode;
	quint64 m_timestamp;

private:
	void setLINId(quint8 id);
    int parseCanFrame(const QByteArray &data, int offset, bool isCompleteCodeInc = true);
    int parseLinFrame(const QByteArray &data, int offset, bool isCompleteCodeInc = true, bool isChecksumInc = true);
};

#endif // XBUSFRAME_H

