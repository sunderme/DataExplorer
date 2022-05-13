#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts>
#include <QtWidgets/QRubberBand>

#include "verticalmarker.h"
#include "horizontalmarker.h"
#include "abmarker.h"
#include "callout.h"


class ZoomableChartView : public QGraphicsView
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

    void zoom(qreal factor);
    void zoom(qreal factor, QPointF center);
    void zoomReset();

    void clear(bool recreateDroppedSeries=true);
    void addSeries(QXYSeries *series);
    void removeSeries(QXYSeries *series);
    void setTitle(const QString &title);

    ZoomMode zoomMode() const;
    void setZoomMode(const ZoomMode &zoomMode);

    void setChart(QChart *chart);
    QChart* chart() const;

    void addVerticalMarker();
    void addHorizontalMarker();
    void addMarker(bool markerB=false);

    bool deleteSelectedMarker();
    bool deleteSelectedSeries();

protected slots:
    void legendMarkerClicked();
    void legendMarkerHovered(bool hover);
    void seriesClicked(const QPointF &point);
    void seriesHovered(const QPointF &point,bool state);
    void updateMarker();
    void keepCallout();
    void tooltip(QPointF point, bool state);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

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

    Callout *m_tooltip;
    QList<Callout *> m_callouts;

    QList<VerticalMarker*> m_verticalMarkers;
    QList<HorizontalMarker*> m_horizontalMarkers;
    QList<ABMarker*> m_markers;

    QList<QAbstractSeries*> m_selectedSeries;

    static bool isAxisTypeZoomableWithMouse(const QAbstractAxis::AxisType type);
    QPointF getSeriesCoordFromChartCoord(const QPointF & mousePos, QAbstractSeries *series) const;
    QPointF getChartCoordFromSeriesCoord(const QPointF & seriesPos, QAbstractSeries *series) const;
    void movePointOnSeries(QPointF &p,QXYSeries *series) const;
    bool isMarkerSelected();
    ABMarker *getMarker(bool markerB);
};

#endif
