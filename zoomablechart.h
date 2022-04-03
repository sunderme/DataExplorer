#ifndef ZOOMABLECHART_H
#define ZOOMABLECHART_H

#include <QtCharts/QChart>

class ZoomableChart : public QChart
{
    Q_OBJECT
public:
    ZoomableChart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::WindowFlags());
    void addSeries(QAbstractSeries *series);
    void removeSeries(QAbstractSeries *series);
    void addAxis(QAbstractAxis *axis, Qt::Alignment alignment);
signals:
    void seriesAdded(QAbstractSeries *series);
    void seriesRemoved(QAbstractSeries *series);
};

#endif // ZOOMABLECHART_H
