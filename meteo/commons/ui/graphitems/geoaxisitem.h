#ifndef METEO_COMMONS_UI_GRAPHITEMS_GEOAXISITEM_H
#define METEO_COMMONS_UI_GRAPHITEMS_GEOAXISITEM_H

#include <qpen.h>
#include <qmargins.h>
#include <qgraphicsitem.h>
#include <qvector.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/profile/axisprofile.h>
#include <meteo/commons/proto/puanson.pb.h>

class TMeteoData;

namespace meteo {
namespace map {

class Document;
class Layer;

} // map
} // meteo

namespace meteo {
namespace graph {

class GeoAxisItem : public QGraphicsItem
{
public:
  enum { Type = UserType + 31 };
  int type() const { return Type; }

  GeoAxisItem(QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
  virtual ~GeoAxisItem();

  //! Возвращает географическую координату элемента
  GeoPoint geoPos() const;
  //! Устанавливает значение географической координаты
  void setGeoPos(const GeoPoint& pos);

  //! Устанавливает длину оси (в проекции карты)
  void setLength(double length);
  //! Устанавливает размеренность шкалы от lower до upper
  void setRange(double lower, double upper);
  //! Устанавливает puanson в качестве шаблона для отображения подписей для насечек
  void setPuanson(const puanson::proto::Puanson& puanson);
  //! Устанавливает вектор значений ticks в качестве отображаемых насечек для уровня генерализации level.
  void setTickVector(const QVector<float> &ticks, int level);

  void setAxisType(map::AxisSimple::AxisType type);

  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);

private:
  map::Document* document() const;

private:
  map::AxisProfile* axis_ ;
};


} // graph
} // meteo

Q_DECLARE_METATYPE( ::meteo::graph::GeoAxisItem* )

#endif // METEO_COMMONS_UI_GRAPH_LABELITEM_H
