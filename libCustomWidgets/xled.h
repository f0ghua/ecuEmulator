#ifndef XLED_H
#define XLED_H

#include <QColor> 
#include "xwidgetwithbackground.h"

class XLed : public XWidgetWithBackground
{
    Q_OBJECT 
    Q_PROPERTY( bool m_checked READ isChecked 	WRITE setChecked)
    Q_PROPERTY( QColor m_color READ color 	WRITE setColor)
     
public:  
    XLed(QWidget *parent = 0);
    virtual ~XLed() {};
    void paintEvent(QPaintEvent *event);
    bool isChecked() const ; 
    QColor color() const; 
    void setColor(QColor); 
     
public slots: 
    void setChecked(bool); 

signals: 
    void checkChanged(bool val); 
     
protected:
    void initCoordinateSystem(QPainter &painter);
    void paintBackground(QPainter &painter);
    
protected: 
    bool m_checked; 
    QColor m_color; 

}; 
   
#endif // QLED_H 
