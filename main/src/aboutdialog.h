#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QtWidgets>

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AboutDialog(QWidget *parent = 0);
	~AboutDialog();

private:
	QGridLayout *m_gridLayout;
	QHBoxLayout *m_horizontalLayout;

	QSpacerItem *m_horizontalSpacer1;
	QLabel *m_labelLogo;
	QPixmap *m_logo;
	QSpacerItem *m_horizontalSpacer2;
	QLabel *m_labelBasicInfo;
	QGroupBox *m_gbOptInfo1;
	QGridLayout *m_gridLayoutOptInfo1;
	QLabel *m_labelOptInfo1;
	QGroupBox *m_gbOptInfo2;
	QGridLayout *m_gridLayoutOptInfo2;
	QLabel *m_labelOptInfo2;
};

#endif // ABOUTDIALOG_H
