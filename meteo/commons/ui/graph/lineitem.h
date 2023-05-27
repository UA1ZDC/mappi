#ifndef METEO_COMMONS_UI_GRAPH_GRAPHVALUELINEITEM_H
#define METEO_COMMONS_UI_GRAPH_GRAPHVALUELINEITEM_H

#include <qgraphicsitem.h>
#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {
class Document;
class LayerItems;
} // map
} // meteo

namespace meteo {
namespace graph {

class LineItem : public QGraphicsItem
{
public:
  enum { Type = UserType + 27 };
  enum LineType { kGraph, kOcean };

  int type() const { return Type; }

  LineItem(map::LayerItems* layer = nullptr, QGraphicsItem* parent = nullptr, QGraphicsScene* scene = nullptr, LineType type = kGraph);
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
  LineType type_;
};


} // graph
} // meteo

Q_DECLARE_METATYPE( ::meteo::graph::LineItem* )

#endif // METEO_COMMONS_UI_GRAPH_GRAPHVALUELINEITEM_H
