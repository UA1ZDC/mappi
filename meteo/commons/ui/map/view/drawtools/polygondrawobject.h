#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_POLYGONDRAWOBJECT_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_POLYGONDRAWOBJECT_H

#include "drawobject.h"
#include "groupobject.h"

#include <qobject.h>
#include <qaction.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/object.h>

namespace meteo {
class MarkerItem;
} // meteo

namespace meteo {
namespace map {

class MapScene;
class GeoObjectItem;
class GraphicsGroupItem;

//! Класс PolygonDrawObject предоставляет интерфейс для редактирования объекта типа GeoPolygon
class PolygonDrawObject : public QObject, public DrawObject
{
  Q_OBJECT
public:
  //! Конструирует объект, с идентификатором id, для редактирования гео-полигона на сцене scene.
  PolygonDrawObject(MapScene* mapScene, const QString& id = QString());
  //!
  virtual ~PolygonDrawObject();

  //! Устанавливает гео-объект для редактиравания, при этом создаётся копия объекта obj.
  virtual void setObject(const Object* obj);
  //! Возвращает гео-объект или 0, если объект не установлен.
  virtual const Object* object() const { return object_; }

  //! Пересчёт координат узлов сцены на основе географических координат
  virtual void calcSceneCoords();

  //! Добавляет новый узел к концу каркаса, на основе экранной координаты
  virtual void appendNode(const QPoint& point, int beforeIdx = -1, bool underMouseCheck = true);
  //! Добавляет новый узел к концу каркаса, на основе географической координаты
  virtual void appendNode(const GeoPoint& point, int beforeIdx = -1);
  //! Вставляет узел с координатой point после узла с координатой after
  virtual void insertNode(const QPoint& point, const QPoint& after);
  //! Вставляет узел с координатой point после узла с координатой after
  virtual void insertNode(const GeoPoint& point, const GeoPoint& after);
  //! Удаляет узел с заданной координатой
  virtual void removeNode(const QPoint& point);
  //! Удаляет узел с заданной координатой
  virtual void removeNode(const GeoPoint& point);

  //! Обновляет объекты размещённые на сцене (каркас, маркеры узлов, ...) и объект на слое
  virtual void update();

  //! Добавляет в контекстное меню список операций доступных для выполнения над объектом
  virtual QList<QAction*> setupContextMenu(Menu* menu, const QPoint& screenPos);
  //! Обрабатывает результат команду выбранную в контекстном меню
  virtual void processContextMenu(QAction* act);

  //! Включает или отключает возможность передвижения обекта в соответствии со значением enabled
  virtual void setMovable(bool enable);
  //! Возвращает true, если объект являеся перемещаемым
  virtual bool isMovable() const;

  //! Данный метод обрабатывает события мыши поступающие от объекта сцены
  virtual void mouseMoveEvent(QMouseEvent* e);

  //! Данный метод вызывается маркерами узлов для уведомления об изменении своего состояния
  virtual void markerChanged(MarkerItem* marker);
  //! Возвращает секцию каркаса, на которой располгается scenePoint или null line, если таких
  //! секций не найдено.
  //! \arg[width] Ширина линии секции для пересечения с точкой scenePoint
  virtual QLine skeletSectionAt(const QPoint& scenePoint, int width) const;
  //! Возвращает маркеры узлов, которые включают scenePoint
  QList<MarkerItem*> skeletMarkersAt(const QPoint& scenePoint) const;
  //!Возвращает последний маркер
  MarkerItem* lastMarker() const;
  //!Возвращает первый маркер
  MarkerItem* firstMarker() const;
  //! Возвращает количество маркеров
  int markersCount() const;
  //! Устанавливает значение узлов каркаса гео-объекта в координатах сцены.
  virtual void setObjectSkelet(const QPolygon& skelet);
  //! Устанавливает значение узлов каркаса гео-объекта в географических координатах.
  virtual void setObjectSkelet(const GeoVector& skelet);
  //! Устанавливает свойства гео-объекта.
  virtual void setObjectProperty(const meteo::Property& prop);
  //! Устанавливает значение гео-объекта.
  virtual void setObjectValue(double value, const QString& format = "4' '.2'0'", const QString& unit = QString());

  //! Удаляет значение гео-объекта.
  virtual void removeObjectValue();

  //! Возвращает каркас гео-объекта.
  virtual GeoVector objectSkelet() const;
  //! Возвращает свойства гео-объекта.
  virtual meteo::Property objectProperty() const;
  //! Возвращает значение гео-объекта.
  virtual double objectValue(bool* hasValue = nullptr) const;
  //! Возвращает формат занчения гео-объекта.
  virtual QString objectValueFormat() const;
  //! Возвращает единицы измерения гео-объекта.
  virtual QString objectValueUnit() const;

  //!
  virtual void removeFromScene();

protected:
  QList<MarkerItem*> markers() { return markers_; }

private:
  void deleteMarkers();
  void uprost(GeoVector *gv);
  void uprost2(GeoVector *gv);

private slots:
  void slotChangeMarkerPos( QPointF delta);
  void slotSkeletPos();

private:
  GeoPolygon* object_ = nullptr;
  QGraphicsPathItem* skeletItem_ = nullptr;
  GeoObjectItem* objItem_ = nullptr;
  QList<MarkerItem*> markers_;
  QAction* addAct_ = nullptr;
  QAction* rmAct_ = nullptr;
  GroupObject* grObject_ = nullptr;
  bool movable_ = false;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_POLYGONDRAWOBJECT_H
