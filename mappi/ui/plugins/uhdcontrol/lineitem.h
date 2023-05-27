#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_LINEITEM_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_LINEITEM_H

#include <qgraphicsitem.h>
#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {
class Document;
class LayerItems;
} // map
} // meteo

namespace meteo {
namespace spectr {

class LineItem : public QGraphicsItem
{
public:
  enum { Type = UserType + 27 };
  int type() const { return Type; }

  LineItem(map::LayerItems* layer = 0, QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
  virtual ~LineItem();

  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  GeoPoint geoPos() const               { return geoPos_; }
  void setGeoPos(const GeoPoint& pos);

  //! Возвращает экранную координату, рассчитанную по георафической.
  QPoint calcScenePoint() const;

  void setLayer(map::LayerItems* layer) { layer_ = layer; }

protected:
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
  map::Document* document() const;

private:
  GeoPoint geoPos_;

  map::LayerItems* layer_;
};


} // spectr
} // meteo

Q_DECLARE_METATYPE( ::meteo::spectr::LineItem* )

#endif // MAPPI_UI_PLUGINS_UHDCONTROL_LINEITEM_H
