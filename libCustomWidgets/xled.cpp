#include "xled.h"

#include <QPainter>

XLed::XLed(QWidget * parent) : XWidgetWithBackground(parent)
{
    m_checked = true; 
    m_color = Qt::red; 
    resize(330,330);
}

bool XLed::isChecked () const
{
    return m_checked; 
}

void XLed::setChecked(bool i)
{
    m_checked = i; 
    updateWithBackground();
    checkChanged( m_checked ); 
}

QColor XLed::color() const
{
    return m_color; 
}

void XLed::setColor(QColor i)
{ 
    m_color = i; 
    updateWithBackground(); 
}

void XLed::paintEvent(QPaintEvent * /* event*/ )
{
    QPainter painter(this); 
    initCoordinateSystem(painter); 
    // *** Draw circle */ 
    int h,s,v,a; 
    QColor c,back = color(); 
    c=back; 
  
    if (!m_checked || !isEnabled()) {
        back.getHsv(&h,&s,&v,&a);
        s*=0.20; 
        back.setHsv(h,s,v,a); 
    }
    painter.setBrush(back); 
  
    QPen pen;
    c.getHsv(&h,&s,&v,&a); 
    s*=0.5; 
    c.setHsv(h,s,v,a);  

    pen.setColor(c); 
    pen.setWidthF(3.0); 
    
    painter.drawEllipse(-149,-149,299,299);
    painter.end(); 
  
    drawBackground();
}

void XLed::paintBackground(QPainter & painter)
{
    initCoordinateSystem(painter); 
    painter.setPen(Qt::NoPen); 
    QRadialGradient shine(QPointF(-40.0,-40.0),120.0,QPointF(-40,-40));
    QColor white1(255,255,255,200);
    QColor white0(255,255,255,0);
  
    shine.setColorAt(0.0,white1); 
    shine.setColorAt(1.0,white0); 
  
    painter.setBrush(shine); 
    painter.drawEllipse(-147,-147,297,297); 
        
}

void XLed::initCoordinateSystem(QPainter & painter)
{
    int side = qMin(width(), height());

    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 330.0, side / 330.0);
}

