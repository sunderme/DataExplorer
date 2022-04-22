#include "zoomablechartview.h"
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGlobal>

#include "rangelimitedvalueaxis.h"

#include <QDebug>

ZoomableChartView::ZoomableChartView(QWidget *parent) :
    QGraphicsView(new QGraphicsScene, parent),m_chart(nullptr),m_tooltip(nullptr)
{
    setDragMode(QGraphicsView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // chart
    m_chart = new QChart;
    m_chart->setMinimumSize(640, 480);
    m_chart->setTitle("Hover the line to show callout. Click the line to make it stay");
    m_chart->legend()->hide();
    m_chart->createDefaultAxes();
    m_chart->setAcceptHoverEvents(true);

    setRenderHint(QPainter::Antialiasing);
    scene()->addItem(m_chart);

    m_coordX = new QGraphicsSimpleTextItem(m_chart);
    m_coordX->setPos(m_chart->size().width()/2 - 50, m_chart->size().height());
    m_coordX->setText("X: ");
    m_coordY = new QGraphicsSimpleTextItem(m_chart);
    m_coordY->setPos(m_chart->size().width()/2 + 50, m_chart->size().height());
    m_coordY->setText("Y: ");

    this->setMouseTracking(true);
    setZoomMode(RectangleZoom);
}
/*!
 * \brief zoom in/out at center
 * \param factor
 */
void ZoomableChartView::zoom(qreal factor)
{
    QPointF center=m_chart->plotArea().center();
    zoom(factor,center);
    updateMarker();
}
/*!
 * \brief set chart
 * \param chart
 */
void ZoomableChartView::setChart(QChart *chart)
{
    m_chart=chart;
    scene()->addItem(m_chart);
}
/*!
 * \brief get chart
 * \return
 */
QChart *ZoomableChartView::chart() const
{
    return m_chart;
}

void ZoomableChartView::mousePressEvent(QMouseEvent *event)
{
    m_isTouching = true;
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_lastMousePos = event->position();
#else
    m_lastMousePos = event->localPos();
#endif
    m_startMousePos=m_lastMousePos;
    QGraphicsView::mousePressEvent(event);
}

void ZoomableChartView::mouseMoveEvent(QMouseEvent *event)
{
    m_coordX->setText(QString("X: %1").arg(m_chart->mapToValue(event->pos()).x()));
    m_coordY->setText(QString("Y: %1").arg(m_chart->mapToValue(event->pos()).y()));

    if (!m_isTouching){
        m_lastMousePos = event->pos();
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if (dragMode() == ScrollHandDrag) {
        if (event->buttons() & Qt::LeftButton) {
            bool moveHorizontalAxis = false;
            for (const auto *axis : chart()->axes()) {
                if (axis->orientation() == Qt::Horizontal && isAxisTypeZoomableWithMouse(axis->type())) {
                    moveHorizontalAxis = true;
                    break;
                }
            }

            if (QGuiApplication::keyboardModifiers() & Qt::KeyboardModifier::ControlModifier)
                moveHorizontalAxis = !moveHorizontalAxis;

            if (moveHorizontalAxis) {
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                qreal dx = -(event->position().x() - m_lastMousePos.x());
#else
                qreal dx = -(event->localPos().x() - m_lastMousePos.x());
#endif
                for (auto *series : this->chart()->series()) {
                    for (const auto *axis : series->attachedAxes()) {
                        if (axis->orientation() != Qt::Horizontal)
                            continue;
                        if (axis->property("rangeLimited").toBool()) {
                            const auto *rangeLimitedAxis = static_cast<const RangeLimitedValueAxis*>(axis);
                            if (rangeLimitedAxis->orientation() != Qt::Horizontal)
                                continue;
                            QPointF oldPoint = getSeriesCoordFromChartCoord(m_lastMousePos, series);
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                            QPointF newPoint = getSeriesCoordFromChartCoord(event->position(), series);
#else
                            QPointF newPoint = getSeriesCoordFromChartCoord(event->localPos(), series);
#endif
                            qreal dxAxis = -(newPoint.x() - oldPoint.x());
                            if (rangeLimitedAxis->isLowerRangeLimited()
                                    && (rangeLimitedAxis->min() + dxAxis) < rangeLimitedAxis->lowerLimit()) {
                                dxAxis = -(rangeLimitedAxis->min() - rangeLimitedAxis->lowerLimit());
                                if (qFuzzyIsNull(dxAxis)) {
                                    dx = 0.0;
                                } else {
                                    QPointF dummyCoord(oldPoint.x() - dxAxis, m_lastMousePos.y());
                                    dummyCoord = getChartCoordFromSeriesCoord(dummyCoord, series);
                                    dx = (m_lastMousePos.x() - dummyCoord.x());
                                }
                            }


                            if (rangeLimitedAxis->isUpperRangeLimited()
                                    && (rangeLimitedAxis->max() + dxAxis) > rangeLimitedAxis->upperLimit()) {
                                dxAxis = -(rangeLimitedAxis->upperLimit() - rangeLimitedAxis->max());
                                if (qFuzzyIsNull(dxAxis)) {
                                    dx = 0.0;
                                } else {
                                    QPointF dummyCoord(oldPoint.x() - dxAxis, m_lastMousePos.y());
                                    dummyCoord = getChartCoordFromSeriesCoord(dummyCoord, series);
                                    dx = -(m_lastMousePos.x() - dummyCoord.x());
                                }
                            }
                        }
                    }
                }
                scrollWithinPlot(dx, 0);
            } else {
                qreal dy = event->pos().y() - m_lastMousePos.y();
                for (const auto series : this->chart()->series()) {
                    for (const auto axis : series->attachedAxes()) {
                        if (axis->orientation() != Qt::Vertical)
                            continue;
                        if (axis->property("rangeLimited").toBool()) {
                            RangeLimitedValueAxis *rangeLimitedAxis = static_cast<RangeLimitedValueAxis*>(axis);
                            if (rangeLimitedAxis->orientation() != Qt::Vertical)
                                continue;
                            QPointF oldPoint = getSeriesCoordFromChartCoord(m_lastMousePos, series);
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                            QPointF newPoint = getSeriesCoordFromChartCoord(event->position(), series);
#else
                            QPointF newPoint = getSeriesCoordFromChartCoord(event->localPos(), series);
#endif
                            qreal dyAxis = -(newPoint.y() - oldPoint.y());
                            if (rangeLimitedAxis->isLowerRangeLimited()
                                    && (rangeLimitedAxis->min() + dyAxis) < rangeLimitedAxis->lowerLimit()) {
                                dyAxis = rangeLimitedAxis->min() - rangeLimitedAxis->lowerLimit();
                                if (qFuzzyIsNull(dyAxis)) {
                                    dy = 0.0;
                                } else {
                                    QPointF dummyCoord(m_lastMousePos.x(), oldPoint.y() - dyAxis);
                                    dummyCoord = getChartCoordFromSeriesCoord(dummyCoord, series);
                                    dy = (m_lastMousePos.y() - dummyCoord.y());
                                }
                            }

                            if (rangeLimitedAxis->isUpperRangeLimited()
                                    && (rangeLimitedAxis->max() + dyAxis) > rangeLimitedAxis->upperLimit()) {
                                dyAxis = rangeLimitedAxis->upperLimit() - rangeLimitedAxis->max();
                                if (qFuzzyIsNull(dyAxis)) {
                                    dy = 0.0;
                                } else {
                                    QPointF dummyCoord(m_lastMousePos.x(), oldPoint.y() - dyAxis);
                                    dummyCoord = getChartCoordFromSeriesCoord(dummyCoord, series);
                                    dy = -(m_lastMousePos.y() - dummyCoord.y());
                                }
                            }
                        }
                    }
                }
                scrollWithinPlot(0, dy);
            }
        }
    }
    m_lastMousePos = event->pos();

    QGraphicsView::mouseMoveEvent(event);
}

void ZoomableChartView::wheelEvent(QWheelEvent *event)
{
    if (QGuiApplication::keyboardModifiers() & Qt::KeyboardModifier::ControlModifier){
        if (event->angleDelta().y() > 0) {
            zoom(1.4, event->position());
        } else if (event->angleDelta().y() < 0) {
            zoom(0.7, event->position());
        }
        updateMarker();
        return;
    }
    if (QGuiApplication::keyboardModifiers() & Qt::KeyboardModifier::ShiftModifier){
        scrollWithinPlot(event->angleDelta().y(), 0);
    }else{
        scrollWithinPlot(0,event->angleDelta().y());
    }
}

bool ZoomableChartView::isAxisTypeZoomableWithMouse(const QAbstractAxis::AxisType type)
{
    return (type == QAbstractAxis::AxisTypeValue
            || type == QAbstractAxis::AxisTypeLogValue
            || type == QAbstractAxis::AxisTypeDateTime);
}

QPointF ZoomableChartView::getSeriesCoordFromChartCoord(const QPointF &chartPos, QAbstractSeries *series) const
{
    auto const chartItemPos = chart()->mapFromScene(chartPos);
    auto const valueGivenSeries = chart()->mapToValue(chartItemPos, series);
    return valueGivenSeries;
}

QPointF ZoomableChartView::getChartCoordFromSeriesCoord(const QPointF &seriesPos, QAbstractSeries *series) const
{
    QPointF ret = chart()->mapToPosition(seriesPos, series);
    ret = chart()->mapFromScene(ret);
    return ret;
}

ZoomableChartView::ZoomMode ZoomableChartView::zoomMode() const
{
    return m_zoomMode;
}

void ZoomableChartView::setZoomMode(const ZoomMode &zoomMode)
{
    m_zoomMode = zoomMode;
    switch (zoomMode) {
    case Pan:
        //setRubberBand(QChartView::NoRubberBand);
        setDragMode(QGraphicsView::ScrollHandDrag);
        break;
    case RectangleZoom:
        setDragMode(QGraphicsView::RubberBandDrag);
        break;
    case HorizontalZoom:
    case VerticalZoom:
        //setRubberBand(QChartView::HorizontalRubberBand);
        setDragMode(QGraphicsView::RubberBandDrag);
        break;
    }
}
/*!
 * \brief add vertical marker at mouse cursor
 */
void ZoomableChartView::addVerticalMarker()
{
    const qreal x=m_lastMousePos.x();
    const QPointF val=m_chart->mapToValue(m_lastMousePos);
    const QRectF rect=m_chart->plotArea();
    VerticalMarker *lineItem=new VerticalMarker();
    lineItem->setLine(0,rect.bottom(),0,rect.top());
    lineItem->setXVal(val.x());
    lineItem->setChart(m_chart);
    QGraphicsScene *scene=chart()->scene();
    scene->addItem(lineItem);
    lineItem->setPos(x,0);
    m_verticalMarkers.append(lineItem);
}
/*!
 * \brief add horizontal marker at mouse cursor
 */
void ZoomableChartView::addHorizontalMarker()
{
    const qreal y=m_lastMousePos.y();
    const QPointF val=m_chart->mapToValue(m_lastMousePos);
    const QRectF rect=m_chart->plotArea();
    HorizontalMarker *lineItem=new HorizontalMarker();
    lineItem->setLine(rect.left(),0,rect.right(),0);
    lineItem->setYVal(val.y());
    lineItem->setChart(m_chart);
    QGraphicsScene *scene=chart()->scene();
    scene->addItem(lineItem);
    lineItem->setPos(0,y);
    m_horizontalMarkers.append(lineItem);
}

bool ZoomableChartView::deleteSelectedMarker()
{
    for(int i=0;i<m_verticalMarkers.size();++i){
        if(m_verticalMarkers[i]->isSelected()){
            VerticalMarker *item=m_verticalMarkers.takeAt(i);
            scene()->removeItem(item);
            return true;
        }
    }
    for(int i=0;i<m_horizontalMarkers.size();++i){
        if(m_horizontalMarkers[i]->isSelected()){
            HorizontalMarker *item=m_horizontalMarkers.takeAt(i);
            scene()->removeItem(item);
            return true;
        }
    }
    for(int i=0;i<m_callouts.size();++i){
        if(m_callouts[i]->isSelected()){
            Callout *item=m_callouts.takeAt(i);
            scene()->removeItem(item);
            return true;
        }
    }
    return false;
}

/*!
 * \brief zoom at center point
 * \param factor
 * \param center
 */
void ZoomableChartView::zoom(qreal factor, QPointF center)
{

    QRectF rect = chart()->plotArea();
    const QPointF oldCenter=rect.center();
    const QPointF transl=(oldCenter-center)/factor;
    const qreal widthOriginal = rect.width();
    const qreal heightOriginal = rect.height();

    rect.setWidth(widthOriginal/factor);
    rect.setHeight(heightOriginal/factor);

    rect.moveCenter(center);
    rect.translate(transl);

    chart()->zoomIn(rect);
}
/*!
 * \brief reset zoom
 * handle markers and pan
 */
void ZoomableChartView::zoomReset()
{
    m_chart->zoomReset();
    updateMarker();
}

/*!
 * \brief remove all series from chart
 */
void ZoomableChartView::clear()
{
    m_chart->removeAllSeries();
    m_chart->legend()->hide();
}
/*!
 * \brief add series to chart and connect legend marker for hide/hover
 * \param series
 */
void ZoomableChartView::addSeries(QXYSeries *series)
{
    m_chart->addSeries(series);
    m_chart->createDefaultAxes();
    if(m_chart->series().length()>1){
        m_chart->legend()->show();
    }
    //QObject::connect(series, &QLineSeries::clicked,this, &ZoomableChartView::seriesClicked);
    //QObject::connect(series, &QLineSeries::clicked, this, &ZoomableChartView::keepCallout);
    QObject::connect(series, &QLineSeries::hovered, this, &ZoomableChartView::tooltip);
    const auto markers = m_chart->legend()->markers(series);
    for (auto marker : markers) {
        QObject::connect(marker, &QLegendMarker::clicked,
                         this, &ZoomableChartView::legendMarkerClicked);
        QObject::connect(marker, &QLegendMarker::hovered,
                         this, &ZoomableChartView::legendMarkerHovered);
    }
}
/*!
 * \brief remove signal/slot connect when series is removed
 * Basically unused.
 * \param series
 */
void ZoomableChartView::removeSeries(QXYSeries *series)
{
    // Connect all markers to handler
    if(!m_chart) return;
    QObject::disconnect(series, &QLineSeries::clicked,this, &ZoomableChartView::seriesClicked);
    QObject::disconnect(series, &QLineSeries::hovered,this, &ZoomableChartView::seriesHovered);
    const auto markers = m_chart->legend()->markers(series);
    for (auto marker : markers) {
        QObject::disconnect(marker, &QLegendMarker::clicked,
                            this, &ZoomableChartView::legendMarkerClicked);
        QObject::disconnect(marker, &QLegendMarker::hovered,
                            this, &ZoomableChartView::legendMarkerHovered);
    }
    m_chart->removeSeries(series);
}
/*!
 * \brief set chart title
 * \param title
 */
void ZoomableChartView::setTitle(const QString &title)
{
    m_chart->setTitle(title);
}
/*!
 * \brief resizeEvent to fill widget with the complete chart
 * \param event
 */
void ZoomableChartView::resizeEvent(QResizeEvent *event)
{
    if (scene()) {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
         m_chart->resize(event->size());
         m_coordX->setPos(m_chart->size().width()/2 - 50, m_chart->size().height() - 20);
         m_coordY->setPos(m_chart->size().width()/2 + 50, m_chart->size().height() - 20);
         /*const auto callouts = m_callouts;
         for (Callout *callout : callouts)
             callout->updateGeometry();*/
    }
    QGraphicsView::resizeEvent(event);
}

/*!
 * \brief mouseReleaseEvent for local zoom
 * \param event
 */
void ZoomableChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(!rubberBandRect().isNull() && m_zoomMode==RectangleZoom){
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_lastMousePos = event->position();
#else
        m_lastMousePos = event->localPos();
#endif
        QRectF rect(m_startMousePos,m_lastMousePos);
        chart()->zoomIn(rect.normalized());
        updateMarker();
    }
    if(!rubberBandRect().isNull() && m_zoomMode==HorizontalZoom){
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_lastMousePos = event->position();
#else
        m_lastMousePos = event->localPos();
#endif
        const QRectF ro=m_chart->plotArea();
        QRectF rect(m_startMousePos,m_lastMousePos);
        rect=rect.normalized();
        rect.setHeight(ro.height());
        chart()->zoomIn(rect.normalized());
        updateMarker();
    }
    if(!rubberBandRect().isNull() && m_zoomMode==VerticalZoom){
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_lastMousePos = event->position();
#else
        m_lastMousePos = event->localPos();
#endif
        const QRectF ro=m_chart->plotArea();
        QRectF rect(m_startMousePos,m_lastMousePos);
        rect=rect.normalized();
        rect.setWidth(ro.width());
        chart()->zoomIn(rect.normalized());
        updateMarker();
    }
    m_isTouching = false;
    QGraphicsView::mouseReleaseEvent(event);
}

/*!
 * \brief keyPressEvent on chart view
 * \param event
 */
void ZoomableChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoom(1.4);
        break;
    case Qt::Key_Minus:
        chart()->zoom(0.7);
        break;
        //![1]
    case Qt::Key_Left:
        scrollWithinPlot(-10, 0);
        break;
    case Qt::Key_Right:
        scrollWithinPlot(10, 0);
        break;
    case Qt::Key_Up:
        scrollWithinPlot(0, 10);
        break;
    case Qt::Key_Down:
        scrollWithinPlot(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

/*!
 * \brief change series visibility when legendMarker is clicked
 */
void ZoomableChartView::legendMarkerClicked()
{
    auto* marker = qobject_cast<QLegendMarker*>(sender());
    Q_ASSERT(marker);

    // Toggle visibility of series
    setSeriesVisible(marker->series(), !marker->series()->isVisible());
}
/*!
 * \brief highlight series when hovering over legen marker
 * Not working properly !!!
 * \param hover
 *
 */
void ZoomableChartView::legendMarkerHovered(bool hover)
{
    auto* marker = qobject_cast<QLegendMarker*>(sender());
    Q_ASSERT(marker);

    QFont font = marker->font();
    font.setBold(hover);
    marker->setFont(font);

    if (marker->series()->type() == QAbstractSeries::SeriesTypeLine) {
        auto series = qobject_cast<QLineSeries*>(marker->series());
        emphasisSeries(series,hover);
    }
}
/*!
 * \brief slot for series clicked
 * yet unused
 * \param point
 */
void ZoomableChartView::seriesClicked(const QPointF &point)
{
    auto *series=qobject_cast<QXYSeries*>(sender());
    //emphasisSeries(series);
}
/*!
 * \brief slot for hover over series
 * \param point
 * \param state
 */
void ZoomableChartView::seriesHovered(const QPointF &point, bool state)
{
    auto *series=qobject_cast<QXYSeries*>(sender());
    emphasisSeries(series,state);
    QList<QLegendMarker*> lst=m_chart->legend()->markers(series);
    for(QLegendMarker* marker:lst){
        QFont font = marker->font();
        font.setBold(state);
        marker->setFont(font);
    }
}

void ZoomableChartView::updateMarker()
{
    for(VerticalMarker *item:m_verticalMarkers){
        qreal x=item->xVal();
        x=m_chart->mapToPosition(QPointF(x,x)).x();
        item->setPos(x,0);
    }
    for(HorizontalMarker *item:m_horizontalMarkers){
        qreal y=item->yVal();
        y=m_chart->mapToPosition(QPointF(y,y)).y();
        item->setPos(0,y);
    }
}

/*!
 * \brief set series in chart visible
 * \param series
 * \param visible
 */
void ZoomableChartView::setSeriesVisible(QAbstractSeries *series, bool visible)
{
    if(!m_chart) return;
    series->setVisible(visible);
    for (auto *marker : m_chart->legend()->markers(series)) {
        // Turn legend marker back to visible, since hiding series also hides the marker
        // and we don't want it to happen now.
        marker->setVisible(true);

        // Dim the marker, if series is not visible
        qreal alpha = visible ? 1.0 : 0.5;
        QColor color;
        QBrush brush = marker->labelBrush();
        color = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setLabelBrush(brush);

        brush = marker->brush();
        color = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setBrush(brush);

        QPen pen = marker->pen();
        color = pen.color();
        color.setAlphaF(alpha);
        pen.setColor(color);
        marker->setPen(pen);
    }

    for (QAbstractAxis *axis : m_chart->axes(Qt::Vertical)) {
        bool hideAxis = true;
        for (auto *series : m_chart->series()) {
            for (const auto *attachedAxis : series->attachedAxes()) {
                if (series->isVisible() && attachedAxis == axis) {
                    hideAxis = false;
                    break;
                }
            }
            if (!hideAxis)
                break;
        }
        axis->setVisible(!hideAxis);
    }
}
/*!
 * \brief draw series wider to show which on is hovered
 * \param series
 * \param emphasis
 */
void ZoomableChartView::emphasisSeries(QXYSeries *series, bool emphasis)
{
    if(!series) return;
    auto pen = series->pen();
    pen.setWidth(emphasis ? (pen.width() * 2) : (pen.width() / 2));
    series->setPen(pen);
}
/*!
 * \brief scroll within plot
 * calls chart scroll and update marker
 * \param dx
 * \param dy
 */
void ZoomableChartView::scrollWithinPlot(qreal dx, qreal dy)
{
    m_chart->scroll(dx,dy);
    updateMarker();
}
/*!
 * \brief make callout permanent
 */
void ZoomableChartView::keepCallout()
{
    m_callouts.append(m_tooltip);
    m_tooltip = new Callout(m_chart);
}
/*!
 * \brief show tooltip of point and sweep when hovering over line
 * \param point
 * \param state
 */
void ZoomableChartView::tooltip(QPointF point, bool state)
{
    auto *series=qobject_cast<QXYSeries*>(sender());
    if (m_tooltip == 0)
        m_tooltip = new Callout(m_chart);

    if (state) {
        QString name=series->name();
        m_tooltip->setText(QString("%1\nX: %2 \nY: %3 ").arg(name).arg(point.x()).arg(point.y()));
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
}
