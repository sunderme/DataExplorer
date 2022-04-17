#include "zoomablechartview.h"
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGlobal>

#include "rangelimitedvalueaxis.h"

#include <QDebug>

ZoomableChartView::ZoomableChartView(QWidget *parent) :
    QGraphicsView(new QGraphicsScene, parent),m_chart(nullptr)
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

void ZoomableChartView::setChart(QChart *chart)
{
    m_chart=chart;
    scene()->addItem(m_chart);
}

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
                chart()->scroll(dx, 0);
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
                chart()->scroll(0, dy);
            }
        }
        m_lastMousePos = event->pos();
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ZoomableChartView::wheelEvent(QWheelEvent *event)
{
    bool zoomHorizontalAxis = false;
    for (auto axis : chart()->axes()) {
        if (axis->orientation() == Qt::Horizontal && isAxisTypeZoomableWithMouse(axis->type())) {
            zoomHorizontalAxis = true;
            break;
        }
    }

    if (QGuiApplication::keyboardModifiers() & Qt::KeyboardModifier::ControlModifier)
        zoomHorizontalAxis = !zoomHorizontalAxis;

    if (zoomHorizontalAxis) {
        if (event->angleDelta().y() > 0) {
            zoomX(2, event->position().x() - chart()->plotArea().x());
        } else if (event->angleDelta().y() < 0) {
            zoomX(0.5, event->position().x() - chart()->plotArea().x());
        }
    } else {
        if (event->angleDelta().y() > 0) {
            zoomY(2, event->position().y() - chart()->plotArea().y());
        } else if (event->angleDelta().y() < 0) {
            zoomY(0.5, event->position().y() - chart()->plotArea().y());
        }
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
        //setRubberBand(QChartView::HorizontalRubberBand);
        setDragMode(QChartView::NoDrag);
        break;
    case VerticalZoom:
        //setRubberBand(QChartView::VerticalRubberBand);
        setDragMode(QChartView::NoDrag);
        break;
    }
}
/*!
 * \brief add vertical marker at value x
 * \param x
 */
void ZoomableChartView::addVerticalMarker(qreal x)
{
    QGraphicsLineItem *lineItem=new QGraphicsLineItem(x,0.,x,5.);
    QGraphicsScene *scene=chart()->scene();
    scene->addItem(lineItem);
}

void ZoomableChartView::zoomX(qreal factor, qreal xcenter)
{
    QRectF rect = chart()->plotArea();
    qreal widthOriginal = rect.width();
    rect.setWidth(widthOriginal / factor);
    qreal centerScale = (xcenter / widthOriginal);

    qreal leftOffset = (xcenter - (rect.width() * centerScale) );

    rect.moveLeft(rect.x() + leftOffset);
    chart()->zoomIn(rect);
}

void ZoomableChartView::zoomX(qreal factor)
{
    QRectF rect = chart()->plotArea();
    qreal widthOriginal = rect.width();
    QPointF center_orig = rect.center();

    rect.setWidth(widthOriginal / factor);

    rect.moveCenter(center_orig);

    chart()->zoomIn(rect);
}

void ZoomableChartView::zoomY(qreal factor, qreal ycenter)
{
    QRectF rect = chart()->plotArea();
    qreal heightOriginal = rect.height();
    rect.setHeight(heightOriginal / factor);
    qreal centerScale = (ycenter / heightOriginal);

    qreal topOffset = (ycenter - (rect.height() * centerScale) );

    rect.moveTop(rect.x() + topOffset);
    chart()->zoomIn(rect);
}
/*!
 * \brief zoom chart in y direction
 * \param factor
 */
void ZoomableChartView::zoomY(qreal factor)
{
    QRectF rect = chart()->plotArea();
    qreal heightOriginal = rect.height();
    QPointF center_orig = rect.center();

    rect.setHeight(heightOriginal / factor);

    rect.moveCenter(center_orig);

    chart()->zoomIn(rect);
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
    QObject::connect(series, &QLineSeries::clicked,this, &ZoomableChartView::seriesClicked);
    QObject::connect(series, &QLineSeries::hovered,this, &ZoomableChartView::seriesHovered);
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
    if(m_zoomMode==RectangleZoom){
#if  QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_lastMousePos = event->position();
#else
        m_lastMousePos = event->localPos();
#endif
        QRectF rect(m_startMousePos,m_lastMousePos);
        chart()->zoomIn(rect.normalized());
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
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
        //![1]
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
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
