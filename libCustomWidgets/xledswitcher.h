#ifndef XLEDSWITCHER_H
#define XLEDSWITCHER_H

#include "xled.h"

class XLedSwitcher : public XLed
{
    Q_OBJECT

public:
    XLedSwitcher(QWidget * parent = NULL);

signals:
    void clicked();

protected:
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    void toggleState();
};

#endif // XLEDSWITCHER_H
