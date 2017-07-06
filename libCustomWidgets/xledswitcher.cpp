#include "xledswitcher.h"
#include <QMouseEvent>

XLedSwitcher::XLedSwitcher(QWidget * parent) :
    XLed(parent)
{
    setColor(Qt::green);
    update();
}

void XLedSwitcher::toggleState()
{
    setChecked(!isChecked());
}

void XLedSwitcher::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        toggleState();
        emit clicked();
    }

    XLed::mouseReleaseEvent(event);
}
