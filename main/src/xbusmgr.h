#ifndef XCANBUS_H
#define XCANBUS_H

#include "hal.h"
#include "DBC.h"

#include <QObject>
#include <QMutex>
#include <QVector>

class XBusFrame;

class XBusMgr : public QObject
{
    Q_OBJECT
public:
    explicit XBusMgr(QObject *parent = 0);
    HAL *hal() { return m_currentHal; }
    void enqueueReceivedFrame(const XBusFrame &newFrame);
    XBusFrame readFrame();
    qint64 framesAvailable() const { return m_incomingFrames.size(); }
    void openDevice(int, QString dev) { emit sigOpenDevice(dev); }
    void closeDevice(int f) { emit sigCloseDevice(f); }
    void getAllDevice(int mode);
    void initDevice();
    void sendMsgRaw(QByteArray &raw) { emit sigSendRawData(raw); }
    int checkBusEngine();
    int getHwReady() { return m_hwReady; }
    bool isRunning() { return m_isRunning; }
    void stop() { m_isRunning = false; }
    void start() { m_isRunning = true; }
    Vector::DBC::Network *getDbcNetwork(int bus)
    {
        return m_dbcFile[bus]?m_dbcFile[bus]->network():NULL;
    }

signals:
    void frameReceived();
	void sigRefreshDevice();
	void sigOpenDevice(QString);
	void sigCloseDevice(int);
	void sigSendRawData(const QByteArray &raw);
	void sendRawFrame(const XBusFrame *);
	void sigUpdateDeviceList(QStringList);
	void sigUpdateDeviceConnState(int);
	void sigCmdFrameResponse(const QByteArray&);
	
public slots:
    void updateDeviceList(QStringList dl) { emit sigUpdateDeviceList(dl); }
    void updateDeviceConnState(int s) 
    { 
        m_hwReady = (s == 0)?true:false;
        emit sigUpdateDeviceConnState(s); 
    }
    void handleCmdResponse(const QByteArray &raw) { emit sigCmdFrameResponse(raw); }
    
private:
    void registerHAL(HAL *hal) {m_hals << hal; hal->start();}	
    void initHAL(int mode);
    void setupSig(HAL *hal);
    
    QList <HAL *> m_hals;
    HAL *m_currentHal = NULL;
    bool m_hwReady = false;
    bool m_isRunning = false;

    Vector::DBC::File *m_dbcFile[2] = {NULL, };

    QVector<XBusFrame> m_incomingFrames;
    QMutex m_incomingFramesGuard;
    QVector<XBusFrame> m_outgoingFrames;
};

#endif // XCANBUS_H
