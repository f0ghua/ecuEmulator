#include <QPainter>
#include "bitmapbutton.h"

#define TIMERCYCLES 5
BitmapButton::BitmapButton(QWidget * parent) :
    QAbstractButton(parent)
{

}

BitmapButton::~BitmapButton()
{

}

QIcon BitmapButton::downIcon() const
{
  return m_downIcon;
}

void BitmapButton::setDownIcon(const QIcon & icon)
{
    m_downIcon = icon;
}

void BitmapButton::paintEvent ( QPaintEvent *  )
{
    QPainter painter(this);
    QIcon *  p_icon, tmp = icon();
    if( isDown() || isChecked())
        p_icon = &m_downIcon;
    else
        p_icon = &tmp;

    QSize aSize = p_icon->actualSize(size());
    painter.drawPixmap(QRect(0,0,width(),height()),
               p_icon->pixmap(aSize)
               ,QRect(0,0,aSize.width(),aSize.height())
               );
}
