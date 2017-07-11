#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include <QThread>
#include <QCheckBox>

#include "DBC.h"

class QLineEdit;
class XBusMgr;
class XFrameSender;
class XBusFrame;
class ConnectDialog;
class AboutDialog;
class XFrameLogger;
class DeviceConfig;

typedef struct PeriodMessage_t
{
    quint8 	enable;
    quint16 period;
    quint16 delay;
    quint8 	header;	// CAN/LIN
    quint32 id;
    QByteArray data;
    Vector::DBC::Message *pMsg;
} PeriodMessage;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow *getReference();

signals:
    void stopWorker();
    void cmd2Sender(const QString &cmdString);
    void setSenderData(quint32 id, const QByteArray &payload);
    void setSenderPriority();

public slots:
    void processReceivedMessages();
    
private slots:
    void on_actionConnect_triggered();
    void on_actionDevice_Config_triggered();
    void on_actionAbout_triggered();
    void on_cbEnable0x3C0_clicked();
    void on_cbSigZAS_Kl_S_clicked();
    void on_cbSigZAS_Kl_15_clicked();
    void on_cbEnable0x663_clicked();
    void on_cbSigBEM_MMI_Vorwarnung_clicked();
    void on_coSigBEM_02_Abschaltstufen_currentIndexChanged(int index);
    void on_cbEnable0x3DB_clicked();
    void on_cbSigBCM_Rueckfahrlicht_Anf_clicked();
    void on_cbEnable0x3DA_clicked();
    void on_cbEnable0x5F0_clicked();
    void on_slSigDI_KL_58xd_valueChanged(int value);
    void on_sbSigDI_KL_58xd_editingFinished();
    void on_slSigDI_KL_58xs_valueChanged(int value);
    void on_sbSigDI_KL_58xs_editingFinished();
    void on_cbEnable0x585_clicked();

private:
    void cusomizePreference();
    QWidget *getSignalWidget(const QString &name);
    int buildPeriodMessageEx(PeriodMessage *pPm, quint32 msgId, int bus = 0, int enable = 0);
	void initTxMessages();
	void updateSendingData(const PeriodMessage &pm);
    void updateSendingDataQuick(const PeriodMessage &pm);
    int updateSignalValue(quint32 id, QString signalName, double phyValue);
    PeriodMessage *getPMsg(quint32 id);
    void updateEnableValue(QCheckBox *cb, quint32 id);

    Ui::MainWindow *ui;
    static MainWindow *m_selfRef;
    XBusMgr *m_busMgr = NULL;
    int m_workingBus = 0;
    XFrameLogger *m_logger = NULL;    
    XFrameSender *m_sender = NULL;
    QThread *m_senderThread = NULL;
    ConnectDialog *m_connectDialog = NULL;
    DeviceConfig *m_configDialog = NULL;
    AboutDialog *m_aboutDialog = NULL;
    qint64 m_baseTime = -1;
    QMap<quint32, PeriodMessage> txMessages;

};

#endif // MAINWINDOW_H
