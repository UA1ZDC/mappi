#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWOBJECT_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWOBJECT_H

#include <qpolygon.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/graphitems/markeritem.h>

class QMenu;
class QPoint;
class QAction;
class QMouseEvent;

namespace meteo {

class GeoPoint;

namespace map {

class Menu;
class Layer;
class Object;
class MapScene;
class DrawObject;

class DrawObjectClosure : public MarkerItemClosure
{
public:
  DrawObjectClosure(DrawObject* object) : object_(object) {}
  virtual ~DrawObjectClosure(){}

  virtual void run(MarkerItem* item);

private:
  DrawObject* object_;

  Q_DISABLE_COPY(DrawObjectClosure)
};

//! Класс DrawObject предоставляет базовый интерфейс для реализации редактирования гео-объектов.
class DrawObject
{
public:
  //! Конструирует объект, с идентификатором id, для редактирования гео-объекта на сцене scene.
  explicit DrawObject(MapScene* scene, const QString& id = QString());
  //!
  virtual ~DrawObject();

  //! Возвращает идентификатор объекта.
  QString id() const { return id_; }
  //! Возвращает указатель на сцену, на которой располежен объект.
  MapScene* scene() const { return scene_; }

  void setHighlightEnable(bool enable);
  bool isHighlighted() const { return highlighted_; }

  //! Добавляет новый узел к концу каркаса, на основе экранной координаты
  virtual void appendNode(const QPoint& point, int beforeIdx = -1);
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
  virtual void update() {}
  //! Данный метод вызывается маркерами узлов для уведомления об изменении своего состояния
  virtual void markerChanged(MarkerItem* marker);
  //! Пересчёт координат узлов сцены на основе географических координат
  virtual void calcSceneCoords();
  //! Пересчёт географических координат на основе координат узлов сцены
  virtual void calcGeoCoords() {}

  //! Возвращает гео-объект или 0, если объект не установлен.
  virtual const Object* object() const { return nullptr; }
  //! Устанавливает значение узлов каркаса гео-объекта в координатах сцены.
  virtual void setObject(const Object* object);
  //! Устанавливает значение узлов каркаса гео-объекта в координатах сцены.
  virtual void setObjectSkelet(const QPolygon& skelet);
  //! Устанавливает значение узлов каркаса гео-объекта в географических координатах.
  virtual void setObjectSkelet(const GeoVector& skelet);
  //! Устанавливает свойства гео-объекта.
  virtual void setObjectProperty(const meteo::Property& prop);
  //! Устанавливает значение гео-объекта.
  virtual void setObjectValue(double value, const QString& format = "4' '.2'0'", const QString& unit = QString());
  //! Устанавливает текст гео-объекта.
  virtual void setObjectText(const QString& text);
  //! Устанавливает pixmap  гео-объекта.
  virtual void setObjectPixmap(const QImage& pixmap);

  //! Удаляет значение гео-объекта.
  virtual void removeObjectValue() {}

  //! Возвращает каркас гео-объекта.
  virtual GeoVector objectSkelet() const { return GeoVector(); }
  //! Возвращает свойства гео-объекта.
  virtual meteo::Property objectProperty() const { return meteo::Property(); }
  //! Возвращает значение гео-объекта. Если для гео-объекта установлено значение hasValue будет установлено в true.
  virtual double objectValue(bool* hasValue = nullptr) const;
  //! Возвращает формат занчения гео-объекта.
  virtual QString objectValueFormat() const { return QString(); }
  //! Возвращает единицы измерения гео-объекта.
  virtual QString objectValueUnit() const { return QString(); }
  //! Возвращает текст гео-объекта
  virtual QString objectText() const { return QString(); }
  //!
  virtual QImage objectPixmap() const { return QImage(); }

  //! Возвращает true, если объект являеся перемещаемым.
  virtual bool isMovable() const { return false; }
  //! Включает или отключает возможность передвижения обекта в соответствии со значением enabled.
  virtual void setMovable(bool enable);

  //! Возвращает секцию каркаса, на которой располгается scenePoint или null line, если таких
  //! секций не найдено.
  //! \arg[width] Ширина линии секции для пересечения с точкой scenePoint
  virtual QLine skeletSectionAt(const QPoint& scenePoint, int width) const;

  //! Добавляет в контекстное меню список операций доступных для выполнения над объектом.
  virtual QList<QAction*> setupContextMenu(Menu* menu, const QPoint& screenPos);
  //! Обрабатывает команду выбранную в контекстном меню.
  virtual void processContextMenu(QAction* act);

  //! Данная функция переопределяется в дочерних класса для получения событий поступающих от action'на
  virtual void mouseMoveEvent(QMouseEvent* e);
  //! Данная функция переопределяется в дочерних класса для получения событий поступающих от action'на
  virtual void mouseReleaseEvent(QMouseEvent* e);

  //! Данный метод должен переопределяться в дочерних классах для удаления элементом со сцены.
  virtual void removeFromScene() {}

protected:
  QPolygon lineGeometry(const QLine& source, double width) const;

private:
  MapScene* scene_;
  QString id_;

  bool highlighted_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWOBJECT_H
