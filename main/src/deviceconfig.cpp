#include "deviceconfig.h"
#include "ui_deviceconfig.h"
#include "xbusmgr.h"
#include "xcmdframe.h"
//#include "serialworker.h"
#include "utils.h"
#include "tprogressdialog.h"

#include <QDir>
#include <QFileDialog>

enum {
    WGTIDX_BASICINFO = 0,
    WGTIDX_MESSAGES,
    WGTIDX_INTERFACES,
    WGTIDX_CAN1,
    WGTIDX_CAN2,
    WGTIDX_LIN1,
    WGTIDX_LIN2,
    WGTIDX_MANAGEMENT,
    WGTIDX_END
};

#define WGTIDX_BEGIN WGTIDX_BASICINFO

const QStringList DeviceConfig::pageLabel = {
    "Basic Info", "Cyclic Message", "Interface",
	"CAN 1", "CAN 2", "LIN 1", "LIN 2",
	"Management"
};

const char *DeviceConfig::type = "type";
const char *DeviceConfig::mode = "mode";
const char *DeviceConfig::freq = "freq";
const char *DeviceConfig::freqraw = "freqraw";

const char *DeviceConfig::version = "version";
const char *DeviceConfig::titleName = "Device Settings";
const char *DeviceConfig::dbcTitle = "Open DBC file";
const char *DeviceConfig::dbcHint = "Document files (*.dbc);;All files(*.*)";
const char *DeviceConfig::ldfTitle = "Open LDF file";
const char *DeviceConfig::ldfHint = "Document files (*.ldf);;All files(*.*)";

const char *DeviceConfig::Interfaces = "Interfaces";
const char *DeviceConfig::dbc = "DBC";
const char *DeviceConfig::ldf = "LDF";
const char *DeviceConfig::notConfigured = "Not Configured";

static SYS_CONF_ST g_defaultSysConf = {
    0x0000,
    // protocols
    {0x50, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // CAN
    {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}
    },
    // LIN
    {
        0x20, 19200, 1,
        {
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()},
            {0, 0, 0, QByteArray()}
        }
    },
    // PMSG
    {
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()},
        {0, 0, 0, 0, QByteArray()}
    },
    0x0000
};


DeviceConfig::DeviceConfig(XBusMgr *mgr, QWidget *parent) : 
	QDialog(parent), 
	m_mgr(mgr), 
	ui(new Ui::DeviceConfig)
{
	ui->setupUi(this);
	ui->grpCAN1SWMode->hide();
	ui->grpCAN2SWMode->hide();
	ui->canFreq1Raw->setPlaceholderText("XX XX");
	ui->canFreq2Raw->setPlaceholderText("XX XX");	
	initSSTTable();
	setWindowTitle(titleName);   
	getHwAvailableInterface();
	fillInterfaceTableWidget();
	fillInterfacesTree();
	createHelper();
	//loadBlankProject();
	//fillConfig();
	
    QObject::connect(m_mgr,
		//static_cast<void (Project:: *)(const QByteArray &)>(&Project::cmdFrameResponse), 
        &XBusMgr::sigCmdFrameResponse,
		this, 
		&DeviceConfig::handleCmdResponse);
		
}

static void clearSysConf(SYS_CONF_ST *sysConf)
{
    sysConf->magic1 = 0;
    for (int i = 0; i < CONF_PROTO_NUM; ++i) {
        sysConf->protocols[i] = 0;
    }
    
    for (int i = 0; i < CAN_NODE_NUM; i++) {
        sysConf->canConf[i].freq = 0;
        sysConf->canConf[i].canType = 0;
        sysConf->canConf[i].trcvMode = 0;
        sysConf->canConf[i].swcanMode = 0;
        sysConf->canConf[i].opt = 0;
    }
    
    sysConf->linConf.ver = 0;
    sysConf->linConf.baudrate = 0;
    sysConf->linConf.autochksum = 0;
    for (int i = 0; i < LIN_SST_NUM; i++) {
        sysConf->linConf.sst[i].enable = 0;
        sysConf->linConf.sst[i].id = 0;
        sysConf->linConf.sst[i].chksum = 0;
        sysConf->linConf.sst[i].data = QByteArray();   
    }

    for (int i = 0; i < PERIOD_MSG_NUM; i++) {
        sysConf->periodConf[i].enable = 0;
        sysConf->periodConf[i].period= 0;
        sysConf->periodConf[i].delay = 0;
        sysConf->periodConf[i].header = 0;
        sysConf->periodConf[i].data = QByteArray();
    }
    sysConf->magic2 = 0;
}

void DeviceConfig::presentDataInitialize()
{
    bool isConneted = m_mgr->getHwReady();
	for (int i = WGTIDX_BEGIN; i < WGTIDX_END; i++) {
        ui->stackedWidget->widget(i)->setEnabled(isConneted);
	}
	ui->cbCAN1Enable->setChecked(false);
    ui->widgetCAN1->setEnabled(false);
    ui->cbCAN2Enable->setChecked(false);
    ui->widgetCAN2->setEnabled(false);
    ui->cbLIN1Enable->setChecked(false);
    ui->widgetLIN1->setEnabled(false);
    
    resetPeriodicMessageTable();
    resetSSTTable();

    canConfReadOptions[0] = CanReadOptionNone;
    canConfReadOptions[1] = CanReadOptionNone;
    linConfReadOptions = LinReadOptionNone;
    m_isFeedBackCanEnable[0] = false;
    m_isFeedBackCanEnable[1] = false;
    m_isFeedBackLinEnable = false;
    
    //clearSysConf(&m_sysConfRead);
    m_sysConfRead = g_defaultSysConf;
    clearSysConf(&m_sysConfWrite);

    fillConfig();
    // reload the config from device
    queryDeviceConfig();
}

void DeviceConfig::initAndShow()
{
    presentDataInitialize();
    this->show();
}

void DeviceConfig::querySSTReportConfig()
{
    QByteArray raw = XCmdFrame::buildLinCmdGetSSTAll();
    m_isReportConfigDone = false;
    m_mgr->sendMsgRaw(raw);
}

void DeviceConfig::queryDeviceConfig()
{
    QByteArray raw;

    isQueryResponseTimeout = false;

    raw = XCmdFrame::buildCfgCmdGetSSBusProtocols();
    m_mgr->sendMsgRaw(raw);

    raw = XCmdFrame::buildCfgCmdGetVersion();
    m_mgr->sendMsgRaw(raw);

    raw = XCmdFrame::buildCfgCmdGetSerialNumber();
    m_mgr->sendMsgRaw(raw);
    
    raw = XCmdFrame::buildCfgCmdPeriodicMessageGetAll();
    m_mgr->sendMsgRaw(raw);

    if (Utils::Base::isSaintDevice) // saint doesn't support following commands
        return;
        
    raw = XCmdFrame::buildCanCmdGetFrequency(0);
    m_mgr->sendMsgRaw(raw);
    raw = XCmdFrame::buildCanCmdGetCanTransceiverCtl(0);
    m_mgr->sendMsgRaw(raw);
    
    raw = XCmdFrame::buildCanCmdGetFrequency(1);
    m_mgr->sendMsgRaw(raw);
    raw = XCmdFrame::buildCanCmdGetCanTransceiverCtl(1);
    m_mgr->sendMsgRaw(raw);
    
    raw = XCmdFrame::buildLinCmdGetVersion();
    m_mgr->sendMsgRaw(raw);
    raw = XCmdFrame::buildLinCmdGetBaudrate();
    m_mgr->sendMsgRaw(raw);
    raw = XCmdFrame::buildLinCmdGetAutoChecksum();
    m_mgr->sendMsgRaw(raw);

    querySSTReportConfig();
    
    QTimer::singleShot(2000, this, [=](){
        isQueryResponseTimeout = true;
        });
}

LIN_SST_ST *DeviceConfig::sstGetFirstNullEntry()
{
    for (int i = 0; i < LIN_SST_NUM; i++) {
        if (m_sysConfRead.linConf.sst[i].id == 0)
            return &m_sysConfRead.linConf.sst[i];
    }

    return NULL;
}

void DeviceConfig::handleCmdResponse(const QByteArray &raw)
{

    if (isQueryResponseTimeout) {
        return;
    }
    
    XCmdFrame frame(raw);
#ifndef F_NO_DEBUG
    qDebug() << QObject::tr("RX CMD[%1] RAW = %2").arg(frame.isProtocolCommandFrame()).arg(Utils::Base::formatByteArray(&raw));
#endif

    if (!frame.isValid() && (!frame.isCommandFrame())) {
        return;
    }
    
    XCmdFrame::CommandType type = frame.getCmdType();
    if (frame.isProtocolCommandFrame()) {
        switch ((quint8)type) {
        	case XCmdFrame::CMD_CAN_SETFREQ:
       		{
#ifndef F_NO_DEBUG
                //qDebug() << QObject::tr("got CMD_CAN_SETFREQ");
#endif            
                quint8 busType, busIndex;
                quint16 data;
                int rc = frame.parseCmdResponseGetFrequency(&busType, &busIndex, &data);
                if (rc == -1)
                    break;
#ifndef F_NO_DEBUG
                //qDebug() << QObject::tr("busType = %1, busIndex = %2").arg(busType).arg(busIndex);
#endif

                if ((busType == PROTOCOL_ID_CAN1)||(busType == PROTOCOL_ID_CAN2)) {
                    m_sysConfRead.canConf[busIndex].freq = data;
                    canConfReadOptions[busIndex] |= CanReadOptionFrequency;
                }

                break;
            }
            case XCmdFrame::CMD_CAN_SETTRMODE:
            {
                quint8 busIndex;
                quint8 type, mode;

                int rc = frame.parseCmdResponseGetCanTransCtrl(&busIndex, &type, &mode);
                if (rc == -1)
                    break;
                    
#ifndef F_NO_DEBUG
                //qDebug() << QObject::tr("get type = %1, mode = %2").arg(type).arg(mode);
#endif

                m_sysConfRead.canConf[busIndex].canType = type;
                m_sysConfRead.canConf[busIndex].trcvMode = mode;
                canConfReadOptions[busIndex] |= (CanReadOptionType|CanReadOptionTrcvMode);

                break;
            }
            case XCmdFrame::CMD_LIN_SETVERSION:
            {
                quint8 version;
                
                int rc = frame.parseCmdResponseGetLinVersion(&version);
                if (rc == -1)
                    break;
#ifndef F_NO_DEBUG                
                qDebug() << QObject::tr("version: %1.%2").arg(version>>4).arg(version&0x0F);
#endif

                m_sysConfRead.linConf.ver = version;
                Utils::Base::linVersion = version;
                linConfReadOptions |= LinReadOptionVersion;
                break;
            }
            case XCmdFrame::CMD_LIN_SETBAUDRATE:
            {
                quint32 baudrate;
                                    
                int rc = frame.parseCmdResponseGetBaudrate(&baudrate);
                if (rc == -1)
                    break;
#ifndef F_NO_DEBUG                
                qDebug() << QObject::tr("baudrate: %1").arg(baudrate);
#endif
                m_sysConfRead.linConf.baudrate = baudrate;
                linConfReadOptions |= LinReadOptionBaudrate;
                break;
            }
            case XCmdFrame::CMD_LIN_SETAUTOCKSUM:
            {
                quint8 cksumEnable;
                                                
                int rc = frame.parseCmdResponseGetAutoChecksum(&cksumEnable);
                if (rc == -1)
                    break;
#ifndef F_NO_DEBUG                
                qDebug() << QObject::tr("checksum enable: %1").arg(cksumEnable);
#endif
                m_sysConfRead.linConf.autochksum = cksumEnable;
                linConfReadOptions |= LinReadOptionAutoCksum;
                break;
            }
            case XCmdFrame::CMD_LIN_SSTREPORTCFG:
            {
                if (m_isReportConfigDone)
                    break;

                quint8 number, enable, pid, chksum;
                QByteArray data;

                int rc = frame.parseCmdResponseGetSSTConfig(&number, &enable, &pid, &data, &chksum);
                if (rc == -1)
                    break;

#ifndef F_NO_DEBUG                
                qDebug() << QObject::tr("SST: %1,%2,%3,%4, data = %5").\
                    arg(number).\
                    arg(enable).\
                    arg(Utils::Base::formatHexNum(pid)).\
                    arg(Utils::Base::formatHexNum(chksum)).\
                    arg(Utils::Base::formatByteArray(&data));
#endif
                if (number == 0xFF) {
                    m_isReportConfigDone = true;
                    fillSSTTable();

                    linConfReadOptions |= LinReadOptionSST;
                    break;
                }

                if (pid == 0)
                    break;
                    
                LIN_SST_ST *pSSTEntry;
                if ((pSSTEntry = sstGetFirstNullEntry()) == NULL)
                    break;

                pSSTEntry->enable = enable;
                pSSTEntry->id = (pid & 0x3F);
                pSSTEntry->chksum = chksum;
                pSSTEntry->data = data;

                break;
            }
        }
#ifndef F_NO_DEBUG
        //qDebug() << QObject::tr("canConfReadOptions[0] = %1").arg(canConfReadOptions[0]);
#endif
        if (canConfReadOptions[0] == CanReadOptionAll) { 
            fillCANPageData(1);
            //ui->stackedWidget->widget(WGTIDX_CAN1)->setEnabled(true);
            canConfReadOptions[0] = CanReadOptionNone;
            m_isFeedBackCanEnable[0] = true;
        }
        if (canConfReadOptions[1] == CanReadOptionAll) {
            fillCANPageData(2);
            //ui->stackedWidget->widget(WGTIDX_CAN2)->setEnabled(true);
            canConfReadOptions[1] = CanReadOptionNone;
            m_isFeedBackCanEnable[1] = true;
        }
        if (linConfReadOptions == LinReadOptionAll) {
            fillLINPageData(1);
            //ui->stackedWidget->widget(WGTIDX_LIN1)->setEnabled(true);
            linConfReadOptions = LinReadOptionNone;
            m_isFeedBackLinEnable = true;
        }

    } else {
        switch ((quint8)type) {
            case XCmdFrame::CMD_CFG_REQSWVER:
            {
                XCmdFrame frame(raw);
                QString response = frame.handleCmdResponse(XCmdFrame::CMD_CFG_REQSWVER);
                if (!response.isEmpty()) {
#ifndef F_NO_DEBUG                
                    qDebug() << QObject::tr("response = %1").arg(response);
#endif
                    QString name, version;
                    if (response.contains("ICITS")) {
                        name = "ICITS";
                        version = response.split(' ').last();

                        ui->lblDeviceName->setText(name);
                        ui->lblSoftwareVersion->setText(version.mid(1));
                    }

                    //ui->m_pteDeviceInfo->appendPlainText(response);
                }

                break;
            }
            case XCmdFrame::CMD_CFG_REQSN:
            {
                XCmdFrame frame(raw);
                QString response = frame.handleCmdResponse(XCmdFrame::CMD_CFG_REQSN);
                if (!response.isEmpty()) {
#ifndef F_NO_DEBUG                
                    qDebug() << QObject::tr("response = %1").arg(response);
#endif
                    ui->lblSerialNumber->setText(response);
                    //ui->m_pteDeviceInfo->appendPlainText(response);
                }

                break;
            }
            case XCmdFrame::CMD_CFG_REQSSBUS:
            {
                XCmdFrame frame(raw);

                int rc = frame.parseCmdResponseSSBusProtocol(m_sysConfRead.protocols, CONF_PROTO_NUM);
#ifndef F_NO_DEBUG      
                QString output;
                for (int i = 0; i < CONF_PROTO_NUM; ++i) {
                    output += QString::number(m_sysConfRead.protocols[i], 16).toUpper();
                    output += " ";
                }
                qDebug() << "protocols = " << output;
#endif
                break;
            }
        	case XCmdFrame::CMD_CFG_PMSG:
       		{
                int index;
                quint8 enable, header;
                quint16 period, delay;
                QByteArray data;
                int rc = frame.parseCmdResponsePeriodicMessage(&index, 
               		&enable, &period, &delay, &header, &data);
                if (rc == -1)
                    break;

                m_sysConfRead.periodConf[index].enable = enable;
                m_sysConfRead.periodConf[index].period = period;
                m_sysConfRead.periodConf[index].delay = delay;
                m_sysConfRead.periodConf[index].header = XCmdFrame::getBusTypeIndex(header);
                m_sysConfRead.periodConf[index].data = data;

#ifndef F_NO_DEBUG
               //qDebug() << QObject::tr("call updatePeriodicMessageTable");
#endif

                updatePeriodicMessageTable(index);
                break;
            }
        }
    }	
}

bool DeviceConfig::isBusProtocolIdxEnable(quint8 protocolIdx)
{
    for (int i = 0; i < CONF_PROTO_NUM; ++i) {
        if (m_sysConfRead.protocols[i] == XBusFrame::getBusTypeValue(protocolIdx))
            return true;
    }
    return false;
}

bool DeviceConfig::isBusProtocolEnable(quint8 *protocols, quint8 protocol)
{
    for (int i = 0; i < CONF_PROTO_NUM; ++i) {
        if (protocols[i] == protocol)
            return true;
    }

    return false;
}

int DeviceConfig::addBusProtocol(quint8 *protocols, quint8 protocol)
{
    for (int i = 0; i < CONF_PROTO_NUM; ++i) {
        if (protocols[i] == protocol)
            return 0;
    }

    for (int i = 0; i < CONF_PROTO_NUM; ++i) {
        if (protocols[i] == 0) {
            protocols[i] = protocol;
            return 1;
        }
    }

    return -1;
}


void DeviceConfig::fillInterfaceTableWidget()
{
	QString name;

	ui->tblInfWidget->resizeColumnsToContents();

	QFont font = ui->tblInfWidget->horizontalHeader()->font();
	font.setBold(true);
	ui->tblInfWidget->horizontalHeader()->setFont(font);
	ui->tblInfWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	ui->tblInfWidget->horizontalHeader()->resizeSection(0, 200);

	for(int i = 0; i < ui->tblInfWidget->rowCount(); i++) {
	    if (i < hwInterfacesList.count()) {
		    name = hwInterfacesList.at(i);
		    if(!interfaceIsAvail(name))
			    continue;

		    ui->tblInfWidget->item(i, 0)->setText(name);
		}
	}	
}

void DeviceConfig::addRow(QTableWidget *tableWidget, int colCount)
{
	int row = tableWidget->rowCount();
	tableWidget->insertRow(row);

#define TBL_ADDITEM(r, c, n) \
	QTableWidgetItem *item##n = new QTableWidgetItem;\
	item##n->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);\
	tableWidget->setItem(r, c, item##n);
	
	for (int col = 0; col < colCount; col++) {
		TBL_ADDITEM(row, col, col);
		//tableWidget->item(row, col)->setBackground(QColor(227,235,242));
	}

	tableWidget->setCurrentItem(tableWidget->item(0, 0));
}

void DeviceConfig::resetPMSGTblRow(int row)
{
    //qDebug() << tr("r = %1, c = %2").arg(row).arg(ui->tblPeriodicMsg->rowCount());
    if (row >= ui->tblPeriodicMsg->rowCount()) //{
        return;
    //}

    ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ENABLE)->setCheckState(Qt::Unchecked);
    ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_PERIOD)->setText("0");
    //ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DELAY)->setText(QString::number(conf.delay));
    //ui->tblPeriodicMsg->item(row, 3)->setText(QString::number(conf.protocol));
    QComboBox *combox = qobject_cast<QComboBox *>(ui->tblPeriodicMsg->cellWidget(row, PERIOD_CONF_ST::COL_HEADER));
    combox->setCurrentIndex(0);
    //ui->tblPeriodicMsg->setCellWidget(row, PERIOD_CONF_ST::COL_HEADER, (QWidget *)combox);  
    ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ID)->setText("");
    ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DATA)->setText("");
}

void DeviceConfig::resetPeriodicMessageTable()
{
	for (int row = 0; row < PERIOD_MSG_NUM; row++) {
        resetPMSGTblRow(row);
	}
}

void DeviceConfig::updatePeriodicMessageTable(int row)
{
    PERIOD_CONF_ST &conf = m_sysConfRead.periodConf[row];
    ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ENABLE)->setCheckState(conf.enable?Qt::Checked:Qt::Unchecked);
    ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_PERIOD)->setText(QString::number(conf.period));
    //ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DELAY)->setText(QString::number(conf.delay));
    //ui->tblPeriodicMsg->item(row, 3)->setText(QString::number(conf.protocol));
    QComboBox *combox = qobject_cast<QComboBox *>(ui->tblPeriodicMsg->cellWidget(row, PERIOD_CONF_ST::COL_HEADER));
    combox->setCurrentIndex(conf.header);
    //ui->tblPeriodicMsg->setCellWidget(row, PERIOD_CONF_ST::COL_HEADER, (QWidget *)combox);  
    //ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DATA)->setText(Utils::Base::formatByteArray(&conf.data));
    XBusFrame frame(conf.header, false, false, false, false, conf.data);
    if (frame.isValid()) { // at least pid included
        ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ID)->setText(Utils::Base::formatHexNum(frame.id()));
        ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DATA)->setText(Utils::Base::formatByteArray(&frame.payload()));
    }

}

void DeviceConfig::fillPeriodicMessageTable()
{
	for (int row = 0; row < PERIOD_MSG_NUM; row++) {
		addRow(ui->tblPeriodicMsg, PERIOD_CONF_ST::COL_COUNT);
		PERIOD_CONF_ST &conf = m_sysConfRead.periodConf[row];
		//ui->tblPeriodicMsg->item(row, 0)->setText(QString::number(conf.enable));
		ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ENABLE)->setCheckState(conf.enable?Qt::Checked:Qt::Unchecked);
		ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_PERIOD)->setText(QString::number(conf.period));
		//ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DELAY)->setText(QString::number(conf.delay));
		//ui->tblPeriodicMsg->item(row, 3)->setText(QString::number(conf.protocol));
		QComboBox *combox = new QComboBox();
		QStringList l = XCmdFrame::getBusTypeDescriptions();
		foreach (const QString s, l) {
		    combox->addItem(s);
		}
		combox->setStyleSheet("background-color: white");
		combox->setCurrentIndex(conf.header);
		ui->tblPeriodicMsg->setCellWidget(row, PERIOD_CONF_ST::COL_HEADER, (QWidget *)combox);

        XBusFrame frame(conf.header, false, false, false, false, conf.data);
        if (frame.isValid()) { // at least pid included
            ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ID)->setText(Utils::Base::formatHexNum(frame.id()));
            ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DATA)->setText(Utils::Base::formatByteArray(&frame.payload()));
        }
	}
}

void DeviceConfig::feedbackPeriodicMessageTable()
{
	QByteArray raw = QByteArray();
	
	for (int row = 0; row < PERIOD_MSG_NUM; row++) {
		PERIOD_CONF_ST &confRead = m_sysConfRead.periodConf[row];
		PERIOD_CONF_ST &confWrite = m_sysConfWrite.periodConf[row];
		
		confWrite.enable = ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ENABLE)->checkState()?1:0;
		confWrite.period = ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_PERIOD)->text().toInt();
		//confWrite.delay = ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DELAY)->text().toInt();

		QComboBox *cb = qobject_cast<QComboBox *>(ui->tblPeriodicMsg->cellWidget(row, PERIOD_CONF_ST::COL_HEADER));
		confWrite.header = cb->currentIndex();

		confWrite.data.clear();
		QString idStr = ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ID)->text();
		if (!idStr.isEmpty()) {
		    quint32 id = Utils::Base::parseHexStringToNum(idStr);
		    confWrite.data.append(XBusFrame::buildIdArray(id, confWrite.header));
        }
		QString dataStr = ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DATA)->text();
		confWrite.data.append(Utils::Base::hexString2ByteArray(dataStr));

#ifndef F_NO_DEBUG
		qDebug() << QObject::tr("PCF: enable = %1, period = %2, delay = %3, pro = %4, data = %5").\
			arg(confWrite.enable).\
			arg(confWrite.period).\
			arg(confWrite.delay).\
			arg(confWrite.header).\
			arg(confWrite.data.toHex().constData());
#endif

        raw.clear();
        
		if (!confWrite.data.isEmpty()) {
			if ((confRead.period != confWrite.period)
				//|| (confRead.delay != confWrite.delay)
				|| (confRead.header != confWrite.header)
				|| (confRead.data != confWrite.data)) {
#ifndef F_NO_DEBUG
				//qDebug() << QObject::tr("row %1 content changed, call cmd sender to update").arg(row);
#endif
				raw = XCmdFrame::buildCfgCmdSetPeriodicMessage(row, confWrite.enable, confWrite.period, 
					confWrite.delay, confWrite.header, confWrite.data);
                m_mgr->sendMsgRaw(raw);

                /*
			    if (!confWrite.enable) {
                    raw = XCmdFrame::buildCfgCmdPeriodicMessageEnable(row, !confWrite.enable);
                    pro->sendMsgRaw(raw);
			    }
			    */
			} else if (confRead.enable != confWrite.enable) {
#ifndef F_NO_DEBUG
				//qDebug() << QObject::tr("row %1 enable changed, call cmd sender to update").arg(row);
#endif
				raw = XCmdFrame::buildCfgCmdPeriodicMessageEnable(row, confWrite.enable);
				m_mgr->sendMsgRaw(raw);
			}

			
		} else {
            if (!confRead.data.isEmpty()) {
#ifndef F_NO_DEBUG
			    //qDebug() << QObject::tr("row %1 to be deleted, call cmd sender to update").arg(row);
#endif
			    raw = XCmdFrame::buildCfgCmdPeriodicMessageDelete(row);
			    m_mgr->sendMsgRaw(raw);
			}
		}	

		//if (!raw.isEmpty()) m_mgr->sendMsgRaw(raw);
	}
}

void DeviceConfig::initSSTTable()
{
    QStringList tblHeader;
    tblHeader<<"Enable"<<"ID"<<"PID"<<"Data";
    ui->tblSST->setColumnCount(LIN_SST_ST::COL_COUNT); 
    ui->tblSST->setHorizontalHeaderLabels(tblHeader);
    //ui->tblSST->setColumnWidth(LIN_SST_ST::COL_DATA, 220);
}

void DeviceConfig::resetSSTTableRow(int row)
{
    ui->tblSST->item(row, LIN_SST_ST::COL_ENABLE)->setCheckState(Qt::Unchecked);
    ui->tblSST->item(row, LIN_SST_ST::COL_ID)->setText("");
    ui->tblSST->item(row, LIN_SST_ST::COL_PID)->setText("");
    ui->tblSST->item(row, LIN_SST_ST::COL_DATA)->setText("");
}

void DeviceConfig::resetSSTTable()
{
    ui->tblSST->setRowCount(0);
    ui->tblSST->clearContents();
}

void DeviceConfig::updateSSTTable(int row)
{
    if ((row >= ui->tblSST->rowCount()) 
        || (row >= LIN_SST_NUM))
        return;
        
    LIN_SST_ST &conf = m_sysConfRead.linConf.sst[row];

    ui->tblSST->item(row, LIN_SST_ST::COL_ENABLE)->setCheckState(conf.enable?Qt::Checked:Qt::Unchecked);
    ui->tblSST->item(row, LIN_SST_ST::COL_ID)->setText(Utils::Base::formatHexNum(conf.id));
    //quint8 pid = XBusFrame::linCalculateIdParity(conf.id);
    //ui->tblSST->item(row, LIN_SST_ST::COL_PID)->setText(Utils::Base::formatHexNum(pid));
    ui->tblSST->item(row, LIN_SST_ST::COL_DATA)->setText(Utils::Base::formatByteArray(&conf.data));
    //ui->tblSST->item(row, LIN_SST_ST::COL_CKSUM)->setText(Utils::Base::formatHexNum(conf.chksum));
}

void DeviceConfig::fillSSTTable()
{
	for (int row = 0; row < LIN_SST_NUM; row++) {
	    LIN_SST_ST &conf = m_sysConfRead.linConf.sst[row];
	    
	    if (conf.id == 0) {
	        break;
	    }
	    
		addRow(ui->tblSST, LIN_SST_ST::COL_COUNT);
		QTableWidgetItem *item = ui->tblSST->item(row, LIN_SST_ST::COL_PID);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		item->setData(Qt::BackgroundRole, QBrush(Qt::lightGray));
        updateSSTTable(row);
	}
}

/*
static LIN_SST_ST *getSSTEntry(quint8 id, LIN_SST_ST *sstArray)
{
    for (int i = 0; i < LIN_SST_NUM; i++) {
        if (sstArray[i].id == id)
            return &sstArray[i];
    }
    
    return NULL;
}
*/

void DeviceConfig::feedbackSSTTable()
{
    QString s;
    QByteArray raw;
    
	for (int row = 0; row < ui->tblSST->rowCount(); row++) {
		LIN_SST_ST &confWrite = m_sysConfWrite.linConf.sst[row];
		
		confWrite.enable = ui->tblSST->item(row, LIN_SST_ST::COL_ENABLE)->checkState()?1:0;
		s = ui->tblSST->item(row, LIN_SST_ST::COL_ID)->text();
		if (!s.isEmpty()) {
		    confWrite.id = Utils::Base::parseHexStringToNum(s);
        }
        /*
		s = ui->tblSST->item(row, LIN_SST_ST::COL_CKSUM)->text();
		if (!s.isEmpty()) {
		    confWrite.chksum = Utils::Base::parseHexStringToNum(s);
        }
        */
		s = ui->tblSST->item(row, LIN_SST_ST::COL_DATA)->text();
		confWrite.data = Utils::Base::hexString2ByteArray(s);
    }

    // in order to make the present order right, we can first delete all, then add from the table
    raw = XCmdFrame::buildLinCmdSSTDeleteAll();
    m_mgr->sendMsgRaw(raw);
    
    for (int i = 0; i < LIN_SST_NUM; i++) {
        LIN_SST_ST &confWrite = m_sysConfWrite.linConf.sst[i];

        if (confWrite.id == 0)
            continue;

        raw = XCmdFrame::buildLinCmdSSTAddModify(confWrite.id, confWrite.data);
        m_mgr->sendMsgRaw(raw);
        if (!confWrite.enable) { // default is enable
            raw = XCmdFrame::buildLinCmdSSTEnable(confWrite.id, confWrite.enable);
            m_mgr->sendMsgRaw(raw);
        }
    }

#if 0
    // now process with add and modify
    for (int i = 0; i < LIN_SST_NUM; i++) {
        LIN_SST_ST &confWrite = m_sysConfWrite.linConf.sst[i];

        if (confWrite.id == 0)
            break;

        LIN_SST_ST *pSSTEntryRead = getSSTEntry(confWrite.id, m_sysConfRead.linConf.sst);
        if (pSSTEntryRead == NULL) { // new record
#ifndef F_NO_DEBUG
            qDebug() << QObject::tr("FB_SST: id = %1, new record find, add it.").arg(confWrite.id);
#endif
            raw = XCmdFrame::buildLinCmdSSTAddModify(confWrite.id, confWrite.data);
            m_mgr->sendMsgRaw(raw);

            if (!confWrite.enable) { // default is enable
                raw = XCmdFrame::buildLinCmdSSTEnable(confWrite.id, confWrite.enable);
                m_mgr->sendMsgRaw(raw);
            }

        } else { // check whether a original record has been changed

            if (confWrite.data != pSSTEntryRead->data) {
#ifndef F_NO_DEBUG
                qDebug() << QObject::tr("FB_SST: id = %1, old record has been modified.").arg(confWrite.id);
#endif
                raw = XCmdFrame::buildLinCmdSSTAddModify(confWrite.id, confWrite.data);
                m_mgr->sendMsgRaw(raw);
            }
            
            if (!confWrite.enable) { // default is enable
                raw = XCmdFrame::buildLinCmdSSTEnable(confWrite.id, confWrite.enable);
                m_mgr->sendMsgRaw(raw);
            } else if (confWrite.enable != pSSTEntryRead->enable) {
#ifndef F_NO_DEBUG
                qDebug() << QObject::tr("FB_SST: id = %1, old record enable changed.").arg(confWrite.id);
#endif
                raw = XCmdFrame::buildLinCmdSSTEnable(confWrite.id, confWrite.enable);
                m_mgr->sendMsgRaw(raw);
            }
        }
    }

    // now process with delete
    for (int i = 0; i < LIN_SST_NUM; i++) {
        LIN_SST_ST &confRead = m_sysConfRead.linConf.sst[i];
        if (confRead.id == 0)
            break;

        LIN_SST_ST *pSSTEntryWrite = getSSTEntry(confRead.id, m_sysConfWrite.linConf.sst);
        if (pSSTEntryWrite == NULL) {
#ifndef F_NO_DEBUG
            qDebug() << QObject::tr("FB_SST: id = %1, original record missing, delete it.").arg(confRead.id);
#endif
            raw = XCmdFrame::buildLinCmdSSTDelete(confRead.id);
            m_mgr->sendMsgRaw(raw);

        }
    }
#endif
}

void DeviceConfig::getHwAvailableInterface()
{
	hwInterfacesList << pageLabel[WGTIDX_CAN1] << pageLabel[WGTIDX_CAN2] << pageLabel[WGTIDX_LIN1];
}

bool DeviceConfig::interfaceIsAvail(QString inf)
{	
	return (hwInterfacesList.contains(inf));
}

void DeviceConfig::newInterface(QString name, QString en)
{
    Q_UNUSED(en);
    
	QTreeWidgetItem *inf = NULL, *interfaces = ui->treeWidget->topLevelItem(WGTIDX_INTERFACES);
	
	new QTreeWidgetItem(interfaces);
	inf = interfaces->child(interfaces->childCount() - 1);
	inf->setText(0, name);
#if 0
	inf->setFlags(inf->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);

	if(en == QString("1"))
		inf->setCheckState(0, Qt::Checked);
	else
		inf->setCheckState(0, Qt::Unchecked);
#endif
	if(interfaceIsAvail(name) == false)
		inf->setFlags(
		    inf->flags() 
		    & (~Qt::ItemIsEnabled) 
		    & (~Qt::ItemIsSelectable) 
		    & (~Qt::ItemIsUserCheckable)
		    );
}

void DeviceConfig::fillInterfacesTree()
{
	for(int i = 0; i < hwInterfacesList.size(); i++) {
		newInterface(hwInterfacesList[i], "1");
	}
	ui->treeWidget->expandAll();
	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
}

void DeviceConfig::fillInterfacesData()
{
	QStringList &inf = hwInterfacesList;

	for(int i = 0; i < inf.size(); i++) {
		QString &name = inf[i];
	    if(name == pageLabel[WGTIDX_CAN1])
		    fillCANPageData(1);
	    else if(name == pageLabel[WGTIDX_CAN2])
		    fillCANPageData(2);
	    else if(name == pageLabel[WGTIDX_LIN1])
		    fillLINPageData(1);
	    else if(name == pageLabel[WGTIDX_LIN2])
		    fillLINPageData(2);
	}
}

void DeviceConfig::fillCANPageData(int idx)
{
	QRadioButton *radioTTypeHCAN = ((idx == 1) ? ui->radioTTypeHCAN1 : ui->radioTTypeHCAN2);
	QRadioButton *radioTTypeSCAN = ((idx == 1) ? ui->radioTTypeSCAN1 : ui->radioTTypeSCAN2);
	QRadioButton *radioTTypeFTCAN = ((idx == 1) ? ui->radioTTypeFTCAN1 : ui->radioTTypeFTCAN2);
	QRadioButton *radioTModeNormal = ((idx == 1) ? ui->radioTModeNormal1 : ui->radioTModeNormal2);
	QRadioButton *radioTModeStandby = ((idx == 1) ? ui->radioTModeStandby1 : ui->radioTModeStandby2);
	QRadioButton *radioTModeSleep = ((idx == 1) ? ui->radioTModeSleep1 : ui->radioTModeSleep2);
	QRadioButton *radioTModeHighSpeed = ((idx == 1) ? ui->radioTModeHighSpeed1 : ui->radioTModeHighSpeed2);
	QRadioButton *radioTModeHighVol = ((idx == 1) ? ui->radioTModeHighVol1 : ui->radioTModeHighVol2);
	QComboBox *canSpeed = ((idx == 1) ? ui->canFreq1 : ui->canFreq2);
	QLineEdit *canfreqRaw = ((idx == 1) ? ui->canFreq1Raw : ui->canFreq2Raw);
    QCheckBox *canEnable = ((idx == 1) ? ui->cbCAN1Enable : ui->cbCAN2Enable);
    
	Q_ASSERT(idx > 0);

    quint8 protocolIdx = ((idx == 1)?BUS_CAN1:BUS_CAN2);

#ifndef F_NO_DEBUG
    qDebug() << tr("protocolIdx = %1, enable = %2").arg(protocolIdx).arg(isBusProtocolIdxEnable(protocolIdx));
#endif
    if (isBusProtocolIdxEnable(protocolIdx)) {
        canEnable->setChecked(true);
    } else {
        canEnable->setChecked(false);
    }

	CAN_CONF_ST &canConf = m_sysConfRead.canConf[idx-1];
	
	QString canType, canMode, canFreq;

	if (canConf.canType == 0) {
		radioTTypeHCAN->setChecked(true);
		resetRadios(idx, true, true, false, false, false);

	    if (canConf.trcvMode == 0)
		    radioTModeNormal->setChecked(true);
	    else if (canConf.trcvMode == 1)
		    radioTModeStandby->setChecked(true);
	}
	else if (canConf.canType == 1) {
		radioTTypeFTCAN->setChecked(true);
		resetRadios(idx, true, false, true, false, false);

        if (canConf.trcvMode == 1)
            radioTModeSleep->setChecked(true);
        else if (canConf.trcvMode == 3)
            radioTModeNormal->setChecked(true);
	}
	else if (canConf.canType == 2) {
		radioTTypeSCAN->setChecked(true);
		resetRadios(idx, true, false, true, true, true);
		
        if (canConf.trcvMode == 0)
            radioTModeSleep->setChecked(true);
        else if (canConf.trcvMode == 1)
            radioTModeHighSpeed->setChecked(true);
        else if (canConf.trcvMode == 2)
            radioTModeHighVol->setChecked(true);
        else if (canConf.trcvMode == 3)
            radioTModeNormal->setChecked(true);            
	}

	canfreqRaw->setText(QString::number(canConf.freq, 16).toUpper());

	int freq, index = 0;
	if ((freq = XCmdFrame::getBaudRateValueByData(canConf.freq)) != -1) {
		QString text = QString::number(freq);
		index = canSpeed->findText(text);	
	}

	canSpeed->setCurrentIndex(index);
}

void DeviceConfig::fillLINPageData(int idx)
{
	QComboBox *lversion = ui->linVersion1;
	QLineEdit *speed = ui->linSpeed1;
	QCheckBox *autoCksum = ui->ckAutoCksum;

	LIN_CONF_ST &linConf = m_sysConfRead.linConf;

    QCheckBox *linEnable = ui->cbLIN1Enable;
    
	Q_ASSERT(idx > 0);

    quint8 protocolIdx = BUS_LIN1;
    if (isBusProtocolIdxEnable(protocolIdx)) {
        linEnable->setChecked(true);
    } else {
        linEnable->setChecked(false);
    }

    QString text = XCmdFrame::linVersion2String(linConf.ver);
    int index = lversion->findText(text);
	lversion->setCurrentIndex(index);
	speed->setText(QString::number(linConf.baudrate));
	autoCksum->setCheckState(linConf.autochksum?Qt::Checked:Qt::Unchecked);
}

void DeviceConfig::fillConfig()
{
	fillPeriodicMessageTable();
	fillInterfacesData();
}

DeviceConfig::~DeviceConfig()
{
	delete ui;
}

void DeviceConfig::createHelper()
{
	for(int i = 0; i < pageLabel.size(); i++)
		stackWidgetHelp.insert(pageLabel[i], i);
}

void DeviceConfig::on_treeWidget_clicked(const QModelIndex &index)
{
	QString inf = index.data().toString();
	
	if((index.parent() != QModelIndex()) && interfaceIsAvail(inf) == false)
		return;
	
	ui->stackedWidget->setCurrentIndex(stackWidgetHelp[inf]);
}

void DeviceConfig::resetRadios(int idx, bool normal, bool standby, bool sleep, bool hs, bool hv)
{
	if(idx == 1) {
		ui->radioTModeNormal1->setEnabled(normal);
		ui->radioTModeStandby1->setEnabled(standby);
		ui->radioTModeSleep1->setEnabled(sleep);
		ui->radioTModeHighSpeed1->setEnabled(hs);
		ui->radioTModeHighVol1->setEnabled(hv);
	}
	else if(idx == 2) {
		ui->radioTModeNormal2->setEnabled(normal);
		ui->radioTModeStandby2->setEnabled(standby);
		ui->radioTModeSleep2->setEnabled(sleep);
		ui->radioTModeHighSpeed2->setEnabled(hs);
		ui->radioTModeHighVol2->setEnabled(hv);
	}
}

void DeviceConfig::canModeCmdSet(int subBusId, QRadioButton *hsCan, QRadioButton *swCan, QRadioButton *ftCan,
					QRadioButton *normal, QRadioButton *standby, QRadioButton *sleep,
					QRadioButton *hs, QRadioButton *hv)
{
	int bus = subBusId;
	char txvr = -1, mode = -1;
	
	if (hsCan->isChecked()) {
		txvr = 0;
		if (normal->isChecked())
			mode = 0;
		else if (standby->isChecked())
			mode = 1;		
	}
	else if (ftCan->isChecked()) {
		txvr = 1;
		if (sleep->isChecked())
			mode = 1;
		else if (normal->isChecked())
			mode = 3;	
	}
	else if (swCan->isChecked()) {
		txvr = 2;
		if (sleep->isChecked())
			mode = 0;
		else if (hs->isChecked())
			mode = 1;	
		else if (hv->isChecked())
			mode = 2;
		else if (normal->isChecked())
			mode = 3;
	}
	
#ifndef F_NO_DEBUG
    qDebug() << QObject::tr("txvr = %1, mode = %2, bus = %3").arg(txvr).arg(mode).arg(bus);
#endif

	if ((txvr == -1)||(mode == -1)||(bus < 0))
		return;

	QByteArray raw = XCmdFrame::buildCanCmdSetCanTransceiverCtl(bus, txvr, mode);
	m_mgr->sendMsgRaw(raw);
}

void DeviceConfig::canFreqCmdSet(int subBusId, QComboBox *canfreq, QLineEdit *canfreqRaw)
{
	QByteArray raw;
	int bus = subBusId;
	if (!canfreqRaw->text().isEmpty()) {
		QString hexStr = canfreqRaw->text().simplified();
		hexStr.replace(" ", "");
		QByteArray data = QByteArray::fromHex(hexStr.toLatin1());
		raw = XCmdFrame::buildCanCmdSetFrequency(bus, data);
	} else {
		int freq = canfreq->currentText().toInt();
		raw = XCmdFrame::buildCanCmdSetFrequency(bus, freq);
	}

	m_mgr->sendMsgRaw(raw);
}

void DeviceConfig::canFreqCmdSet(int subBusId, QLineEdit *canfreqRaw)
{
	QByteArray raw;
	int bus = subBusId;
	if (canfreqRaw->text().isEmpty()) {
        return;
	}
	
	QString hexStr = canfreqRaw->text().simplified();
	hexStr.replace(" ", "");
	QByteArray data = QByteArray::fromHex(hexStr.toLatin1());
	raw = XCmdFrame::buildCanCmdSetFrequency(bus, data);

	m_mgr->sendMsgRaw(raw);
}

void DeviceConfig::feedbackCAN(int subBusId, QCheckBox *canEnable, QRadioButton *hsCan, QRadioButton *swCan, QRadioButton *ftCan,
					QRadioButton *normal, QRadioButton *standby, QRadioButton *sleep,
					QRadioButton *hs, QRadioButton *hv, QComboBox *canfreq, QLineEdit *canfreqRaw)
{
	bool canModeChanged = false, freqChanged = false;
	CAN_CONF_ST &canConfRead = m_sysConfRead.canConf[subBusId];

    // if we hasn't get all of the config from device, don't set
    if (!m_isFeedBackCanEnable[subBusId]) {
        return;
    }

    quint8 protocolIdx = ((subBusId == 0)?BUS_CAN1:BUS_CAN2);
    if (canEnable->isChecked()) {
        addBusProtocol(m_sysConfWrite.protocols, XBusFrame::getBusTypeValue(protocolIdx));
    }
    
    quint16 freqRaw = canfreqRaw->text().toInt(NULL, 16);
    if (freqRaw && (freqRaw != canConfRead.freq)) {
#ifndef F_NO_DEBUG
        qDebug() << tr("freq changed, curr = %1, prev = %2").\
            arg(freqRaw, 4, 16, QChar('0')).arg(canConfRead.freq, 4, 16, QChar('0'));
#endif

        canFreqCmdSet(subBusId, canfreqRaw);
    }

#ifndef F_NO_DEBUG
    qDebug() << QObject::tr("canType = %1, trcvMode = %2; type: %3, %4, %5; mode: %6, %7, %8, %9, %10").\
        arg(canConfRead.canType).\
        arg(canConfRead.trcvMode).\
        arg(hsCan->isChecked()).\
        arg(ftCan->isChecked()).\
        arg(swCan->isChecked()).\
        arg(normal->isChecked()).\
        arg(standby->isChecked()).\
        arg(hs->isChecked()).\
        arg(sleep->isChecked()).\
        arg(hv->isChecked());
#endif

	if ((hsCan->isChecked() && (canConfRead.canType != 0))
		|| (ftCan->isChecked() && (canConfRead.canType != 1))
		|| (swCan->isChecked() && (canConfRead.canType != 2))
		|| (normal->isChecked() && (canConfRead.trcvMode != 0))
		|| (standby->isChecked() && (canConfRead.trcvMode != 2))
		|| (sleep->isChecked() && (canConfRead.trcvMode != 1))
		|| (hs->isChecked() && (canConfRead.trcvMode != 3))
		|| (hv->isChecked() && (canConfRead.trcvMode != 4)))
		canModeChanged = true;

#ifndef F_NO_DEBUG
    qDebug() << QObject::tr("canModeChanged = %1").arg(canModeChanged);
#endif

	if (!canModeChanged) {
		return;
	}
	
	canModeCmdSet(subBusId, hsCan, swCan, ftCan, normal, standby, sleep, hs, hv);
}

void DeviceConfig::feedbackLIN(int subBusId, QCheckBox *linEnable, QComboBox *lversion, QLineEdit *speed, QCheckBox *cksum)
{
    QByteArray raw;
	int idx = subBusId + WGTIDX_LIN1;
    LIN_CONF_ST &linConfWrite = m_sysConfWrite.linConf;
    LIN_CONF_ST &linConfRead = m_sysConfRead.linConf;

    // if we hasn't get all of the config from device, don't set
    if (!m_isFeedBackLinEnable) {
        return;
    }

    quint8 protocolIdx = BUS_LIN1;
    if (linEnable->isChecked()) {
        addBusProtocol(m_sysConfWrite.protocols, XBusFrame::getBusTypeValue(protocolIdx));
    }
    
    linConfWrite.ver = XCmdFrame::getLinVersionByDescription(lversion->currentText());

    if ((linConfWrite.ver != 0) && (linConfWrite.ver != linConfRead.ver)) {
        raw = XCmdFrame::buildLinCmdSetVersion(0, linConfWrite.ver);
	    m_mgr->sendMsgRaw(raw);
    }
    
	linConfWrite.baudrate = speed->text().toULong();
	if (linConfWrite.baudrate != linConfRead.baudrate) {
        raw = XCmdFrame::buildLinCmdSetBaudrate(0, linConfWrite.baudrate);
        m_mgr->sendMsgRaw(raw);
    }
    
    linConfWrite.autochksum = (cksum->checkState() == Qt::Unchecked) ? 0 : 1;
	if (linConfWrite.autochksum != linConfRead.autochksum) {
        raw = XCmdFrame::buildLinCmdSetAutoChecksum(0, linConfWrite.autochksum);
        m_mgr->sendMsgRaw(raw);
    }

    feedbackSSTTable();
}

void DeviceConfig::feedbackInterfaceTree()
{

}

void DeviceConfig::feedbackProtocols()
{
    bool needUpdate = false;
    quint8 *protocolsRead = m_sysConfRead.protocols;
    quint8 *protocolsWrite = m_sysConfWrite.protocols;
    
    for (int i = 0; i < CONF_PROTO_NUM; ++i) {
        if (protocolsRead[i] != 0) {
            if (!isBusProtocolEnable(protocolsWrite, protocolsRead[i])) {
                // not found in write protocols, remove it
                needUpdate = true;
                protocolsRead[i] = 0;
            }
        }
    }

    for (int i = 0; i < CONF_PROTO_NUM; ++i) {
        if (protocolsWrite[i] != 0) {
            if (addBusProtocol(protocolsRead, protocolsWrite[i]) == 1) {
                needUpdate = true;
            }
        }
    }

    if (needUpdate == true) {
        QByteArray baProtocols;
        for (int i = 0; i < CONF_PROTO_NUM; ++i) {
            if (protocolsRead[i] != 0) {
                baProtocols.append(protocolsRead[i]);
            }
        }
#ifndef F_NO_DEBUG
        qDebug() << "write protocols = " << Utils::Base::formatByteArray(&baProtocols);
#endif
        QByteArray raw = XCmdFrame::buildCfgCmdSetSSBusProtocols(baProtocols);
        m_mgr->sendMsgRaw(raw);
    }
}

void DeviceConfig::feedbackConfig()
{
	feedbackInterfaceTree();

	feedbackPeriodicMessageTable();

	feedbackCAN(0, ui->cbCAN1Enable, ui->radioTTypeHCAN1, ui->radioTTypeSCAN1, ui->radioTTypeFTCAN1, 
		ui->radioTModeNormal1, ui->radioTModeStandby1, ui->radioTModeSleep1, 
		ui->radioTModeHighSpeed1, ui->radioTModeHighVol1, 
		ui->canFreq1, ui->canFreq1Raw);
	feedbackCAN(1, ui->cbCAN2Enable, ui->radioTTypeHCAN2, ui->radioTTypeSCAN2, ui->radioTTypeFTCAN2, 
	    ui->radioTModeNormal2, ui->radioTModeStandby2, ui->radioTModeSleep2, 
	    ui->radioTModeHighSpeed2, ui->radioTModeHighVol2, 
	    ui->canFreq2, ui->canFreq2Raw);
	feedbackLIN(0, ui->cbLIN1Enable, ui->linVersion1, ui->linSpeed1, ui->ckAutoCksum);

	// compare write protocols and read protocols, if different set it
	feedbackProtocols();
}

void DeviceConfig::loadBlankProject()
{
/*
	if(blockInterfaces.count() == 0) {
		for(int i = WGTIDX_BEGIN; i < WGTIDX_END; i++) {

			blockInterfaces[pageLabel[i]] = QString::number(i <= WGTIDX_LIN1);
			blockInterfaces[pageLabel[i]][mode] = "0";
			if(i == WGTIDX_CAN1 || i == WGTIDX_CAN2) {
				blockInterfaces[pageLabel[i]][type] = "0";
				blockInterfaces[pageLabel[i]][freq] = "8";
			}
			else if(i == WGTIDX_LIN1 || i == WGTIDX_LIN2) {
				blockInterfaces[pageLabel[i]][version] = "5";
				blockInterfaces[pageLabel[i]][freq] = "115200";
			}
		}
		m_mgr->setConfModify(0);
	}
*/
}

void DeviceConfig::changeEvent(QEvent *event)
{   
    QDialog::changeEvent(event);
	return;
	
    if (event->type() == QEvent::ActivationChange)
    {
        if(this->isActiveWindow())
        {
            // widget is now active
			this->setWindowOpacity(1);

        }
        else
        {
            // widget is now inactive        
			this->setWindowOpacity(0.5);
        }
    }
}

void DeviceConfig::on_btnCancel_clicked()
{
    /*
	QTreeWidgetItem *interfaces = ui->treeWidget->topLevelItem(WGTIDX_INTERFACES);

	for(int i = interfaces->childCount() - 1; i >= 0; i--)
		delete interfaces->takeChild(i);

	fillConfig();
	*/
	
	QDialog::reject();
}

void DeviceConfig::on_btnApply_clicked()
{
	feedbackConfig();
	on_btnRefresh_clicked();
	//QDialog::accept();
}

void DeviceConfig::on_btnRefresh_clicked()
{
    presentDataInitialize();
}

void DeviceConfig::on_btnPmTblClear_clicked()
{
	 QList<QTableWidgetItem *> items = ui->tblPeriodicMsg->selectedItems();

	 if (items.count() <= 0)
	 	return;

	 for (int i = 0; i < items.count(); i++) {
		QTableWidgetItem *item = items[i];
	 	int col = item->column();
		switch (col) {
			case PERIOD_CONF_ST_t::COL_ENABLE:
				item->setCheckState(Qt::Unchecked);
				break;
			case PERIOD_CONF_ST_t::COL_PERIOD:
			//case PERIOD_CONF_ST_t::COL_DELAY:
				item->setText(QStringLiteral("0"));
				break;
			case PERIOD_CONF_ST_t::COL_HEADER:
			{
				QComboBox *cb = qobject_cast<QComboBox *>(ui->tblPeriodicMsg->cellWidget(item->row(), col));
				cb->setCurrentIndex(0);
				break;
			}
			case PERIOD_CONF_ST_t::COL_ID:
				item->setText("");
				break;
			case PERIOD_CONF_ST_t::COL_DATA:
				item->setText("");
				break;
			default:
				break;
		}
	 }
}

void DeviceConfig::on_btnPmTblEnAll_clicked()
{
	for (int row = 0; row < PERIOD_MSG_NUM; row++) {
        QString s = ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DATA)->text();
        if (!s.isEmpty()) {
            ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ENABLE)->setCheckState(Qt::Checked);
        }
	}
}

void DeviceConfig::on_btnPmTblDisAll_clicked()
{
	for (int row = 0; row < PERIOD_MSG_NUM; row++) {
	    PERIOD_CONF_ST &conf = m_sysConfRead.periodConf[row];
        QString s = ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_DATA)->text();
        if (!s.isEmpty()) {
            ui->tblPeriodicMsg->item(row, PERIOD_CONF_ST::COL_ENABLE)->setCheckState(Qt::Unchecked);
        }
	}
}

void DeviceConfig::on_canFreq1_currentIndexChanged(const QString &arg1)
{
    int freq = arg1.toInt();

    QByteArray ba = XCmdFrame::getBaudRateDataByValue(freq);

    if (!ba.isEmpty()) {
        ui->canFreq1Raw->setText(QString(ba.toHex().constData()).toUpper());
    } else {
        ui->canFreq1Raw->setText("");
    }
}

void DeviceConfig::on_canFreq2_currentIndexChanged(const QString &arg1)
{
    int freq = arg1.toInt();

    QByteArray ba = XCmdFrame::getBaudRateDataByValue(freq);

    if (!ba.isEmpty()) {
        ui->canFreq2Raw->setText(QString(ba.toHex().constData()).toUpper());
    } else {
        ui->canFreq2Raw->setText("");
    }
}


void DeviceConfig::on_pbSSTAdd_clicked()
{
    if (ui->tblSST->rowCount() >= LIN_SST_NUM) {
        return;
    }
    
    addRow(ui->tblSST, LIN_SST_ST::COL_COUNT);
    QTableWidgetItem *item = ui->tblSST->item(ui->tblSST->rowCount()-1, LIN_SST_ST::COL_PID);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    item->setData(Qt::BackgroundRole, QBrush(Qt::lightGray));

    resetSSTTableRow(ui->tblSST->rowCount()-1);
}

void DeviceConfig::on_pbSSTDelete_clicked()
{
    int rowIndex = ui->tblSST->currentRow();
    if (rowIndex != -1)
        ui->tblSST->removeRow(rowIndex);

}

void DeviceConfig::on_tblSST_itemChanged(QTableWidgetItem *item)
{
    if (item == NULL)
        return;

    int row = item->row();
    int column = item->column();
    
    if (column != LIN_SST_ST::COL_ID)
        return;

    QString s = item->text();
    quint8 id, pid;
    QTableWidget *table = item->tableWidget();
    
    if (!s.isEmpty()) {
        quint64 v = Utils::Base::parseHexStringToNum(s);
        id = v;
        if (v < 64) {
            pid = XBusFrame::linCalculateIdParity(id);
            table->item(row, LIN_SST_ST::COL_PID)->setText(Utils::Base::formatHexNum(pid));
        } else {
            table->item(row, LIN_SST_ST::COL_PID)->setText("Invalid");
        }
    } else {
        if (table->item(row, LIN_SST_ST::COL_PID))
            table->item(row, LIN_SST_ST::COL_PID)->setText("");
    }

}

void DeviceConfig::on_btnSave_clicked()
{
#ifdef DISABLE_BUTTON_EXECUTING
    ui->btnSave->setEnabled(false);
#endif
    QByteArray raw = XCmdFrame::buildCfgCmdSaveConfig();
    m_mgr->sendMsgRaw(raw);

    TProgressDialog dialog(1000, this);
    dialog.exec();

#ifdef DISABLE_BUTTON_EXECUTING
    QTimer::singleShot(1000, this, [=](){
        ui->btnSave->setEnabled(true);
    });
#endif
}

void DeviceConfig::on_btnRestore_clicked()
{
    QByteArray raw = XCmdFrame::buildCfgCmdRestoreConfig();
    m_mgr->sendMsgRaw(raw);

    TProgressDialog dialog(1000, this);
    dialog.exec();

    QTimer::singleShot(500, this, [=](){
        presentDataInitialize();
    });
}

void DeviceConfig::on_btnReboot_clicked()
{
    QByteArray raw = XCmdFrame::buildCfgCmdReset();
    m_mgr->sendMsgRaw(raw);

    TProgressDialog dialog(1000, this);
    dialog.exec();   

    QTimer::singleShot(500, this, [=](){
        presentDataInitialize();
    });    
}

void DeviceConfig::on_cbCAN1Enable_toggled(bool checked)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("on_cbCAN1Enable_toggled checked = %1").arg(checked);
#endif

    if (checked) {
        ui->widgetCAN1->setEnabled(true);
    } else {
        ui->widgetCAN1->setEnabled(false);
    }
}

void DeviceConfig::on_cbCAN2Enable_toggled(bool checked)
{
#ifndef F_NO_DEBUG
    qDebug() << tr("on_cbCAN2Enable_toggled checked = %1").arg(checked);
#endif

    if (checked) {
        ui->widgetCAN2->setEnabled(true);
    } else {
        ui->widgetCAN2->setEnabled(false);
    }
}

void DeviceConfig::on_cbLIN1Enable_toggled(bool checked)
{
    if (checked) {
        ui->widgetLIN1->setEnabled(true);
    } else {
        ui->widgetLIN1->setEnabled(false);
    }
}
