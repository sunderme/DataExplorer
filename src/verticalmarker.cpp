#include "verticalmarker.h"
#include "qgraphicssceneevent.h"
#include <QCursor>


VerticalMarker::VerticalMarker(QGraphicsItem *parent):
    QGraphicsLineItem(parent)
{
    setFlag(QGraphicsItem::ItemIsMovable,true);
    setCursor(Qt::SizeHorCursor);
}

void VerticalMarker::setXVal(qreal x)
{
    m_xv=x;
}

qreal VerticalMarker::xVal() const
{
    return m_xv;
}

void VerticalMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF new_pos = event->scenePos();
    const QPointF old_pos = this->scenePos();
    new_pos.setY( old_pos.y() );
    this->setPos( new_pos );
}
