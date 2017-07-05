
#include "xcommdefine.h"
#include "busengine.h"
//#include "canframe.h"
#include "xbusmgr.h"
//#include "datahelper.h"
//#include "xcanframe.h"
#include "xbusframe.h"

#include <QTcpSocket>
#include <QHostInfo>

BusEngine::BusEngine(XBusMgr *mgr, QString ip):HAL(mgr, 0), remoteIp(ip)
{
	lookForServer();
}

void BusEngine::lookForServer()
{
	QString name = QHostInfo::localHostName();

	if(socket == NULL)
		socket = new QTcpSocket(this);

	connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(handleChange()));

	socket->connectToHost(remoteIp, 8000);
	socket->waitForConnected();
	connected = 0;
#ifndef F_NO_DEBUG
	qDebug()<<"CLIENT_CONNECT";
#endif
	SendCommand(CLIENT_CONNECT, name.toLatin1().data(), name.size());
	//SendCommand(REQUEST_VERSION,NULL,0);
}

void BusEngine::handleChange()
{
	if(connected == 1) {

		deviceList.clear();
		emit updateDeviceList(deviceList);
		emit updateDeviceConnState(CLOSE_SUCC);
	}
#ifndef F_NO_DEBUG
	qDebug()<<"refresh socket";
#endif
	PhyCloseDevice();
}

void BusEngine::changeServer(QString ip)
{
	PhyCloseDevice();
	remoteIp = ip;	
	lookForServer();
#ifndef F_NO_DEBUG
	qDebug()<<"REQUEST_DEVICES";
#endif
	SendCommand(REQUEST_DEVICES,NULL,0);
}

unsigned int BusEngine::mergeLastHalf(char *data, unsigned int len)
{
	unsigned int partRead = 0;
	COMMAND_DATA_PACKET *commandData;

	if(sofar < sizeof(COMMAND_DATA_PACKET)) {

		partRead += (sizeof(COMMAND_DATA_PACKET) - sofar);
		memcpy(halfBuf + sofar, data, partRead);
		sofar += partRead;
		data += partRead;
		len -= partRead;
	}
	
	commandData = (COMMAND_DATA_PACKET *)halfBuf;

	unsigned int left = commandData->Size - sofar;
	if(left > len) {

		unsigned int dataLen = len - partRead;
		memcpy(halfBuf + sofar, data, dataLen);
		sofar += dataLen;
#ifndef F_NO_DEBUG
		qDebug()<<"not a complete pkt";
#endif
		return len;
	}
	else {

		memcpy(halfBuf + sofar, data, left);
		partRead += left;
		sofar = 0;
#ifndef F_NO_DEBUG
		qDebug()<<"A complete pkt";
#endif
		processResponse(commandData);
		return partRead;
	}
}

void BusEngine::readData()
{
	char data[4096], *pdata;
	unsigned int len;
	COMMAND_DATA_PACKET *commandData;

	if(connected < 0)
		return;

	memset(data, 0, sizeof(data));
	pdata = data;

	if((len = socket->read(data, sizeof(data))) <= 0)
		return;

	if(sofar != 0) {
		unsigned int last = mergeLastHalf(data, len);

		pdata += last;
		len -= last;
	}

	commandData = (COMMAND_DATA_PACKET *)pdata;

	while(len > 0) {
		if(len <= sizeof(COMMAND_DATA_PACKET)) {//lost size indicator and data
			memcpy(halfBuf + sofar, commandData, len);
			sofar += len;
#ifndef F_NO_DEBUG
			qDebug()<<"wait 1";
#endif
			break;
		}
		else if((qint32)len < commandData->Size) {//lost data
			memcpy(halfBuf + sofar, commandData, len);
			sofar += len;
#ifndef F_NO_DEBUG
			qDebug()<<"wait 2, but comsize " << commandData->Size << "len " << len;
#endif
			break;
		}
		else {

			processResponse(commandData);
	//		qDebug()<<commandData << len << commandData->Size;
			len -= commandData->Size;
			commandData = (COMMAND_DATA_PACKET *)((unsigned char *)commandData + commandData->Size);
		}
	}
}

void BusEngine::processRequseDevices(QString s)
{
	QStringList ql;

	deviceList.clear();
	ql = s.split(",");

	for(int i = 0; i < ql.size() - 1; i += 2)
		deviceList << ql[i];
#ifndef F_NO_DEBUG
	qDebug()<<"new update";
#endif
	emit updateDeviceList(deviceList);
}

void BusEngine::processDeviceData(DEVICE_DATA_PACKET *pDeviceData)
{
	if(pDeviceData->Data[0] != lastPktType) {
		lastPktType = pDeviceData->Data[0];
		ffFlag = 0;
		raw.clear();
	}

	for(int i = 0; i < pDeviceData->Size; i++)
		raw.append(pDeviceData->Data[i]);

	handleFullData(raw);
	raw.clear();
}

void BusEngine::processResponse(COMMAND_DATA_PACKET *commandData)
{
	DEVICE_DATA_PACKET *pDeviceData;

	switch(commandData->Command) {

		case CLIENT_CONNECT:
			connected = 1;
			break;

		case REQUEST_VERSION:
			break;

		case DEVICE_DATA:
			pDeviceData = (DEVICE_DATA_PACKET *) (commandData->Data);
			processDeviceData(pDeviceData);
			break;

		case REQUEST_DEVICES:
			processRequseDevices(commandData->Data);
			break;

		case CONNECT_TO_DEVICE:
			connId = (qint32)(*(commandData->Data));
#ifndef F_NO_DEBUG
			qDebug()<<"commid " << connId;
#endif
			if(connId != -1) {
				emit updateDeviceConnState(OPEN_SUCC);
				getv = 1;
			}
			else
				SendCommand(CONNECT_TO_DEVICE, phydev.toLatin1().data(), phydev.size());
			break;

		case DEVICE_LIST_CHANGED:
			PhyCloseDevice();
			deleteSocket();
			break;

		default:
#ifndef F_NO_DEBUG
			qDebug()<<"some ev" << commandData->Command;
#endif
			break;
	}
}

void BusEngine::deleteSocket()
{
	connected = -1;
	delete socket;
	socket = NULL;
}

void BusEngine::SendCommand(COMMAND command, const void *pdata, int plen)
{
	qint32 len = plen + sizeof(COMMAND_DATA_PACKET);
	COMMAND_DATA_PACKET *commandData = (COMMAND_DATA_PACKET *)calloc(len, 1);

	commandData->Command = command;
	commandData->Size = len;

	if(plen != 0 && pdata != NULL)
		memcpy(commandData->Data, pdata, plen);

	if(connected < 0)
		goto out;

	socket->write((const char *)commandData, len);
	
out:
	free(commandData);
}

void BusEngine::sigRefreshDevice()
{
#ifndef F_NO_DEBUG
	qDebug()<<"sig REQUEST_DEVICES";
#endif
	if(connected == -1) {
		m_mgr->checkBusEngine();
		lookForServer();
	}
	SendCommand(REQUEST_DEVICES, NULL, 0);
}

void BusEngine::sigOpenDevice(QString dev)
{
	if(connected == -1) {
		lookForServer();
	}
	else if(connected == 0)
		lookForServer();

	if(dev.isNull()== false && dev.isEmpty() == false) {
#ifndef F_NO_DEBUG
		qDebug()<<"CONNECT_TO_DEVICE" << dev;
#endif
    	phydev = dev;
		SendCommand(CONNECT_TO_DEVICE, dev.toLatin1().data(), dev.size());
		}
}

void BusEngine::sigCloseDevice(int f) //no response
{
	if(connId >= 0) {
#ifndef F_NO_DEBUG
		qDebug()<<"DISCONNECT_FROM_DEVICE";
#endif
		SendCommand(DISCONNECT_FROM_DEVICE, &connId, sizeof(connId));
	}

	updateDeviceConnState(CLOSE_SUCC);
	//if(f == 1)
		PhyCloseDevice();
}

void BusEngine::halWrite(QByteArray &data)
{
	if(connId < 0)
		return;

	SendData(DEVICE_DATA, data.data(), data.size());
}

void BusEngine::SendData(COMMAND command, const void *pdata, int plen)
{
	qint32 len = plen + sizeof(DEVICE_DATA_PACKET) + sizeof(COMMAND_DATA_PACKET);
	COMMAND_DATA_PACKET *commandData = (COMMAND_DATA_PACKET *)calloc(len, 1);
	DEVICE_DATA_PACKET *pDeviceData = (DEVICE_DATA_PACKET *) (commandData->Data);

	//qDebug()<<len << sizeof(DEVICE_DATA_PACKET) + sizeof(COMMAND_DATA_PACKET) << connId;
	pDeviceData->DeviceID = connId;
	pDeviceData->Size = plen;
	commandData->Command = command;
	commandData->Size = len;

	if(plen != 0 && pdata !=NULL)
		memcpy(pDeviceData->Data, pdata, plen);

	socket->write((const char *)commandData, len);
	
	free(commandData);
}

void BusEngine::PhyCloseDevice()
{
#ifndef F_NO_DEBUG
	qDebug()<<"closeDevice";
#endif
	socket->close();
	connected = -1;
	connId = -1;
}
