#include "aboutdialog.h"

#define APP_NAME 	"Tiny ECUSimulator"
#define APP_VERSION	"0.1.01"

const QDate g_buildDate = QLocale( QLocale::C ).toDate( QString( __DATE__ ).simplified(), "MMM d yyyy");
const QTime g_buildTime = QTime::fromString( __TIME__, "hh:mm:ss" );
	
#ifdef SUPPORT_DIP_SCALE
extern double g_dpiScaleValue;
#endif

/**

- QGridLayout
	- QHBoxLayout
		- QSpacerItem
		- QLabel
		- QSpacerItem
	- QLabel (Name/Version/ReleaseDate)
	- QGroupBox (Description)
		- QLable
	- QGroupBox (Copy Right)
		- QLable

 */
AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent)
{
	QFont font;
	font.setPointSize(9);
	//font.setFamily(QStringLiteral("Ping Hei"));
	font.setFamily(QStringLiteral("Comic Sans MS"));
	this->setFont(font);

	setWindowModality(Qt::NonModal);
	resize(400, 300);
	setWindowTitle(tr("About"));
	setWindowIcon(QIcon(":/images/tb_about.png"));
	m_gridLayout = new QGridLayout(this);

	m_horizontalLayout = new QHBoxLayout();
	m_horizontalSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	m_horizontalLayout->addItem(m_horizontalSpacer1);
	m_labelLogo = new QLabel(this);
	m_labelLogo->setMinimumSize(QSize(64, 64));
	m_labelLogo->setMaximumSize(QSize(64, 64));
	m_labelLogo->setText(QStringLiteral(""));
    m_labelLogo->setPixmap(QPixmap(QString::fromUtf8(":/images/ecuSimulator.png")));
	m_labelLogo->setScaledContents(true);
	m_labelLogo->setAlignment(Qt::AlignCenter);
	m_horizontalLayout->addWidget(m_labelLogo);
	m_horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	m_horizontalLayout->addItem(m_horizontalSpacer2);
	m_gridLayout->addLayout(m_horizontalLayout, 0, 0);

	m_labelBasicInfo = new QLabel(this);
	m_labelBasicInfo->setAlignment(Qt::AlignCenter);
	qDebug() << tr("data is %1").arg(QString( __DATE__ ));
	m_labelBasicInfo->setText(tr(
		"<h3><b>%1</b></h3>"
		"<p style='color:blue'>Version %2</p>"
		"<p style='color:blue'>Release Date: %3 %4<p>"
		).arg(APP_NAME).arg(APP_VERSION).arg(g_buildDate.toString("yyyy-MM-dd")).arg(g_buildTime.toString("hh:mm:ss"))
		);
	m_gridLayout->addWidget(m_labelBasicInfo);

	m_gbOptInfo1 = new QGroupBox(this);
	m_gbOptInfo1->setTitle(QStringLiteral("Description"));
	m_gbOptInfo1->setAlignment(Qt::AlignCenter);
	m_gbOptInfo1->setFlat(true);
	m_gridLayoutOptInfo1 = new QGridLayout(m_gbOptInfo1);
	m_labelOptInfo1 = new QLabel(this);
	m_labelOptInfo1->setWordWrap(true);
	m_labelOptInfo1->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	m_labelOptInfo1->setText(tr(
        "This tool is designed for engineers to deeply view CAN, LIN bus data activities, or emulate and test ECUs communication as a part of the whole electronic systems."
		)
		);
	m_gridLayoutOptInfo1->addWidget(m_labelOptInfo1, 0, 0, 1, 1);
	m_gridLayout->addWidget(m_gbOptInfo1);

	m_gbOptInfo2 = new QGroupBox(this);
	m_gbOptInfo2->setTitle(QStringLiteral("About Author"));
	m_gbOptInfo2->setAlignment(Qt::AlignCenter);
	m_gbOptInfo2->setFlat(true);
	m_gridLayoutOptInfo2 = new QGridLayout(m_gbOptInfo2);
	m_labelOptInfo2 = new QLabel(this);
	m_labelOptInfo2->setWordWrap(true);
	m_labelOptInfo2->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
	m_labelOptInfo2->setText(tr(
        "Copyright(C) 2017~2022 Suzhou Sapa Automotive Co Ltd. All right reserved."
		)
		);
	m_gridLayoutOptInfo2->addWidget(m_labelOptInfo2, 0, 0, 1, 1);
	m_gridLayout->addWidget(m_gbOptInfo2);

	//m_gridLayout->setColumnStretch(0, 1);
	m_gridLayout->setRowStretch(0, 2);
	m_gridLayout->setRowStretch(1, 1);
	m_gridLayout->setRowStretch(1, 1);
	m_gridLayout->setRowStretch(1, 1);

#ifdef SUPPORT_DIP_SCALE
	resize(width()*g_dpiScaleValue, height()*g_dpiScaleValue);
#endif
	//show();
}

AboutDialog::~AboutDialog()
{

}
