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

    virtual QRectF boundingRect()const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget) override;
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    struct Intersection{
        QAbstractSeries *series;
        QList<qreal> xs;
    };
    QList<Intersection> intersectingPointsWithSeries();
    QList<qreal> intersectingPoints();
    bool m_lastStateSelected;

    qreal m_yv;
    QChart *m_chart;
};

#endif // HORIZONTALMARKER_H
