#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts>
#include <QtWidgets/QRubberBand>


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

    ZoomableChartView(QWidget *parent = 0);

    void zoomX(qreal factor, qreal xcenter);
    void zoomX(qreal factor);

    void zoomY(qreal factor, qreal ycenter);
    void zoomY(qreal factor);

    void clear();
    void addSeries(QXYSeries *series);
    void removeSeries(QXYSeries *series);
    void setTitle(const QString &title);

    //![2]
    ZoomMode zoomMode() const;
    void setZoomMode(const ZoomMode &zoomMode);

    void setChart(QChart *chart);
    QChart* chart() const;

    void addVerticalMarker(qreal x);
    void addHorizontalMarker(qreal x);

protected slots:
    void legendMarkerClicked();
    void legendMarkerHovered(bool hover);
    void seriesClicked(const QPointF &point);
    void seriesHovered(const QPointF &point,bool state);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);

    void setSeriesVisible(QAbstractSeries *series, bool visible = true);
    void emphasisSeries(QXYSeries *series, bool emphasis = true);

private:
    QGraphicsSimpleTextItem *m_coordX;
    QGraphicsSimpleTextItem *m_coordY;

    bool m_isTouching = false;
    QPointF m_lastMousePos;
    QPointF m_startMousePos;
    ZoomMode m_zoomMode = RectangleZoom;
    QChart *m_chart;

    static bool isAxisTypeZoomableWithMouse(const QAbstractAxis::AxisType type);
    QPointF getSeriesCoordFromChartCoord(const QPointF & mousePos, QAbstractSeries *series) const;
    QPointF getChartCoordFromSeriesCoord(const QPointF & seriesPos, QAbstractSeries *series) const;
};

#endif
