#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPOBJECT_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPOBJECT_H

#include <qgraphicsitem.h>
#include <unordered_set>

class QGraphicsPathItem;

namespace meteo {

class Projection;

namespace map {

class Object;
class GeoGroup;
class MapScene;
class Document;

class GroupObject: public QGraphicsObject
{
  Q_OBJECT
public:
  enum State {
    kNone,
    kMove
  };
  GroupObject( QGraphicsItem* parent = nullptr );
  GroupObject( GeoGroup* geoGroup, QGraphicsItem* parent = nullptr );
  ~GroupObject();
  QRectF boundingRect() const;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem *o, QWidget *w );
  void addObject(const Object *object);
  void removeObject(const Object *object);
  void setItem(QGraphicsPathItem* path);
  void finish();
  void abort();
  void deleteObjects();
  void setInvisible(bool invisible);
  bool isInvisible() {return invisible_;}

signals:
  void skeletChanged();

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
  MapScene* mapScene() const;
  Document* doc() const;
  Projection* projection() const;

private:
  std::unordered_set<Object*> objects_;
  QGraphicsPathItem* path_ = nullptr;
  mutable QRectF boundingRect_;
  State state_ = kNone;
  GeoGroup* geoGroup_ = nullptr;
  QMap<GeoGroup*,std::unordered_set<Object*>> listGeoGroup_;
  bool invisible_ = false;
};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPOBJECT_H
