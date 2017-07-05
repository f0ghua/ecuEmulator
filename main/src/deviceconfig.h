#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <QDialog>
#include <QAction>
#include <QThread>

class XBusMgr;

namespace Ui {
class DeviceConfig;
}

class QRadioButton;
class QComboBox;
class QCheckBox;
class QLineEdit;
class Project;
class QTableWidget;
class QTableWidgetItem;

#define CONF_PROTO_NUM      16
#define LIN_SST_NUM         10
#define CAN_NODE_NUM       	2
#define PERIOD_MSG_LEN      16
#define PERIOD_MSG_NUM      16

typedef struct CAN_CONF_ST_t
{
public:
	quint16 freq;
	quint8  canType;
	quint8  trcvMode;
	quint8  swcanMode;
	quint16 opt;
} CAN_CONF_ST;

typedef struct LIN_SST_ST_t
{
public:
	quint8 enable;
	quint8 id;
	quint8 chksum;
	QByteArray data;

	enum {
		COL_ENABLE 	= 0,
		COL_ID      = 1,
		COL_PID     = 2,
		COL_DATA	= 3,
		//COL_CKSUM	= 3,
		COL_COUNT   = (COL_DATA+1)
	};
} LIN_SST_ST;

typedef struct LIN_CONF_ST_t
{
public:
	quint8 ver;
	quint32 baudrate;
	quint8 autochksum;
	LIN_SST_ST sst[LIN_SST_NUM];
} LIN_CONF_ST;

typedef struct{
	quint8  opt;
	quint8  stmin_ms;
	quint8  id_type;
	quint32 id;
	quint8  ext_id;
	quint8  configed;
	quint8  fc_stmin;
} ISO15765_2_TX_ST;

typedef struct{
	quint8   opt;
	quint8   id_type;
	quint32  id;
	quint8   ext_id;
	quint32  mask;
	quint8   ext_mask;
	quint8   configed;
} ISO15765_2_RX_ST;

typedef struct{
	quint8  opt;
	quint8  block_size;
	quint8  stmin;
	quint8  id_type;
	quint32 id;
	quint8  ext_id;
	quint8  configed;
} ISO15765_2_FC_ST;

typedef struct{
	ISO15765_2_TX_ST tx;
	ISO15765_2_RX_ST rx;
	ISO15765_2_FC_ST fc;
} ISO_CONF_ST; //ISO15765_2_CONF_ST;

typedef struct PERIOD_CONF_ST_t
{
	quint8 	enable;
	quint16 period;
	quint16 delay;
	quint8 	header;	// CAN/LIN
	QByteArray data;

public:
	enum {
		COL_ENABLE 	= 0,
		COL_PERIOD 	= 1,
		//COL_DELAY	= 2,
		COL_HEADER	= 2,
		COL_ID      = 3,
		COL_DATA	= 4,
		COL_COUNT   = (COL_DATA+1)
	};
} PERIOD_CONF_ST;

typedef struct SYS_CONF_ST_t
{
	quint32 magic1;
    quint8 	protocols[CONF_PROTO_NUM];
	CAN_CONF_ST canConf[CAN_NODE_NUM];
	LIN_CONF_ST linConf;
	//ISO_CONF_ST isoConf[CAN_NODE_NUM];
	PERIOD_CONF_ST periodConf[PERIOD_MSG_NUM];
	quint32 magic2;
} SYS_CONF_ST;

class DeviceConfig : public QDialog
{
	Q_OBJECT

public:

    enum CanConfReadOption {
        CanReadOptionNone           = 0,
        CanReadOptionFrequency      = 1 << 0,
        CanReadOptionType           = 1 << 1,
        CanReadOptionTrcvMode       = 1 << 2,
        CanReadOptionAll            = CanReadOptionFrequency | CanReadOptionType | CanReadOptionTrcvMode
    };
    Q_DECLARE_FLAGS(CanConfReadOptions, CanConfReadOption);

    enum LinConfReadOption {
        LinReadOptionNone           = 0,
        LinReadOptionVersion        = 1 << 0,
        LinReadOptionBaudrate       = 1 << 1,
        LinReadOptionAutoCksum      = 1 << 2,
        LinReadOptionSST            = 1 << 3,
        LinReadOptionAll            = LinReadOptionVersion | LinReadOptionBaudrate | LinReadOptionAutoCksum | LinReadOptionSST
    };
    Q_DECLARE_FLAGS(LinConfReadOptions, LinConfReadOption);

	static const char *Interfaces;
	static const char *dbc;
	static const char *ldf;
	static const char *notConfigured;

	explicit DeviceConfig(XBusMgr *mgr, QWidget *parent = 0);
	~DeviceConfig();
	int getHwInterfacesSize() {return hwInterfacesList.size();}
	QString hwInterface(int i) {return hwInterfacesList[i];}
	static const QStringList pageLabel;
	void loadBlankProject();
	void setHighlight(int m);
	void initAndShow();

signals:
	//void sendMsgRaw(const QByteArray &);
		
public slots:
	void handleCmdResponse(const QByteArray &);
	void changeEvent(QEvent *event);
	
private slots:
	void on_treeWidget_clicked(const QModelIndex &index);
	void on_radioTTypeHCAN2_clicked() {resetRadios(2, true, true, false, false, false);}
	void on_radioTTypeFTCAN2_clicked() {resetRadios(2, true, false, true, false, false);}
	void on_radioTTypeSCAN2_clicked() {resetRadios(2, true, false, true, true, true);}
	void on_radioTTypeHCAN1_clicked(){resetRadios(1, true, true, false, false, false);}
	void on_radioTTypeFTCAN1_clicked() {resetRadios(1, true, false, true, false, false);}
	void on_radioTTypeSCAN1_clicked() {resetRadios(1, true, false, true, true, true);}
    void on_btnCancel_clicked();
    void on_btnApply_clicked();
    void on_btnSave_clicked();
    void on_btnPmTblClear_clicked();
    void on_btnPmTblEnAll_clicked();
    void on_btnPmTblDisAll_clicked();
    void on_btnRefresh_clicked();
    void on_canFreq1_currentIndexChanged(const QString &arg1);
    void on_canFreq2_currentIndexChanged(const QString &arg1);
    void on_pbSSTAdd_clicked();
    void on_pbSSTDelete_clicked();
    void on_tblSST_itemChanged(QTableWidgetItem *item);
    void on_btnRestore_clicked();
    void on_btnReboot_clicked();
    void on_cbCAN1Enable_toggled(bool checked);
    void on_cbCAN2Enable_toggled(bool checked);
    void on_cbLIN1Enable_toggled(bool checked);

private:
	static const char *titleName;
	static const char *dbcTitle;
	static const char *dbcHint;
	static const char *ldfTitle;
	static const char *ldfHint;
	static const char *type;
	static const char *mode;
	static const char *freq;
	static const char *freqraw;
	static const char *version;

	XBusMgr *m_mgr;
	Ui::DeviceConfig *ui;
	QMap <QString, int>stackWidgetHelp;
	QStringList hwInterfacesList;
	QString currentHW;
    bool isQueryResponseTimeout;
    bool m_isReportConfigDone;
    bool m_isFeedBackCanEnable[2];
    bool m_isFeedBackLinEnable;
    CanConfReadOptions canConfReadOptions[2];
    LinConfReadOptions linConfReadOptions;
    
	SYS_CONF_ST m_sysConfRead;
	SYS_CONF_ST m_sysConfWrite;

	void newInterface(QString name, QString);
	void initHwDevAvailList(int);
	void resetRadios(int, bool normal, bool standby, bool sleep, bool hs, bool hv);
	void createHelper();
	bool interfaceIsAvail(QString);
	void getHwAvailableInterface();

	void fillInterfacesTree();
	void fillInterfacesData();
	void fillInterfaceTableWidget();
	void fillCANPageData(int idx);
	void fillLINPageData(int idx);
	void fillConfig();
		
	void feedbackInterfaceTree();
	void feedbackCAN(int, QCheckBox *, QRadioButton *, QRadioButton *, QRadioButton *, QRadioButton *,
					 QRadioButton *, QRadioButton *, QRadioButton *, QRadioButton *, QComboBox *, QLineEdit *);
	void feedbackLIN(int subBusId, QCheckBox *, QComboBox *, QLineEdit *, QCheckBox *);
	void feedbackConfig();
	void canModeCmdSet(int subBusId, QRadioButton *hsCan, QRadioButton *swCan, QRadioButton *ftCan,
						QRadioButton *normal, QRadioButton *standby, QRadioButton *sleep,
						QRadioButton *hs, QRadioButton *hv);
	void canFreqCmdSet(int subBusId, QComboBox *canfreq, QLineEdit *canfreqRaw);
    void canFreqCmdSet(int subBusId, QLineEdit *canfreqRaw);
    
	void addRow(QTableWidget *tableWidget, int colCount);
    void updatePeriodicMessageTable(int row);
	void fillPeriodicMessageTable();
	void feedbackPeriodicMessageTable();
	int parseSysConf(QByteArray &ba);
    void queryDeviceConfig();
    void resetPMSGTblRow(int row);
    void resetPeriodicMessageTable();

    void initSSTTable();
    void resetSSTTableRow(int row);
    void resetSSTTable();
    void updateSSTTable(int row);
    void fillSSTTable();
    void feedbackSSTTable();
    void querySSTReportConfig();
    LIN_SST_ST *sstGetFirstNullEntry();
    void presentDataInitialize();
    bool isBusProtocolIdxEnable(quint8 protocolIdx);
    bool isBusProtocolEnable(quint8 *protocols, quint8 protocol);
    int addBusProtocol(quint8 *protocols, quint8 protocol);
    void feedbackProtocols();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DeviceConfig::CanConfReadOptions)

#endif // DEVICECONFIG_H
