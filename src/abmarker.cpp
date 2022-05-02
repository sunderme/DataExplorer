#include "abmarker.h"
#include "qgraphicssceneevent.h"
#include "qxyseries.h"
#include "qpainter.h"
#include <QCursor>


ABMarker::ABMarker(QGraphicsItem *parent):
    QGraphicsItem(parent),m_series(nullptr),m_chart(nullptr),m_anchorPoint(nullptr)
{
    setFlag(QGraphicsItem::ItemIsMovable,true);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setCursor(Qt::SizeAllCursor);
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

void ABMarker::setSeries(QAbstractSeries *series)
{
    m_series=series;
}
/*!
 * \brief set marker to be A (false) or B (true)
 * \param isB
 */
void ABMarker::setMarkerType(bool isB)
{
    m_isB=isB;
}
/*!
 * \brief return true if this is marker B
 * \return
 */
bool ABMarker::getMarkerType() const
{
    return m_isB;
}

void ABMarker::setAnchor(ABMarker *anchor)
{
    m_anchorPoint=anchor;
}

QRectF ABMarker::boundingRect() const
{
    QRectF rect;
    if(isSelected() || m_lastStateSelected){
        //rect=m_chart->plotArea();
        rect.setRect(-30,-30,180,60);
    }else{
        rect.setRect(-30,-30,60,60);
    }

    return rect;
}

void ABMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    // draw cross
    const qreal w=10;
    const qreal d=2;

    if(isSelected()){
        int textHeight=painter->fontMetrics().height();
        int textWidth=painter->fontMetrics().horizontalAdvance("0000000000000000");
        // get crossing series with line
        painter->save();
        QBrush brush(Qt::SolidPattern);
        brush.setColor(Qt::white); // needs to query style
        painter->setBrush(brush);
        QPointF anchor=QPointF();
        QRectF rect(anchor,anchor);
        rect.setWidth(textWidth);
        rect.setHeight(textHeight);
        rect.translate(QPointF(2,-textHeight-2));
        painter->drawRect(rect);
        painter->drawText(rect,0,QString("%1/%2").arg(m_p.x(),3,'g',3).arg(m_p.y(),3,'g',3));
        // draw cross
        if(m_isB){
            painter->setPen(Qt::red);
        }else{
            painter->setPen(Qt::green);
        }
        painter->drawLine(-w-d,0,-d,0);
        painter->drawLine(0,-w-d,0,-d);
        painter->drawLine(w+d,0,+d,0);
        painter->drawLine(0,+w+d,0,+d);
        painter->restore();
    }else{
        painter->drawLine(-w-d,0,-d,0);
        painter->drawLine(0,-w-d,0,-d);
        painter->drawLine(w+d,0,+d,0);
        painter->drawLine(0,+w+d,0,+d);
    }
    m_lastStateSelected=isSelected();
}

void ABMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF new_pos = event->scenePos();
    movePointOnSeriesChartCoord(new_pos);
    this->setPos( new_pos );
    // update real x values
    if(m_chart)
        m_p=m_chart->mapToValue(new_pos);
}

/*!
 * \brief move pointer onto a series point
 * \param p pointer point
 */
void ABMarker::movePointOnSeriesChartCoord(QPointF &p) const
{
    QXYSeries *series=qobject_cast<QXYSeries*>(m_series);
    if(!series || series->count()==0) return;
    QPointF p0=m_chart->mapToPosition(series->at(0));
    if(series->count()==1){
        p=p0;
        return;
    }
    qreal minimalDistance=sqrt(QPointF::dotProduct(p-p0, p-p0));
    QPointF bestFittingPoint=p0;
    // find closest point on lines between points
    for(int i=1;i<series->count();++i){
        QPointF p1=m_chart->mapToPosition(series->at(i));
        QPointF v=p1-p0;
        qreal lv=sqrt(QPointF::dotProduct(v, v));
        qreal dotp=QPointF::dotProduct(p-p0, v)/lv;
        if(dotp>=0 && dotp<=lv){
            // p vertical between to p0/p1
            qreal lp=QPointF::dotProduct(p-p0, p-p0);
            qreal verticalDistance=sqrt(lp-dotp*dotp);
            if(verticalDistance<minimalDistance){
                bestFittingPoint=p0+v*dotp/lv;
                minimalDistance=verticalDistance;
            }
        }
        // check closesness to point p1
        qreal distance=sqrt(QPointF::dotProduct(p-p1, p-p1));
        if(distance<minimalDistance){
            minimalDistance=distance;
            bestFittingPoint=p1;
        }
        p0=p1;
    }
    p=bestFittingPoint;
}

