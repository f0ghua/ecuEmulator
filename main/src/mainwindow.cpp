#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdialog.h"
#include "aboutdialog.h"
#include "xbusmgr.h"
#include "xframelogger.h"
#include "deviceconfig.h"
#include "utils.h"
#include "xcmdframe.h"
#include "xframesender.h"

#include <windows.h>

#include <QDesktopWidget>
#include <QFont>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

static const char g_logFileName[] = "./log.bf";
static const int g_logFileMaxSize = 1024*1024;
static const int g_logFileMaxBkpNumber = 2;
static const char *g_signal_0x5BF= "MFL_Tastencode_1";

MainWindow *MainWindow::m_selfRef = NULL;

MainWindow *MainWindow::getReference()
{
    return m_selfRef;
}

void MainWindow::cusomizePreference()
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    QFont font;
    int fontId = QFontDatabase::addApplicationFont(":/fonts/LucidaTypewriterRegular.ttf");
    if (fontId != -1) {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.empty()) {
            font.setFamily(families.at(0));
            font.setPointSize(8);
        }
    } else {
        //font.setFamily(QStringLiteral("Courier"));
        font.setPointSize(9);
    }
    qApp->setFont(font);


    // set the window size
    int h = availableGeometry.height() * 2 / 3;
    int w = h * 850 / 600;
    resize(w, h);
    setIconSize(QSize(16, 16));

    QFile file(":/ecuSimulator.qss");
    bool bOpened = file.open(QFile::ReadOnly);
    //assert (bOpened == true);
    if (bOpened) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet (styleSheet);
    }

    return;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->actionDevice_Config->setDisabled(true);
    ui->slSigDI_KL_58xd->setTracking(false);
    ui->slSigDI_KL_58xs->setTracking(false);
    cusomizePreference();

    m_busMgr = new XBusMgr(this);
    if (m_busMgr->getDbcNetwork(0) == NULL) {
        if (m_busMgr->getDbcNetwork(1) != NULL) {
            m_workingBus = 1;
        } else {
            QMessageBox::warning(NULL,
                        tr("Warning"),
                        tr("No DBC file exist. Please put file CAN1.dbc or CAN2.dbc at the same directory as ecuSimulator and restart the application."),
                        QMessageBox::Cancel,
                        QMessageBox::Cancel);
        }
    }
    m_connectDialog = new ConnectDialog(m_busMgr, this);

    connect(m_busMgr, &XBusMgr::sigUpdateDeviceList, m_connectDialog, &ConnectDialog::updateDeviceList);
    connect(m_busMgr, &XBusMgr::sigUpdateDeviceConnState, m_connectDialog, &ConnectDialog::updateDeviceConnState);
    connect(m_busMgr, &XBusMgr::frameReceived, this, &MainWindow::processReceivedMessages);

    m_baseTime = -1;
#ifdef LOG_ENABLE
    m_logger = new XFrameLogger(this);
#endif
    //m_busMgr->start();

    m_senderThread = new QThread;
    m_sender = new XFrameSender(m_busMgr);
    m_sender->moveToThread(m_senderThread);
    connect(m_senderThread, &QThread::started, m_sender, &XFrameSender::run);
    connect(m_sender, &XFrameSender::finished, m_senderThread, &QThread::quit);
    connect(m_sender, &XFrameSender::finished, m_sender, &XFrameSender::deleteLater);
    connect(m_senderThread, &QThread::finished, m_senderThread, &QThread::deleteLater);
    connect(this, &MainWindow::stopWorker, m_sender, &XFrameSender::stopThread);
    connect(this, &MainWindow::cmd2Sender, m_sender, &XFrameSender::slotCmdParser);
    connect(this, &MainWindow::setSenderData, m_sender, &XFrameSender::slotChangeData);
    connect(this, &MainWindow::setSenderPriority, m_sender, &XFrameSender::setWorkThreadPriority);
    m_senderThread->start();
    emit setSenderPriority();

    initTxMessages();

    connect(ui->pushButton_Arrow_A_Down_Left, &QPushButton::released, this, &MainWindow::on_pushButton_released);
    connect(ui->pushButton_Arrow_A_Up_Right, &QPushButton::released, this, &MainWindow::on_pushButton_released);
    connect(ui->pushButton_Up, &QPushButton::released, this, &MainWindow::on_pushButton_released);
    connect(ui->pushButton_Down, &QPushButton::released, this, &MainWindow::on_pushButton_released);
    connect(ui->pushButton_Volume_Down, &QPushButton::released, this, &MainWindow::on_pushButton_released);
    connect(ui->pushButton_Volume_Up, &QPushButton::released, this, &MainWindow::on_pushButton_released);
    connect(ui->pushButton_Mute, &QPushButton::released, this, &MainWindow::on_pushButton_released);
/*
    emit cmd2Sender("en@1#id@0x061#bus@0#data@00112233#tr@1000ms");
    emit cmd2Sender("en@1#id@0x062#bus@0#data@00112233#tr@100ms#mo@D0=D0^0x01");
    emit cmd2Sender("en@1#id@0x063#bus@0#data@00112233#tr@10ms#mo@D0=D0^0x01");

    QTimer::singleShot(15000, this, [=](){
        emit cmd2Sender("id@0x061#data@001122334455");
    });
*/
    return;
}

MainWindow::~MainWindow()
{
    delete ui;
}

QWidget *MainWindow::getSignalWidget(const QString &name)
{
    QString widgetName = name;
    QWidget *widget;
    widget = this->findChild<QLineEdit *>(widgetName);
    if (widget != NULL) {
        return widget;
    }

    return NULL;
}

void MainWindow::processReceivedMessages()
{
    if (!m_busMgr)
        return;

    while (m_busMgr->framesAvailable()) {
        const XBusFrame frame = m_busMgr->readFrame();

        if (!frame.isValid())
            continue;

        if (m_baseTime == -1) {
            m_baseTime = frame.timestamp();
#ifndef F_NO_DEBUG
            qDebug() << tr("reset base time to %1").arg(m_baseTime);
#endif
#ifdef LOG_ENABLE
            // start log
            m_logger->startLog(g_logFileName, g_logFileMaxSize, g_logFileMaxBkpNumber);
#endif
        }

#ifndef F_NO_DEBUG
        //qDebug() << frame.toString(m_baseTime);
#endif
#ifdef LOG_ENABLE
        m_logger->writeFrame(frame, m_baseTime);
#endif
    }
}

int MainWindow::buildPeriodMessageEx(PeriodMessage *pPm, quint32 msgId, int bus, int enable)
{
    Vector::DBC::Network *pNet = m_busMgr->getDbcNetwork(bus);
    if (pNet == NULL)
        return -1;
    Vector::DBC::Message *pMsg = pNet->findMsgByID(msgId);
    if (pMsg == NULL)
        return -1;

    pPm->enable = enable;
    pPm->header = bus;
    pPm->id = pMsg->id;
    QByteArray &payload = pPm->data;
    QByteArray ba(pMsg->size, 0);
    payload.clear();
    payload.append(ba);

    QMap<QString, Vector::DBC::Signal>::const_iterator end = pMsg->m_signals.constEnd();
    for (QMap<QString, Vector::DBC::Signal>::const_iterator ci = pMsg->m_signals.constBegin();
            ci != end;
            ci++) {
        const Vector::DBC::Signal *pSignal = &ci.value();
        bool ok;
        double rawValue = DBCHelper::getDefaultSignalValue(pSignal, pNet, &ok);
        pSignal->encode((uint8_t *)payload.data(), rawValue);
    }
    pPm->period =
        DBCHelper::getMessageCycleTime(pMsg, pNet, (quint8 *)payload.data());

    pPm->pMsg = pMsg;

    return 0;
}

void MainWindow::updateSendingData(const PeriodMessage &pm)
{
    QString cmdStr = QString("en@%1#id@%2#bus@%3#data@%4#tr@%5ms").\
        arg(pm.enable).\
        arg(pm.id).\
        arg(pm.header).\
        arg(pm.data.toHex().constData()).\
        arg(pm.period);

    emit cmd2Sender(cmdStr);
}

void MainWindow::updateSendingDataQuick(const PeriodMessage &pm)
{
    QString cmdStr = QString("en@%1#id@%2#bus@%3#data@%4").\
        arg(pm.enable).\
        arg(pm.id).\
        arg(pm.header).\
        arg(pm.data.toHex().constData());

    emit cmd2Sender(cmdStr);
}

void MainWindow::initTxMessages()
{
    PeriodMessage pm;
    
    if (0 == buildPeriodMessageEx(&pm, 0x3C0, m_workingBus)) {
        txMessages.insert(pm.id, pm);
        updateSendingData(pm);
    }
    
    if (0 == buildPeriodMessageEx(&pm, 0x663, m_workingBus)) {
        txMessages.insert(pm.id, pm);
        updateSendingData(pm);
    }

    if (0 == buildPeriodMessageEx(&pm, 0x3DA, m_workingBus)) {
        txMessages.insert(pm.id, pm);
        updateSendingData(pm);
    }

    if (0 == buildPeriodMessageEx(&pm, 0x3DB, m_workingBus)) {
        txMessages.insert(pm.id, pm);
        updateSendingData(pm);
    }

    if (0 == buildPeriodMessageEx(&pm, 0x5F0, m_workingBus)) {
        txMessages.insert(pm.id, pm);
        updateSendingData(pm);
    }

    if (0 == buildPeriodMessageEx(&pm, 0x585, m_workingBus)) {
        // all data should be filled with 0 as customer request
        for (int i = 0; i < pm.data.size(); ++i) {
            pm.data[i] = 0;
        }
        txMessages.insert(pm.id, pm);
        updateSendingData(pm);
    }

	if (0 == buildPeriodMessageEx(&pm, 0x5BF, m_workingBus)) {
        txMessages.insert(pm.id, pm);
        updateSendingData(pm);
    }
}

int MainWindow::updateSignalValue(quint32 id, QString signalName, double phyValue)
{
    PeriodMessage *pPm;
    if ((pPm = getPMsg(id)) == NULL)
        return -1;

    QByteArray &data = pPm->data;
    if (pPm->pMsg != NULL) {
        QMap<QString, Vector::DBC::Signal>::iterator it = pPm->pMsg->m_signals.find(signalName);
        if (it == pPm->pMsg->m_signals.end())
            return -1;

        Vector::DBC::Signal *pSignal = &it.value();
        double rawValue = pSignal->physicalToRawValue(phyValue);
        pSignal->encode((quint8 *)data.data(), rawValue);
    }

    updateSendingDataQuick(*pPm);
    return 0;
}

PeriodMessage *MainWindow::getPMsg(quint32 id)
{
    QMap<quint32, PeriodMessage>::iterator ci = txMessages.find(id);
    if (ci == txMessages.end()) {
        return NULL;
    }
    return &ci.value();
}

void MainWindow::on_actionConnect_triggered()
{
    if(m_connectDialog->exec() == QDialog::Accepted) {
    
    }
}

void MainWindow::on_actionDevice_Config_triggered()
{
    if (!m_configDialog) {
        m_configDialog = new DeviceConfig(m_busMgr, this);
        m_configDialog->setModal(false);
    }
    //m_deviceConfigDialog->setAttribute(Qt::WA_DeleteOnClose);
    //m_deviceConfigDialog->show();
    m_configDialog->initAndShow();

}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::updateEnableValue(QCheckBox *cb, quint32 id)
{
    quint8 enable;
    if (cb->isChecked()) {
        enable = 1;
    } else {
        enable = 0;
    }

    PeriodMessage *pPm;
    if ((pPm = getPMsg(id)) == NULL)
        return;
    pPm->enable = enable;
    updateSendingDataQuick(*pPm);
}

void MainWindow::on_cbEnable0x3C0_clicked()
{
    updateEnableValue(ui->cbEnable0x3C0, 0x3C0);
}

void MainWindow::on_cbSigZAS_Kl_S_clicked()
{
    double phyValue = 0;
    if (ui->cbSigZAS_Kl_S->isChecked()) {
        phyValue = 1;
    } else {
        phyValue = 0;
    }

    updateSignalValue(0x3C0, "ZAS_Kl_S", phyValue);
}

void MainWindow::on_cbSigZAS_Kl_15_clicked()
{
    double phyValue = 0;
    if (ui->cbSigZAS_Kl_15->isChecked()) {
        phyValue = 1;
    } else {
        phyValue = 0;
    }

    updateSignalValue(0x3C0, "ZAS_Kl_15", phyValue);
}

void MainWindow::on_cbEnable0x663_clicked()
{
    updateEnableValue(ui->cbEnable0x663, 0x663);
}

void MainWindow::on_cbSigBEM_MMI_Vorwarnung_clicked()
{
    double phyValue = 0;
    if (ui->cbSigBEM_MMI_Vorwarnung->isChecked()) {
        phyValue = 1;
    } else {
        phyValue = 0;
    }

    updateSignalValue(0x663, "BEM_MMI_Vorwarnung", phyValue);
}

void MainWindow::on_coSigBEM_02_Abschaltstufen_currentIndexChanged(int index)
{
    double phyValue = 0;
    phyValue = index; //ui->coSigBEM_02_Abschaltstufen->currentIndex();
    updateSignalValue(0x663, "BEM_02_Abschaltstufen", phyValue);
}

void MainWindow::on_cbEnable0x3DB_clicked()
{
    updateEnableValue(ui->cbEnable0x3DB, 0x3DB);
}

void MainWindow::on_cbSigBCM_Rueckfahrlicht_Anf_clicked()
{
    double phyValue = 0;
    if (ui->cbSigBCM_Rueckfahrlicht_Anf->isChecked()) {
        phyValue = 1;
    } else {
        phyValue = 0;
    }

    updateSignalValue(0x3DB, "BCM_Rueckfahrlicht_Anf", phyValue);
}

void MainWindow::on_cbEnable0x3DA_clicked()
{
    updateEnableValue(ui->cbEnable0x3DA, 0x3DA);
}

void MainWindow::on_cbEnable0x5F0_clicked()
{
    updateEnableValue(ui->cbEnable0x5F0, 0x5F0);
}

void MainWindow::on_slSigDI_KL_58xd_valueChanged(int value)
{
    double phyValue = 0;
    phyValue = value;
    updateSignalValue(0x5F0, "DI_KL_58xd", phyValue);
}

void MainWindow::on_sbSigDI_KL_58xd_editingFinished()
{
    double phyValue = 0;
    phyValue = ui->sbSigDI_KL_58xd->value();
    updateSignalValue(0x5F0, "DI_KL_58xd", phyValue);
}

void MainWindow::on_slSigDI_KL_58xs_valueChanged(int value)
{
    double phyValue = 0;
    phyValue = value;
    updateSignalValue(0x5F0, "DI_KL_58xs", phyValue);
}

void MainWindow::on_sbSigDI_KL_58xs_editingFinished()
{
    double phyValue = 0;
    phyValue = ui->sbSigDI_KL_58xs->value();
    updateSignalValue(0x5F0, "DI_KL_58xs", phyValue);
}

void MainWindow::on_cbEnable0x585_clicked()
{
    updateEnableValue(ui->cbEnable0x585, 0x585);
}

void MainWindow::on_cbEnable0x5bf_clicked()
{
    updateEnableValue(ui->cbEnable0x5bf, 0x5BF);
}

void MainWindow::on_pushButton_Volume_Up_pressed()
{
    double phyValue = 16;
    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}

void MainWindow::on_pushButton_Up_pressed()
{
    double phyValue = 4;
    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}

void MainWindow::on_pushButton_Down_pressed()
{
    double phyValue = 5;
    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}



void MainWindow::on_pushButton_Volume_Down_pressed()
{
    double phyValue = 17;

    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}

void MainWindow::on_pushButton_Arrow_A_Up_Right_pressed()
{
    double phyValue = 21;

    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}

void MainWindow::on_pushButton_Arrow_A_Down_Left_pressed()
{
    double phyValue = 22;

    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}

void MainWindow::on_pushButton_Mute_pressed()
{
    double phyValue = 32;
    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}

void MainWindow::on_pushButton_released()
{
    double phyValue = 0;
    updateSignalValue(0x5BF, g_signal_0x5BF, phyValue);
}
