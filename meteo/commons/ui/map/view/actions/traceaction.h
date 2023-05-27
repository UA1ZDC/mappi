#ifndef TraceAction_H
#define TraceAction_H

#include <QtGui>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include "traceitem.h"

namespace meteo {
namespace map {

class MapView;
class Layer;
class GeoPolygon;
class ArrowItem;

class TraceAction  : public Action
{
  Q_OBJECT
public:
  enum InteractionMode { InsertItem, MoveItem, MoveMap, DoNothing };
  enum WorkMode { TraceMode, DotMode };
  TraceAction (MapScene* scene, const QString& name);
  ~TraceAction();


  virtual void mousePressEvent( QMouseEvent* e );
  virtual void mouseReleaseEvent( QMouseEvent* e );
  virtual void mouseDoubleClickEvent( QMouseEvent* e );
  virtual void mouseMoveEvent( QMouseEvent* e );
  virtual void addActionsToMenu( Menu* menu ) const ;

  virtual void appendPlace(const GeoPoint& gp, bool move = false );

  virtual TraceItem* addNode( const GeoPoint &gp, bool reverse = false,
                      TraceItem::CollideItemOption collide = TraceItem::HideMark);
  virtual TraceItem* insertNode( const GeoPoint &gp, TraceItem* after, TraceItem* before,
                        TraceItem::CollideItemOption collide = TraceItem::HideMark);
  void installScene( MapScene* sc );
  virtual void removeFromScene();

  int addTrace();
  int current_;

  void activate() { activate_ = true; }
  void deactivate() { activate_ = false; }
  void setInteractionMode(InteractionMode mode);
  void setWorkMode( WorkMode mode );
  void deleteItem( TraceItem* item );
  void setTraceEditable(bool st);
  void setSearchNear(bool sn) {search_near_ = sn;}

  int nodeCount(int num_trace) const;

  public slots:
    void slotChangeNode(int num, const GeoPoint& point, const QString& text);
    void slotAppendSingleNode(const GeoPoint& point, const QString& title, bool move);
    void slotRemoveTrace();

protected:
  InteractionMode interactionMode_;
  WorkMode workmode_;
  TraceItem::ItemShape shape_;
  QMap < int, QList<TraceItem*> > traces_;
  QAction *deleteAction_;
  ArrowItem* createArrow(TraceItem* itemStart, TraceItem* itemEnd);
  bool press_;
  bool activate_;
  bool editable_;
  bool search_near_=false;
  Layer* activeLayer();

  bool eventFilter( QObject* o, QEvent* e );
  static QString makePlaceName(const meteo::GeoPoint& gpoint, bool search_near );

protected slots:
  void deleteItem();
  void slotProcessMenuAction();  
  void slotItemRemove(int num);
  void slotItemChange(int, const GeoPoint&, const QString &);
  void slotItemAdd(int, const GeoPoint&, const QString &);
  void slotItemInsert();

  void slotSceneToNull(QObject*);
signals:
  void changeItem(int, const GeoPoint& );
  void removeTrace();
  void itemAdded(int, const GeoPoint& coord);
  void itemRemoved(int num);
};

}
}

#endif
