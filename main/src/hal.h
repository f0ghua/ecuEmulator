#ifndef _HAL_H
#define _HAL_H

#include "xbusframe.h"

#include <QThread>

class XBusMgr;

class HAL: public QThread
{
	Q_OBJECT

public:
	HAL(XBusMgr *mgr, int m);
	~HAL(){closeDevice();}
	virtual void run(){}
	int getMode(){return mode;}
	virtual void changeServer(QString){};

public slots:
    virtual void sendFrame(const XBusFrame *);
	virtual void sendRawData(const QByteArray &raw);
	virtual void sigOpenDevice(QString) {};
	virtual void sigCloseDevice(int) {};
	virtual void sigRefreshDevice() {};

signals:
	void updateDeviceList(QStringList);
	void updateDeviceConnState(int);
	void frameUpdateRapid(int);
	void cmdFrameResponse(const QByteArray&);
	void upgradeFrameResponse(const QByteArray&);

protected:
	XBusMgr *m_mgr = NULL;
	int mode;
	QByteArray raw;
	int framesRapid = 0;
	static unsigned char ffFlag;
	QStringList deviceList;
	static const char *vHdr;
	int getv = 0;
	
	void handleFullData(const QByteArray &raw);
	void procRXChar(unsigned char c);
	virtual void closeDevice() {}
	virtual void halWrite(QByteArray &){};
};
#endif
