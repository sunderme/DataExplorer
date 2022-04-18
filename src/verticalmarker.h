#ifndef VERTICALMARKER_H
#define VERTICALMARKER_H

#include <QGraphicsLineItem>

class VerticalMarker : public QGraphicsLineItem
{
public:
    VerticalMarker(QGraphicsItem *parent = nullptr);

    void setXVal(qreal x);
    qreal xVal() const;
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    qreal m_xv;
};

#endif // VERTICALMARKER_H
