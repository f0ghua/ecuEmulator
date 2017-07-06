#include <QtGui>
#include "xwidgetwithbackground.h"

XWidgetWithBackground::XWidgetWithBackground(QWidget * parent) : QWidget(parent)
{
    m_pixmap = new QPixmap(size());
    m_modified = false;
}

XWidgetWithBackground::~XWidgetWithBackground()
{
    if (m_pixmap) {
        delete m_pixmap;
        m_pixmap = NULL;
    }
}

void XWidgetWithBackground::drawBackground()
{
    if (m_pixmap->size() != size() || m_modified ) {
        delete m_pixmap;
        m_pixmap = new QPixmap(size());
        m_modified=true;
        repaintBackground();
        m_modified=false;
    }
    QPainter painter(this);
    painter.drawPixmap(0,0,*m_pixmap);
}

void XWidgetWithBackground::updateWithBackground()
{
    m_modified=true;
    update();
}

bool XWidgetWithBackground::doRepaintBackground()
{
    return m_modified;
}

void XWidgetWithBackground::repaintBackground()
{
    m_pixmap->fill(QColor(0,0,0,0));
    QPainter painter(m_pixmap);
    paintBackground(painter);
}
