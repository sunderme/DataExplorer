#include "horizontalmarker.h"
#include "qgraphicssceneevent.h"
#include "qxyseries.h"
#include "qpainter.h"
#include <QCursor>


HorizontalMarker::HorizontalMarker(QGraphicsItem *parent):
    QGraphicsLineItem(parent),m_chart(nullptr)
{
    setFlag(QGraphicsItem::ItemIsMovable,true);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
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

QRectF HorizontalMarker::boundingRect() const
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

void HorizontalMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
        QList<qreal> result=intersectingPoints();
        qreal last_xt,last_xb;
        qreal last_xs;
        bool firstLoop=true;
        for(const qreal x:result){
            QPointF anchor=m_chart->mapToPosition(QPointF(x,m_yv));
            anchor=mapFromParent(anchor);
            QRectF rect(anchor,anchor);
            rect.setWidth(textWidth);
            rect.setHeight(textHeight);
            rect.translate(QPointF(-textWidth/2,-textHeight-2));
            if(!firstLoop){
                if(qFuzzyCompare(x,last_xs))
                    continue; // don't draw same marker twice (for different series ot looped series)
                if(anchor.x()-last_xt<textWidth){
                    // move to other side
                    if(anchor.x()-last_xb<textWidth){
                        // already text shown, give up
                        continue;
                    }
                    rect.translate(QPointF(0,textHeight+4));
                    last_xb=anchor.x();
                }else{
                    last_xt=anchor.x();
                }
            }else{
                last_xt=anchor.x();
                last_xb=last_xt-100;
            }
            painter->drawRect(rect);
            painter->drawText(rect,0,QString("%1").arg(x,6));

            firstLoop=false;
            last_xs=x;
        }
        // draw current y value
        QPointF anchor=line().p2();
        QRectF rect(anchor,anchor);
        rect.setWidth(textWidth);
        rect.setHeight(textHeight);
        rect.translate(QPointF(2,-textHeight/2));
        painter->drawRect(rect);
        painter->drawText(rect,0,QString("%1").arg(m_yv,6));

        painter->restore();
    }
    m_lastStateSelected=isSelected();
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

QList<HorizontalMarker::Intersection> HorizontalMarker::intersectingPointsWithSeries()
{
    QList<HorizontalMarker::Intersection> result;
    QLineF l=line();
    QLineF horizontalLine(m_chart->mapToValue(mapToParent(l.p1())),m_chart->mapToValue(mapToParent(l.p2())));
    for(QAbstractSeries *series:m_chart->series()){
        if(!series->isVisible()) continue;
        QList<qreal> xs;
        QXYSeries *s=qobject_cast<QXYSeries *>(series);
        if(s->count()==0) continue;
        QPointF p0=s->at(0);
        if(s->count()==1 && p0.y()==m_yv) {
            xs<<p0.x();
            continue;
        }
        for(int i=1;i<s->count();++i){
            QPointF p1=s->at(i);
            QLineF line(p0,p1);
            QPointF p;
            auto res=line.intersects(horizontalLine,&p);
            if(res == QLineF::BoundedIntersection){
                xs<<p.x();
            }
            p0=p1;
        }
        if(!xs.isEmpty()){
            std::sort(xs.begin(),xs.end());
            auto it=std::unique(xs.begin(),xs.end());
            xs.resize( std::distance(xs.begin(),it) );
            result<<Intersection{series,xs};
        }
    }
    return result;
}

QList<qreal> HorizontalMarker::intersectingPoints()
{
    QList<qreal> result;
    QLineF l=line();
    QLineF horizontalLine(m_chart->mapToValue(mapToParent(l.p1())),m_chart->mapToValue(mapToParent(l.p2())));
    for(QAbstractSeries *series:m_chart->series()){
        if(!series->isVisible()) continue;
        QList<qreal> xs;
        QXYSeries *s=qobject_cast<QXYSeries *>(series);
        if(s->count()==0) continue;
        QPointF p0=s->at(0);
        if(s->count()==1 && p0.y()==m_yv) {
            xs<<p0.x();
            continue;
        }
        for(int i=1;i<s->count();++i){
            QPointF p1=s->at(i);
            QLineF line(p0,p1);
            QPointF p;
            auto res=line.intersects(horizontalLine,&p);
            if(res == QLineF::BoundedIntersection){
                xs<<p.x();
            }
            p0=p1;
        }
        if(!xs.isEmpty()){
            result<<xs;
        }
    }
    std::sort(result.begin(),result.end());
    auto it=std::unique(result.begin(),result.end());
    result.resize( std::distance(result.begin(),it) );
    return result;
}

