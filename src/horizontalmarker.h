#ifndef HORIZONTALMARKER_H
#define HORIZONTALMARKER_H

#include <QGraphicsLineItem>

class HorizontalMarker : public QGraphicsLineItem
{
public:
    HorizontalMarker(QGraphicsItem *parent = nullptr);

    void setYVal(qreal y);
    qreal yVal() const;
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    qreal m_yv;
};

#endif // HORIZONTALMARKER_H
