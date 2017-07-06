#ifndef XWIDGETWITHBACKGROUND_H
#define XWIDGETWITHBACKGROUND_H

#include <QWidget>

class QPixmap;

class XWidgetWithBackground : public QWidget
{
	Q_OBJECT

public:
    XWidgetWithBackground(QWidget * parent = 0);
    ~XWidgetWithBackground();
    void drawBackground();
    void updateWithBackground();
    bool doRepaintBackground(); 
     
protected:
    void repaintBackground();
    virtual void paintBackground (QPainter & painer) = 0;

protected:
    QPixmap *m_pixmap;
    bool m_modified;
};
#endif //XWIDGETWITHBACKGROUND_H

