#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PIXMAPDRAWOBJECT_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PIXMAPDRAWOBJECT_H

#include "drawobject.h"
#include <meteo/commons/ui/map/geopixmap.h>

namespace meteo {
class MarkerItem;
} // meteo

namespace meteo {
namespace map {

class GeoPixmap;
class GeoObjectItem;

//!
class PixmapDrawObject : public DrawObject
{
public:
  //! Конструирует объект, с идентификатором id, для редактирования гео-текста на сцене scene.
  explicit PixmapDrawObject(MapScene* mapScene, const QString& id = QString());
  //!
  virtual ~PixmapDrawObject();

  //! Добавляет новый узел к концу каркаса, на основе экранной координаты
  virtual void appendNode(const QPoint& point, int beforeIdx = -1);
  //! Добавляет новый узел к концу каркаса, на основе географической координаты
  virtual void appendNode(const GeoPoint& point, int beforeIdx = -1);
  //! Пересчёт координат узлов сцены на основе географических координат
  virtual void calcSceneCoords();
  //! Обновляет объекты размещённые на сцене (каркас, маркеры узлов, ...) и объект на слое
  virtual void update();
  //! Данный метод вызывается маркерами узлов для уведомления об изменении своего состояния
  virtual void markerChanged(MarkerItem* marker);

  //! Возвращает гео-объект или 0, если объект не установлен.
  virtual const Object* object() const { return geoPixmap_; }
  //! Устанавливает значение узлов каркаса гео-объекта в координатах сцены.
  virtual void setObject(const Object* obj);

  //! Устанавливает значение узлов каркаса гео-объекта в координатах сцены.
  virtual void setObjectSkelet(const QPolygon& skelet);
  //! Устанавливает значение узлов каркаса гео-объекта в географических координатах.
  virtual void setObjectSkelet(const GeoVector& skelet);
  //! Устанавливает свойства гео-объекта.
  virtual void setObjectProperty(const meteo::Property& prop);
  //! Устанавливает текст гео-объекта.
  virtual void setObjectPixmap(const QImage& text);

  //! Возвращает каркас гео-объекта.
  virtual GeoVector objectSkelet() const;
  //! Возвращает свойства гео-объекта.
  virtual meteo::Property objectProperty() const;
  //!
  virtual QImage objectPixmap() const;

  //! Данный метод обрабатывает события мыши поступающие от объекта сцены
  virtual void mouseMoveEvent(QMouseEvent* e);
  //! Данный метод обрабатывает события мыши поступающие от объекта сцены
  virtual void mouseReleaseEvent(QMouseEvent* e);

  virtual void removeFromScene();

private:
  MarkerItem* createNode();

private:
  GeoPixmap* geoPixmap_;
  MarkerItem* nodeItem_;
  GeoObjectItem* objItem_;
};

} // map
} // meteo

#endif
