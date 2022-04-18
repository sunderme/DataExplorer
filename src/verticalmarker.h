#ifndef VERTICALMARKER_H
#define VERTICALMARKER_H

#include "qchart.h"
#include <QGraphicsLineItem>

class VerticalMarker : public QGraphicsLineItem
{
public:
    VerticalMarker(QGraphicsItem *parent = nullptr);

    void setXVal(qreal x);
    qreal xVal() const;
    void setChart(QChart *chart);
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    qreal m_xv;
    QChart *m_chart;
};

#endif // VERTICALMARKER_H
