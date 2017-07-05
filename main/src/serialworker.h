#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMutex>
#include <QSettings>

#include "hal.h"

class XBusMgr;

class SerialWorker : public HAL
{
	Q_OBJECT

public:
	explicit SerialWorker(XBusMgr *);
	~SerialWorker();
	void getAllDevice(QStringList &listDev);
	void run(){};
	void FF(int s) {ffFlag = s;}

public slots:
	void sigOpenDevice(QString tryDev);
	void sigCloseDevice(int);
	void sigRefreshDevice();

private slots:
	void readSerialData();
	
signals:
	void deviceSucc(int);
	void frameUpdateRapid(int);
	void cmdFrameResponse(const QByteArray&);

protected:
	void halWrite(QByteArray &buffer);
	
private:
	void setSerialPort(QSerialPortInfo*);

	QByteArray raw;
	int framesRapid = 0;
	static unsigned char ffFlag;
	QSerialPort *serial = NULL;
};

#endif // SERIALWORKER_H
