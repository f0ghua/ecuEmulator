#include "connectdialog.h"
#include "ui_connectdialog.h"
#include "xbusmgr.h"
#include "xcmdframe.h"
#include "utils.h"

#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>

//const char *ConnectDialog::selectDev = "Select...";
const char *ConnectDialog::CONS_disconnect = "Disconnect...";
const char *ConnectDialog::CONS_refeshDev = "Refresh...";

ConnectDialog::ConnectDialog(XBusMgr *mgr, QWidget *parent) :
    QDialog(parent),
    m_mgr(mgr),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);

/*
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
*/
    
    initHwDevAvailList();

    QObject::connect(m_mgr, 
        &XBusMgr::sigCmdFrameResponse,
        this, 
        &ConnectDialog::handleCmdResponse);
        
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

void ConnectDialog::initHwDevAvailList()
{
	m_mgr->getAllDevice(0);
}

void ConnectDialog::updateDeviceList(QStringList listDev)
{
	ui->m_cbDeviceList->clear();
	
	for(int i = 0; i < listDev.size(); i++)
		ui->m_cbDeviceList->addItem(listDev[i]);

	//ui->m_cbDeviceList->addItem(CONS_refeshDev);
	m_currentHW = "";
}

void ConnectDialog::updateDeviceConnState(int success)
{
    QByteArray raw;

	if (success == 0) { //connnected
		ui->m_lbDeviceStatus->setText(QStringLiteral("Connected"));

        // query device version
        isQueryResponseTimeout = false;
		ui->m_pteDeviceInfo->clear();
		
        m_mgr->initDevice();

#ifndef F_NO_DEBUG
        //qDebug() << tr("[%1], send cmd frame").arg(QDateTime::currentMSecsSinceEpoch());
#endif		
	    QTimer::singleShot(500, this, [=](){
            isQueryResponseTimeout = true;
        });

	} else {
		ui->m_lbDeviceStatus->setText(QStringLiteral("Disconnected"));
	}
}

void ConnectDialog::setHighlight(int m)
{
	ui->m_lbDeviceList->setStyleSheet((m == 0) ? "color:black;" : "color:red;");
}

void ConnectDialog::handleCmdResponse(const QByteArray &raw)
{
    if (isQueryResponseTimeout) {
        return;
    }

    XCmdFrame frame(raw);

    QString response = frame.handleCmdResponse(XCmdFrame::CMD_CFG_REQSWVER);
    if (!response.isEmpty()) {
        if (response.contains("ICITS")) {
            Utils::Base::isSaintDevice = false;
        } else {
            // only support ICITS device
            QMessageBox::warning(NULL,
                        tr("Warning"),
                        tr("Failed to connect to ICITS at that interface."),
                        QMessageBox::Cancel,
                        QMessageBox::Cancel);
            on_pbDisconnect_clicked();
        }

        ui->m_pteDeviceInfo->appendPlainText(response);
    }

}

void ConnectDialog::on_m_pbConnect_clicked()
{
    m_mgr->openDevice(0, ui->m_cbDeviceList->currentText());
    //ui->deviceList->setItemText(0, disconnect);
    m_currentHW = ui->m_cbDeviceList->currentText();
}

void ConnectDialog::on_m_cbDeviceList_activated(const QString &arg1)
{
/*
    if(arg1 == CONS_refeshDev) {
        initHwDevAvailList();
        m_currentHW = "";
    }
*/
}

void ConnectDialog::on_pbDisconnect_clicked()
{
    m_mgr->closeDevice(0);
    ui->m_pteDeviceInfo->clear();
    m_currentHW = "";
}

void ConnectDialog::on_m_pbCancel_clicked()
{
    QDialog::reject();
}

void ConnectDialog::on_m_pbRefresh_clicked()
{
    initHwDevAvailList();
    m_currentHW = "";
}

/*
bool ConnectDialog::eventFilter(QObject *watched, QEvent *event)
{

}
*/

