#ifndef ABMARKER_H
#define ABMARKER_H

#include <QGraphicsLineItem>
#include <QChart>

class ABMarker : public QGraphicsLineItem
{
public:
    ABMarker(QGraphicsItem *parent = nullptr);

    void setVal(QPointF p);
    QPointF val() const;
    void setChart(QChart *chart);

    virtual QRectF boundingRect()const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget) override;
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool m_lastStateSelected;

    QPointF m_p;
    QAbstractSeries *m_series;
    QChart *m_chart;
};

#endif // ABMARKER_H
