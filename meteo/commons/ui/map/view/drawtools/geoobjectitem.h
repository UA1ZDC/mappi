#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOOBJECTITEM_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOOBJECTITEM_H

#include <qgraphicsitem.h>
#include <meteo/commons/ui/map/object.h>

namespace meteo {
namespace map {

//!
class GeoObjectItem : public QGraphicsItem
{
public:
  //! Коструирует пустой объект. Установить гео-объект для отрисовки можно с помощью setObject().
  explicit GeoObjectItem(QGraphicsScene* scene = 0, QGraphicsItem* parent = 0);
  //! Коструирует графический элемент сцены для гео-объекта object. При удаления экземпляра GeoObjectItem
  //! object не удаляется.
  explicit GeoObjectItem(Object* object, QGraphicsScene* scene, QGraphicsItem* parent = 0);
  //!
  virtual ~GeoObjectItem();

  //!
  virtual QRectF boundingRect() const;
  //!
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  //! Устанавливает гео-объект, для которого будет производится отрисовка на сцене
  void setObject(Object* obj);
  //! Возвращает текущий гео-объект или 0, если объект не установлен.
  Object* object() const { return object_; }

  void setHighlight(bool enable) { highlighted_ = enable; }

  //!
  void setSkelet(const GeoVector& skelet);
  //!
  void setProperty(const meteo::Property& prop);
  //!
  void setValue(double value, const QString& format, const QString& unit);
  //!
  void removeValue();
  //!
  void setText(const QString& text);
  //!
  void setPixmap(const QImage& pixmap);

private:
  void calcBoundingRect();

private:
  Object* object_;
  QRectF boundingRect_;
  bool highlighted_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOOBJECTITEM_H
