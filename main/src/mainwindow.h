#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include <QThread>

#include "DBC.h"

class QLineEdit;
class XBusMgr;
class XFrameSender;
class XBusFrame;
class ConnectDialog;
class AboutDialog;
class XFrameLogger;
class DeviceConfig;

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

private:
    void cusomizePreference();
    QWidget *getSignalWidget(const QString &name);
	void initTxMessages();
	
    Ui::MainWindow *ui;
    static MainWindow *m_selfRef;
    XBusMgr *m_busMgr = NULL;
    XFrameLogger *m_logger = NULL;    
    XFrameSender *m_sender = NULL;
    QThread *m_senderThread = NULL;
    ConnectDialog *m_connectDialog = NULL;
    DeviceConfig *m_configDialog = NULL;
    AboutDialog *m_aboutDialog = NULL;
    qint64 m_baseTime = -1;

};

#endif // MAINWINDOW_H
