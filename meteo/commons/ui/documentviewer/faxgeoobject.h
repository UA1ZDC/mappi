#ifndef METEO_COMMONS_UI_MAP_ACTIONS_FAX_FAXGEOOBJECT_H
#define METEO_COMMONS_UI_MAP_ACTIONS_FAX_FAXGEOOBJECT_H

#include <qgraphicsitem.h>
#include <meteo/commons/ui/map/object.h>

namespace meteo {
namespace map {

//!
class FaxGeoObject : public QGraphicsItem
{
public:
  //! Конструирует пустой объект. Установить гео-объект для отрисовки можно с помощью setObject().
  explicit FaxGeoObject(QGraphicsScene* scene = 0, QGraphicsItem* parent = 0);
  //! Конструирует графический элемент сцены для гео-объекта object. При удалении экземпляра FaxGeoObject
  //! object не удаляется.
  explicit FaxGeoObject(Object* object, QGraphicsScene* scene, QGraphicsItem* parent = 0);
  //!
  virtual ~FaxGeoObject();

  //!
  virtual QRectF boundingRect() const;
  //!
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  //! Устанавливает гео-объект, для которого будет производится отрисовка на сцене
  void setObject(Object* obj);
  //! Возвращает текущий гео-объект или 0, если объект не установлен.
  Object* object() const { return object_; }

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
  void setPixmap(const QPixmap& pixmap);

  void setCoord(int a, const GeoPoint &coor);
  void setPixmapPoints(int num, QPoint pnt);



private:
  void calcBoundingRect();

private:
  Object* object_;
  QRectF boundingRect_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_ACTIONS_FAX_FAXGEOOBJECT_H
