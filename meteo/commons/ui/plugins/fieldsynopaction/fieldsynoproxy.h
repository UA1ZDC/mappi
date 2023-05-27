#ifndef FIELDSYNOPROXY_H
#define FIELDSYNOPROXY_H

#include <QtGui>
#include <commons/geobasis/geopoint.h>
#include "fieldsynopwidg.h"


namespace meteo {
namespace map {


class FieldSynoProxy : public QGraphicsProxyWidget
{
  Q_OBJECT
public:
  enum State{
    Moving = 0,
    Fixed  = 1
  };
  FieldSynoProxy(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
  State state() const;
  void setState(State state);
  GeoPoint gp() const;
  void setGeoPoint(GeoPoint gp);
  QPoint clickPos() const;

protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
  State state_;
  GeoPoint gp_;
  bool press_;
  QPoint click_;

};


}
}

#endif // FIELDSYNOPROXY_H
