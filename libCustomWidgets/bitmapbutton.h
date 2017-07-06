#ifndef BITMAPBUTTON_H
#define BITMAPBUTTON_H

#include <QAbstractButton>
#include <QIcon> 
#include <QTimer> 

/** Two stat bitmap buttom may be used for touch screen keyboard button  */
class BitmapButton: public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY (QIcon downIcon READ downIcon WRITE setDownIcon)    

public:
    BitmapButton(QWidget * parent = 0);
    ~BitmapButton(); 
    QIcon downIcon() const ;
    void setDownIcon(const QIcon & icon);

protected slots:

protected: 
    void init();
    virtual void paintEvent (QPaintEvent *event);

    QIcon m_downIcon;

}; // BitmapButton

#endif // BITMAPBUTTON_H

