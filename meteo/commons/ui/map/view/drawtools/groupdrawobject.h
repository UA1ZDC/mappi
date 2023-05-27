#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPDRAWOBJECT_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPDRAWOBJECT_H

#include "drawobject.h"

#include <qlist.h>
#include <qgraphicsitem.h>

#include <meteo/commons/ui/map/geogroup.h>

class QMouseEvent;

namespace meteo {
namespace map {

class Object;
class GeoGroup;
class MapScene;
class GeoObjectItem;

//!
class SkeletItem : public QGraphicsItem
{
public:
  SkeletItem(const QPolygon& skelet, QGraphicsItem* parent = 0, MapScene* scene = 0);
  virtual ~SkeletItem() {}
  //!
  virtual QRectF boundingRect() const;
  //!
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
  //!
  void setSkelet(const QPolygon& skelet);
  //!
  const QList<QLine>& sections() const { return sections_; }
  //!
  QLine sectionAt(const QPoint& scenePos, int width) const;
  //!
  QPolygon skelet() const { return skelet_; }
  //!
  void updateSkelet();
  //!
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
  QPolygon lineGeometry(const QLine& source, double width) const;

private:
  QRectF boundingRect_;
  MapScene* scene_;
  QPolygon skelet_;
  QList<QLine> sections_;

  QPen pen_;
};

//! Класс GroupDrawObject предоставляет интерфейс для редактирования объектов типа GeoGroup
class GroupDrawObject : public DrawObject
{
public:
  //! Конструирует объект, с идентификатором id, для редактирования гео-группы на сцене scene.
  GroupDrawObject(MapScene* mapScene, const QString& id = QString());
  //!
  virtual ~GroupDrawObject();

  //! Возвращает гео-объект или 0, если объект не установлен.
  virtual const Object* object() const { return object_; }
  //! Устанавливает значение узлов каркаса гео-объекта в координатах сцены.
  virtual void setObject(const Object* obj);

  //! Пересчёт координат узлов сцены на основе географических координат
  virtual void calcSceneCoords();
  //!
  QLine skeletSectionAt(const QPoint &scenePos, int width) const;
  //!
  void setMovable(bool enable);
  //!
  bool isMovable() const;

  //! Устанавливает значение гео-объекта.
  virtual void setObjectValue(double value, const QString& format = "4' '.2'0'", const QString& unit = QString());
  //! Удаляет значение гео-объекта.
  virtual void removeObjectValue();

  //!
  virtual void mouseMoveEvent(QMouseEvent* e);

private:
  void addObjectSkelet(Object* obj);
  void updateObjectSceneSkelet(Object* obj, const QStringList& uuidList, const QList<SkeletItem*> skeletList);
  void moveSkelets(Object* obj, const QStringList& uuidList, const QList<SkeletItem*> skeletList);

  void deleteSkeletItems();

  // debug funcs
  void printDebug(Object* obj, int indent = 0);

private:
  GeoGroup* object_;
  GeoObjectItem* objItem_;
  QGraphicsItemGroup* groupItem_;

  QStringList uuidList_;
  QList<SkeletItem*> skeletList_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPDRAWOBJECT_H
