#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

class XBusMgr;

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(XBusMgr *mgr, QWidget *parent = 0);
    ~ConnectDialog();
    void setHighlight(int m);

public slots:
   	void updateDeviceConnState(int);
    void updateDeviceList(QStringList listDev);
    void handleCmdResponse(const QByteArray &);
	
private slots:
    void on_m_pbConnect_clicked();
    void on_m_cbDeviceList_activated(const QString &arg1);
    void on_pbDisconnect_clicked();
    void on_m_pbCancel_clicked();
    void on_m_pbRefresh_clicked();

private:
    void initHwDevAvailList();
    
	//static const char *selectDev;
	static const char *CONS_disconnect;
	static const char *CONS_refeshDev;

    XBusMgr *m_mgr;
    Ui::ConnectDialog *ui;
    QString m_currentHW;
    bool isQueryResponseTimeout;
};

#endif // CONNECTDIALOG_H
