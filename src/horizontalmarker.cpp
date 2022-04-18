#include "horizontalmarker.h"
#include "qgraphicssceneevent.h"
#include <QCursor>


HorizontalMarker::HorizontalMarker(QGraphicsItem *parent):
    QGraphicsLineItem(parent)
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

void HorizontalMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF new_pos = event->scenePos();
    const QPointF old_pos = this->scenePos();
    new_pos.setX( old_pos.x() );
    this->setPos( new_pos );
}
