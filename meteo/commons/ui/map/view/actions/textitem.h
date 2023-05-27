#ifndef METEO_COMMONS_UI_TRACEACTION_TEXTITEM_H
#define METEO_COMMONS_UI_TRACEACTION_TEXTITEM_H

#include <QList>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <commons/geobasis/geopoint.h>
#include "traceitem.h"

class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
class Arrow;

namespace meteo {
namespace map {

class MapScene;

class TextItem : public QGraphicsItem
{
public:
  enum { Type = QGraphicsItem::UserType + 55 };

  TextItem(const meteo::GeoPoint& gp, const QString &text,
           QGraphicsItem *parent = 0, MapScene *sc = 0 );
  ~TextItem();
  void setGeoPoint(const GeoPoint& gp) { geoPoint_ = gp; }
  void setText( const QString text );
  void setAngle(double angle);
  meteo::GeoPoint geoPoint() { return geoPoint_; }

  int type() const { return Type; }

  QRectF boundingRect() const ;
  QPainterPath shape() const ;
  QString text() { return text_; }

protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
  meteo::GeoPoint geoPoint_;
  MapScene* scene_;
  double angle_;
  QString text_;

  // служебные
  QRectF textRect_; // рассчитанный размер текста
};

}
}
#endif
