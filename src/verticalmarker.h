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

    virtual QRectF boundingRect()const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget) override;
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    struct Intersection{
        QAbstractSeries *series;
        QList<qreal> ys;
    };
    QList<qreal> intersectingPoints();

    qreal m_xv;
    QChart *m_chart;
    bool m_lastStateSelected;
};

#endif // VERTICALMARKER_H
