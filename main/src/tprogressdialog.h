#ifndef TPROGRESSDIALOG_H
#define TPROGRESSDIALOG_H

#include <QTimer>
#include <QProgressDialog>

class TProgressDialog : public QProgressDialog {
public:
#define TPROGRESS_MAX_STEPS 100
    explicit TProgressDialog(int milliseconds, QWidget *parent) :
        QProgressDialog(parent),
        m_steps(0)
    {
        this->setMinimumWidth(300);
        this->setWindowTitle(tr("Please Wait ..."));
        //this->setLabelText(tr("processing ..."));
        this->setRange(0, TPROGRESS_MAX_STEPS);
        this->setModal(true);
        this->setCancelButton(0);
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint);

        m_timer = new QTimer(this);
        m_timer->setInterval(milliseconds/100);
        connect(m_timer, &QTimer::timeout, this, &TProgressDialog::handleTimeout);
        m_timer->start();
    }
    ~TProgressDialog() {}

public slots:
    void handleTimeout()
    {
        if( m_steps > TPROGRESS_MAX_STEPS ) {
            m_timer->stop();
            close();
        }
        else {
            this->setValue(m_steps++);
        }
    }

private:
    QTimer *m_timer;
    int m_steps;
};

#endif // TPROGRESSDIALOG_H
