#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts>
#include <QtWidgets/QRubberBand>

#include "verticalmarker.h"
#include "horizontalmarker.h"


//![1]
class ZoomableChartView : public QGraphicsView
        //![1]
{
public:
    enum ZoomMode {
        Pan,
        RectangleZoom,
        VerticalZoom,
        HorizontalZoom
    };
    enum MarkerData{
        Type,
        ValueX,ValueY
    };
    enum MarkerType{
        Vertical,Horizontal,XY
    };

    ZoomableChartView(QWidget *parent = 0);

    void zoom(qreal factor, QPointF center);

    void clear();
    void addSeries(QXYSeries *series);
    void removeSeries(QXYSeries *series);
    void setTitle(const QString &title);

    //![2]
    ZoomMode zoomMode() const;
    void setZoomMode(const ZoomMode &zoomMode);

    void setChart(QChart *chart);
    QChart* chart() const;

    void addVerticalMarker();
    void addHorizontalMarker();

protected slots:
    void legendMarkerClicked();
    void legendMarkerHovered(bool hover);
    void seriesClicked(const QPointF &point);
    void seriesHovered(const QPointF &point,bool state);
    void updateMarker();
    void updateMarkerArea(const QRectF &plotArea);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);

    void setSeriesVisible(QAbstractSeries *series, bool visible = true);
    void emphasisSeries(QXYSeries *series, bool emphasis = true);
    void scrollWithinPlot(qreal dx,qreal dy);

private:
    QGraphicsSimpleTextItem *m_coordX;
    QGraphicsSimpleTextItem *m_coordY;

    bool m_isTouching = false;
    QPointF m_lastMousePos;
    QPointF m_startMousePos;
    ZoomMode m_zoomMode = RectangleZoom;
    QChart *m_chart;

    QList<VerticalMarker*> m_verticalMarkers;
    QList<HorizontalMarker*> m_horizontalMarkers;
    QList<QGraphicsItem*> m_markers;

    static bool isAxisTypeZoomableWithMouse(const QAbstractAxis::AxisType type);
    QPointF getSeriesCoordFromChartCoord(const QPointF & mousePos, QAbstractSeries *series) const;
    QPointF getChartCoordFromSeriesCoord(const QPointF & seriesPos, QAbstractSeries *series) const;
};

#endif
