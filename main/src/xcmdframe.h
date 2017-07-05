#ifndef XCMDFRAME_H
#define XCMDFRAME_H

#include "xbusframe.h"

class XCmdFrame : public XBusFrame
{
public:
    enum CommandType {
        CMD_UNKNOW              = 0,
        CMD_CFG_REQSDFCRC       = 1,
        CMD_CFG_TMOFF           = 2,
        CMD_CFG_TMON            = 3,
        CMD_CFG_REQSWVER        = 4,
        CMD_CFG_REQOWCODE       = 5,
        CMD_CFG_PMSG            = 6,
        CMD_CFG_RESET           = 7,
        CMD_CFG_REQSN           = 8,
        CMD_CFG_REQSSBUS        = 9,
        
        CMD_CAN_START           = 16,
        CMD_CAN_SETFREQ         = CMD_CAN_START+1,
        CMD_CAN_SETTRMODE       = CMD_CAN_START+2,

        CMD_LIN_START           = 32,
        CMD_LIN_SETVERSION      = CMD_LIN_START+1,
        CMD_LIN_SETBAUDRATE     = CMD_LIN_START+2,
        CMD_LIN_SETAUTOCKSUM    = CMD_LIN_START+3,
        CMD_LIN_SSTREPORTCFG    = CMD_LIN_START+4,
        
        CMD_IGNORE              = 255
    };

    XCmdFrame(const QByteArray &data = QByteArray());
	QString handleCmdResponse(CommandType cmdType = CommandType::CMD_IGNORE);
	int parseCmdResponseSSBusProtocol(quint8 *protocols, int maxNumber);
    int parseCmdResponsePeriodicMessage(int *index, quint8 *enable, quint16 *period, 
        quint16 *delay, quint8 *header, QByteArray *data);
    int parseCmdResponseGetFrequency(quint8 *busType, quint8 *busIndex, quint16 *data);
    int parseCmdResponseGetCanTransCtrl(quint8 *busIndex, quint8 *type, quint8 *mode);
    int parseCmdResponseGetLinVersion(quint8 *version);
    int parseCmdResponseGetBaudrate(quint32 *baudrate);
    int parseCmdResponseGetAutoChecksum(quint8 *enable);
    int parseCmdResponseGetSSTConfig(quint8 *number, quint8 *enable, quint8 *id, 
        QByteArray *data, quint8 *chksum);
    
    bool isCmdType(CommandType cmdType);
    
    static bool isCmdType(quint8 cmdId, CommandType cmdType);
    XCmdFrame::CommandType getCmdType();

    static QString linVersion2String(quint8 version);
    static int getBaudRateValueByData(quint16 data);
    static QByteArray getBaudRateDataByValue(int baudrate);
    static QString getLinVersionDescription(quint8 version);
	static quint8 getLinVersionByDescription(QString s);
	
    static QByteArray buildCfgCmdSaveConfig();
    static QByteArray buildCfgCmdTimpStampMode(bool isLong = true);
    static QByteArray buildCfgCmdEnableInterfaces();
    static QByteArray buildCfgCmdGetSSBusProtocols();
	static QByteArray buildCfgCmdGetVersion();
	static QByteArray buildCfgCmdGetSerialNumber();
	static QByteArray buildCfgCmdSetPeriodicMessage(int index, quint8 enable, 
		quint16 period, quint16 delay, quint8 header, const QByteArray &data);
	static QByteArray buildCfgCmdPeriodicMessageEnable(int index, quint8 enable);
	static QByteArray buildCfgCmdPeriodicMessageDelete(int index);
    static QByteArray buildCfgCmdPeriodicMessageGetAll();
    static QByteArray buildCfgCmdReset();
    static QByteArray buildCfgCmdRestoreConfig();
    static QByteArray buildCfgCmdSetSSBusProtocols(QByteArray &ba);
    static QByteArray buildCanCmdGetFrequency(int bus);
	static QByteArray buildCanCmdSetFrequency(int bus, int frequencey);
	static QByteArray buildCanCmdSetFrequency(int bus, const QByteArray &data);
	static QByteArray buildCanCmdSetSWMode(int bus, char mode);
    static QByteArray buildCanCmdGetCanTransceiverCtl(int bus);
	static QByteArray buildCanCmdSetCanTransceiverCtl(int bus, char txvr, char mode);
	static QByteArray buildLinCmdGetVersion(int bus = BUS_LIN1);
	static QByteArray buildLinCmdSetVersion(int bus, quint8 version);
	static QByteArray buildLinCmdGetBaudrate(int bus = BUS_LIN1);
	static QByteArray buildLinCmdSetBaudrate(int bus, quint32 baudrate);
	static QByteArray buildLinCmdGetAutoChecksum(int bus = BUS_LIN1);
	static QByteArray buildLinCmdSetAutoChecksum(int bus, quint8 enable);
	static QByteArray buildLinCmdGetSSTAll(int bus = BUS_LIN1);
	static QByteArray buildLinCmdSSTAddModify(quint8 id, const QByteArray &data);
	static QByteArray buildLinCmdSSTEnable(quint8 id, quint8 enable);
	static QByteArray buildLinCmdSSTDelete(quint8 id);
	static QByteArray buildLinCmdSSTEnableAll(bool enable);
	static QByteArray buildLinCmdSSTDeleteAll();

private:
    QString handleConfigCmdResponse();
    QString handleCanCmdResponse();
    QString handleLinCmdResponse();
};

#endif // XCMDFRAME_H
