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

    int h = availableGeometry.height() * 3 / 4;
    int w = h * 850 / 600;
    resize(w, h);
    setIconSize(QSize(16, 16));

    return;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cusomizePreference();
    m_logger = new XFrameLogger(this);
    m_busMgr = new XBusMgr(this);
    m_connectDialog = new ConnectDialog(m_busMgr, this);
    m_baseTime = -1;
    initTxMessages();

    connect(m_busMgr, &XBusMgr::sigUpdateDeviceList, m_connectDialog, &ConnectDialog::updateDeviceList);
    connect(m_busMgr, &XBusMgr::sigUpdateDeviceConnState, m_connectDialog, &ConnectDialog::updateDeviceConnState);
    connect(m_busMgr, &XBusMgr::frameReceived, this, &MainWindow::processReceivedMessages);

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
    emit cmd2Sender("en@1#id@0x061#bus@0#data@00112233#tr@1000ms");
    emit cmd2Sender("en@1#id@0x062#bus@0#data@00112233#tr@100ms#mo@D0=D0^0x01");
    emit cmd2Sender("en@1#id@0x063#bus@0#data@00112233#tr@10ms#mo@D0=D0^0x01");

    QTimer::singleShot(15000, this, [=](){
        emit cmd2Sender("id@0x061#data@001122334455");
    });

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
            // start log
            m_logger->startLog(g_logFileName, g_logFileMaxSize, g_logFileMaxBkpNumber);
        }

#ifndef F_NO_DEBUG
        //qDebug() << frame.toString(m_baseTime);
#endif
        m_logger->writeFrame(frame, m_baseTime);
    }
}

void MainWindow::initTxMessages()
{

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
