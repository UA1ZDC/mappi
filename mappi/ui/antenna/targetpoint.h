#pragma once

#include <QGraphicsObject>


namespace mappi {

namespace antenna {

/** Целеуказатель - графический элемент обозначающий конечную точку в которой должна оказаться антенна. */
class TargetPoint :
  public QGraphicsObject
{
public :
  TargetPoint(QGraphicsItem* parent = nullptr);
  virtual ~TargetPoint();

  virtual QRectF boundingRect() const override;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* parent = nullptr) override;
};


}

}
