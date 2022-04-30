#include "abmarker.h"
#include "qgraphicssceneevent.h"
#include "qxyseries.h"
#include "qpainter.h"
#include <QCursor>


ABMarker::ABMarker(QGraphicsItem *parent):
    QGraphicsLineItem(parent),m_chart(nullptr)
{
    setFlag(QGraphicsItem::ItemIsMovable,true);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setCursor(Qt::SizeVerCursor);
}

void ABMarker::setVal(QPointF p)
{
    m_p=p;
}

QPointF ABMarker::val() const
{
    return m_p;
}

void ABMarker::setChart(QChart *chart)
{
    m_chart=chart;
}

QRectF ABMarker::boundingRect() const
{
    QLineF l=line();
    QRectF rect;
    qreal width=pen().widthF()/2;
    if(isSelected() || m_lastStateSelected){
        rect.setLeft(l.x1()-60);
        rect.setRight(l.x2()+60);
        rect.setTop(l.y1()-width-20);
        rect.setBottom(l.y1()+width+20);
    }else{
        rect.setLeft(l.x1());
        rect.setRight(l.x2());
        rect.setTop(l.y1()-width);
        rect.setBottom(l.y1()+width);
    }

    return rect;
}

void ABMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->drawLine(line());
    if(isSelected()){
        int textHeight=painter->fontMetrics().height();
        int textWidth=painter->fontMetrics().horizontalAdvance("0000000");
        // get crossing series with line
        painter->save();
        QBrush brush(Qt::SolidPattern);
        brush.setColor(Qt::white); // needs to query style
        painter->setBrush(brush);
        QPointF anchor=line().p2();
        QRectF rect(anchor,anchor);
        rect.setWidth(textWidth);
        rect.setHeight(textHeight);
        rect.translate(QPointF(2,-textHeight/2));
        painter->drawRect(rect);
        painter->drawText(rect,0,QString("%1/%2").arg(m_p.x(),6).arg(m_p.y(),6));

        painter->restore();
    }
    m_lastStateSelected=isSelected();
}

void ABMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF new_pos = event->scenePos();
    const QPointF old_pos = this->scenePos();
    new_pos.setX( old_pos.x() );
    this->setPos( new_pos );
    // update real x values
    if(m_chart)
        m_p=m_chart->mapToValue(new_pos);
}

