#include "horizontalmarker.h"
#include "qgraphicssceneevent.h"
#include <QCursor>


HorizontalMarker::HorizontalMarker(QGraphicsItem *parent):
    QGraphicsLineItem(parent),m_chart(nullptr)
{
    setFlag(QGraphicsItem::ItemIsMovable,true);
    setCursor(Qt::SizeVerCursor);
}

void HorizontalMarker::setYVal(qreal y)
{
    m_yv=y;
}

qreal HorizontalMarker::yVal() const
{
    return m_yv;
}

void HorizontalMarker::setChart(QChart *chart)
{
    m_chart=chart;
}

void HorizontalMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF new_pos = event->scenePos();
    const QPointF old_pos = this->scenePos();
    new_pos.setX( old_pos.x() );
    this->setPos( new_pos );
    // update real x values
    if(m_chart)
        m_yv=m_chart->mapToValue(new_pos).y();
}
