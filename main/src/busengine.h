#ifndef _BUS_ENGINE_H
#define _BUS_ENGINE_H

#include "hal.h"
#include "busenginepri.h"

class QTcpSocket;

class BusEngine : public HAL
{
	Q_OBJECT
public:
	explicit BusEngine(XBusMgr *, QString ip=QString("127.0.0.1"));
	~BusEngine(){}
	void changeServer(QString ip);

public slots:
	void sigOpenDevice(QString tryDev);
	void sigCloseDevice(int);
	void sigRefreshDevice();

private:
	QTcpSocket *socket = NULL;
	qint32 connId = -1, connected = -1;
	QString remoteIp;
	unsigned char lastPktType = 0xff;
	unsigned int sofar = 0;
	char halfBuf[1024];
	QString phydev;
	
	void PhyCloseDevice();
	void halWrite(QByteArray &);
	void processResponse(COMMAND_DATA_PACKET *raw);
	void processRequseDevices(QString s);
	void processDeviceData(DEVICE_DATA_PACKET *pDeviceData);
	void SendCommand(COMMAND command, const void *pdata, int plen);
	void SendData(COMMAND command, const void *pdata, int plen);
	void lookForServer();
	void deleteSocket();
	unsigned int mergeLastHalf(char *data, unsigned int len);

private slots:
	void readData();
	void handleChange();
};
#endif
