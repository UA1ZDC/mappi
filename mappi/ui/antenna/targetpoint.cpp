#include "targetpoint.h"
#include <qpainter.h>


namespace mappi {

namespace antenna {

TargetPoint::TargetPoint(QGraphicsItem *parent /*=*/) :
    QGraphicsObject(parent)
{
  // setFlag(QGraphicsItem::ItemIgnoresTransformations);
}

TargetPoint::~TargetPoint()
{
}

QRectF TargetPoint::boundingRect() const
{
  return QRectF(-6, -6, 12, 12);
}

void TargetPoint::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* parent /*=*/)
{
  Q_UNUSED(option)
  Q_UNUSED(parent)

  painter->save();

  QPen pen(Qt::darkRed);
  pen.setWidth(2);
  painter->setPen(pen);
  painter->drawLine(-6, -6, 6, 6);
  painter->drawLine(-6, 6, 6, -6);

  painter->restore();
}

}

}
