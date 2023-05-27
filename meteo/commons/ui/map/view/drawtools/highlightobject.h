#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_HIGHLIGHTOBJECT_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_HIGHLIGHTOBJECT_H

#include <qlist.h>
#include <qline.h>
#include <qpolygon.h>

class QGraphicsPathItem;


namespace meteo {
namespace map {

class Object;
class MapScene;


//! Класс HighlightObject отвечает за "подсветку" выделенных объектов
class HighlightObject
{
public:
  HighlightObject(const Object* object, MapScene* scene);
  ~HighlightObject();

  //! Возвращает географический объект, для которого отображается выделение
  const Object* object() const { return object_; }
  //! Пересчёт координат узлов сцены на основе географических координат
  void calcSceneCoords();
  //! Возвращает true, если выделенный объект находится под курсором мыши
  bool isUnderMouse() const;
  //! Возвращает true, если точка scenePoint располагается на каркасе объекта.
  bool isPointOnObject(const QPointF& scenePoint, int width) const;
  //! Возвращает true, если obj содержится в иерархии объектов, для которой отображается выделение
  bool contains(const Object* obj) const;

  void removeFromScene();
  void setLayerUuid( const QString& layeruuid ) { layeruuid_ = layeruuid; }
  const QString& layerUuid() const { return layeruuid_; }

private:
  QPolygon lineGeometry(const QLine& source, double width) const;

private:
  const Object* object_;
  MapScene* scene_;
  QGraphicsPathItem* sceneItem_;
  QList<HighlightObject*> childs_;
  QString layeruuid_;
};

}
}

#endif
