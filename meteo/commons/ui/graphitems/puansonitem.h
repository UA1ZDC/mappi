#ifndef METEO_COMMONS_UI_GRAPHITEMS_PUANSONITEM_H
#define METEO_COMMONS_UI_GRAPHITEMS_PUANSONITEM_H

#include <qpen.h>
#include <qmargins.h>
#include <qgraphicsitem.h>
#include <qvector.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>

class TMeteoData;

namespace meteo {
namespace map {

class Document;
class Layer;

} // map
} // meteo

namespace meteo {
namespace graph {

class PuansonItem : public QGraphicsItem
{
public:
  enum { Type = UserType + 32 };
  int type() const { return Type; }

  PuansonItem(QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
  virtual ~PuansonItem();

  //! Возвращает географическую координату элемента
  GeoPoint geoPos() const;
  //! Устанавливает значение географической координаты
  void setGeoPos(const GeoPoint& pos);
  //! Устанавливает puanson в качестве шаблона для отображения
  void setPuanson(const puanson::proto::Puanson& puanson);
  puanson::proto::Puanson puanson() const             { return puanson_; }

  void setBorderStyle(const QColor& color, int width);
  int borderWidth() const                             { return borderWidth_; }
  void setBackgroundStyle(const QColor& color, Qt::BrushStyle style);

  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);

private:
  map::Document* document() const;
  map::LayerProfile* layer() const;

private:
  GeoPoint geoPos_;
  meteo::puanson::proto::Puanson puanson_;

  QColor borderColor_;
  int    borderWidth_ = 0;
  QColor bgColor_;
  Qt::BrushStyle bgStyle_ = Qt::NoBrush;
};


} // graph
} // meteo

Q_DECLARE_METATYPE( ::meteo::graph::PuansonItem* )

#endif // METEO_COMMONS_UI_GRAPH_LABELITEM_H
