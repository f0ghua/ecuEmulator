#include "xbusmgr.h"
#include "utils.h"
#include "serialworker.h"

#include <QDebug>

unsigned char SerialWorker::ffFlag = 0;
static const char g_frameEndStr[2] = {(char)0xFF, (char)0x00};

SerialWorker::SerialWorker(XBusMgr *mgr) : HAL(mgr, 1)
{
	raw.clear();
}

void SerialWorker::getAllDevice(QStringList &listDev)
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

	for(int i = 0; i < ports.count(); i++)
		listDev << ports.at(i).portName();
}

void SerialWorker::sigOpenDevice(QString tryDev)
{
	QSerialPortInfo portInfo;
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

	qDebug()<<tryDev;
	for (int i = 0; i < ports.count(); i++) {
		if (ports.at(i).portName() == tryDev) {

			portInfo = ports.at(i);
			setSerialPort(&portInfo);
			break;
		}
	}
}

void SerialWorker::sigRefreshDevice()
{
	QStringList deviceList;
	
	getAllDevice(deviceList);

	emit updateDeviceList(deviceList);
}


SerialWorker::~SerialWorker()
{
	closeDevice();
}

void SerialWorker::sigCloseDevice(int)
{
	if (serial != NULL) {
		if (serial->isOpen()) {
			serial->clear();
			serial->close();
		}
		serial->disconnect();
		delete serial;
		serial = NULL;
		emit updateDeviceConnState(CLOSE_SUCC);
	}
}

void SerialWorker::setSerialPort(QSerialPortInfo *port)
{
	closeDevice();

	serial = new QSerialPort(*port);
	serial->setBaudRate(57600);
	serial->setDataBits(QSerialPort::Data8);
	serial->setParity(QSerialPort::NoParity);
	serial->setStopBits(QSerialPort::OneStop);
	serial->setFlowControl(QSerialPort::NoFlowControl);
	//serial->setFlowControl(QSerialPort::HardwareControl);

	if(serial->isOpen() == false) {
		if (!serial->open(QIODevice::ReadWrite)) {
			serial->clear();
			serial->close();
			delete serial;
			serial = NULL;
			emit updateDeviceConnState(OPEN_ERR);
			return;
		}
	}

	serial->setDataTerminalReady(true); //you do need to set these or the fan gets dirty
	serial->setRequestToSend(true);

	connect(serial, &QSerialPort::readyRead, this, &SerialWorker::readSerialData);
	qDebug()<<"com ok";
	emit updateDeviceConnState(OPEN_SUCC);
}

void SerialWorker::readSerialData()
{
	unsigned char c;
	QByteArray data = serial->readAll();

	//qDebug() << (tr("Got data from serial. Len = %0").arg(data.length()));

	for (int i = 0; i < data.length(); i++) {
		c = data.at(i);
		procRXChar(c);
	}

	if (framesRapid > 0) {
		emit frameUpdateRapid(framesRapid);
		framesRapid = 0;
	}
}

void SerialWorker::halWrite(QByteArray &buffer)
{
	if (serial == NULL) return;
	if (!serial->isOpen()) return;

	serial->write(buffer);
}

