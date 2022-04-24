#include "verticalmarker.h"
#include "qgraphicssceneevent.h"
#include "qpainter.h"
#include "qxyseries.h"
#include <QCursor>


VerticalMarker::VerticalMarker(QGraphicsItem *parent):
    QGraphicsLineItem(parent),m_chart(nullptr),m_lastStateSelected(false)
{
    setFlag(QGraphicsItem::ItemIsMovable,true);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
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

void VerticalMarker::setChart(QChart *chart)
{
    m_chart=chart;
}

QRectF VerticalMarker::boundingRect() const
{
    QLineF l=line();
    QRectF rect;
    qreal width=pen().widthF()/2;
    if(isSelected() || m_lastStateSelected){
        rect.setLeft(l.x1()-width);
        rect.setRight(l.x1()+width+60);
        rect.setTop(qMin(l.y1(), l.y2()));
        rect.setBottom(qMax(l.y1(), l.y2()));
    }else{
        rect.setLeft(l.x1()-width);
        rect.setRight(l.x1()+width);
        rect.setTop(qMin(l.y1(), l.y2()));
        rect.setBottom(qMax(l.y1(), l.y2()));
    }

    return rect;
}

void VerticalMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawLine(line());
    if(isSelected()){
        int textHeight=painter->fontMetrics().height();
        int textWidth=painter->fontMetrics().horizontalAdvance("0000000");
        // get crossing series with line
        painter->save();
        QBrush brush(Qt::SolidPattern);
        brush.setColor(Qt::white); // needs to query style
        painter->setBrush(brush);
        QList<VerticalMarker::Intersection> result=intersectingPoints();
        for(const auto &elem:result){
            for(const qreal y:elem.ys){
                QPointF anchor=m_chart->mapToPosition(QPointF(m_xv,y));
                anchor=mapFromParent(anchor);
                QRectF rect(anchor,anchor);
                rect.setWidth(textWidth);
                rect.setHeight(textHeight);
                rect.translate(QPointF(2,-textHeight/2));
                painter->drawRect(rect);
                painter->drawText(rect,0,QString("%1").arg(y,6));

            }
        }
        painter->restore();
    }
    m_lastStateSelected=isSelected();
}

void VerticalMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF new_pos = event->scenePos();
    const QPointF old_pos = this->scenePos();
    new_pos.setY( old_pos.y() );
    this->setPos( new_pos );
    // update real x values
    if(m_chart)
        m_xv=m_chart->mapToValue(new_pos).x();
}

QList<VerticalMarker::Intersection> VerticalMarker::intersectingPoints()
{
    QList<VerticalMarker::Intersection> result;
    QLineF l=line();
    QLineF verticalLine(m_chart->mapToValue(mapToParent(l.p1())),m_chart->mapToValue(mapToParent(l.p2())));
    for(QAbstractSeries *series:m_chart->series()){
        QList<qreal> ys;
        QXYSeries *s=qobject_cast<QXYSeries *>(series);
        if(s->count()==0) continue;
        QPointF p0=s->at(0);
        if(s->count()==1 && p0.x()==m_xv) {
            ys<<p0.y();
            continue;
        }
        for(int i=1;i<s->count();++i){
            QPointF p1=s->at(i);
            QLineF line(p0,p1);
            QPointF p;
            auto res=line.intersects(verticalLine,&p);
            if(res == QLineF::BoundedIntersection){
                ys<<p.y();
            }
            p0=p1;
        }
        if(!ys.isEmpty()){
            result<<Intersection{series,ys};
        }
    }
    return result;
}
