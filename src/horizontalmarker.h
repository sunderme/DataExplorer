#ifndef HORIZONTALMARKER_H
#define HORIZONTALMARKER_H

#include <QGraphicsLineItem>
#include <QChart>

class HorizontalMarker : public QGraphicsLineItem
{
public:
    HorizontalMarker(QGraphicsItem *parent = nullptr);

    void setYVal(qreal y);
    qreal yVal() const;
    void setChart(QChart *chart);
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    qreal m_yv;
    QChart *m_chart;
};

#endif // HORIZONTALMARKER_H
