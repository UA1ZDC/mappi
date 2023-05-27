#include "mapscene.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qobject.h>
#include <qgraphicsitem.h>
#include <qdebug.h>
#include <qsettings.h>
#include <qpluginloader.h>
#include <qdir.h>
#include <qapplication.h>
#include <qgraphicssceneevent.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/layeritems.h>

#include "mapview.h"

#include "action.h"
#include "debugaction.h"
#include "hidebuttonsaction.h"
#include "layersaction.h"
#include "scaleaction.h"
#include "translateaction.h"
#include "rotateaction.h"
#include "focusaction.h"
#include "selectaction.h"
#include "fanalyseaction.h"
#include "contextmenueventaction.h"
#include "savesxfaction.h"
#include "savebmpaction.h"
#include "printdocaction.h"
#include "opensxfaction.h"
#include "coordsaction.h"
#include "mousecoordaction.h"
#include "incutaction.h"
#include "legendaction.h"
#include "informaction.h"
#include "insertaction.h"

namespace meteo {
namespace map {

MapScene::MapScene( Document* d, MapView* p )
  : QGraphicsScene(p),
  document_(d),
  view_(p),
  state_(kRenderLayers)
{
  QGraphicsScene::setItemIndexMethod( QGraphicsScene::NoIndex );
  QGraphicsScene::setSceneRect( QRectF(QPoint(0,0), document_->mapsize() ) );
  QGraphicsScene::setBackgroundBrush( QBrush( document_->backgroundColor() ) );
  internal::ActionSpace topspace( kTopRight, this );
  topspace.setCellSize(22);
  spaces_.append( topspace );
  spaces_.append( internal::ActionSpace( kTopLeft, this ) );
  spaces_.append( internal::ActionSpace( kBottomLeft, this ) );
  spaces_.append( internal::ActionSpace( kBottomRight, this ) );

  setMapview(view_);
  if ( nullptr != view_ ) {
    view_->setMapscene(this);
  }
  initBaseActions();
  initActions();
}

MapScene::~MapScene()
{
  muteEvents();
  if ( nullptr != view_ ) {
    view_->setMapscene(nullptr);
  }

  spaces_.clear();

  while ( 0 != this->actions_.size() ) {
    auto action = this->actions_.first();
    if ( true == action->deleteble() ){
      delete action;
    }
    else {
      action->removeFromScene();
      this->actions_.removeAll(action);
    }
  }

  while ( 0 != baseactions_.size() ) {
    delete baseactions_[0];
  }

  delete document_;
}

void MapScene::setMapview( MapView* v )
{
 view_ = v;
 if ( nullptr != view_ && nullptr != document_ ) {
   document_->resizeDocument( view_->size() );
 }
}

MapView* MapScene::mapview() const
{
  MapView* v = nullptr;
  QList<QGraphicsView*> vs = QGraphicsScene::views();
  for ( int i = 0, sz = vs.size(); i < sz; ++i ) {
    MapView* tmp = qobject_cast<MapView*>(vs[i]);
    if ( nullptr != tmp ) {
      v = tmp;
      break;
    }
  }
  return v;
}

void MapScene::turnEvents()
{
  if ( nullptr == document_ ) {
    return;
  }
  document_->turnEvents();
  if ( nullptr != document_->eventHandler() ) {
    document_->eventHandler()->installEventFilter(this);
  }
  else {
    error_log << QObject::tr("Не удалось включить уведомления о событиях документа");
  }
}

void MapScene::muteEvents()
{
  if ( nullptr == document_ ) {
    return;
  }
  if ( nullptr != document_->eventHandler() ) {
    document_->eventHandler()->removeEventFilter(this);
  }
  else {
    error_log << QObject::tr("Не удалось выключить уведомления о событиях документа");
  }
  document_->muteEvents();
}

void MapScene::setDocument( Document* doc )
{
  takeDocument();
  document_ = doc;
  if ( nullptr != document_ ) {
    turnEvents();
  }
  emit documentChanged(document_);
}

void MapScene::rmDocument()
{
  takeDocument();
  delete document_;
  document_ = nullptr;
}

Document* MapScene::takeDocument()
{
  Document* doc = document_;
  muteEvents();
  document_ = nullptr;
  if ( nullptr != doc ) {
    emit documentRemoved(doc);
  }
  return doc;
}

void MapScene::setScale( double scale )
{
  if ( nullptr == document_ ) {
    return;
  }
  document_->setScale(scale);
}

void MapScene::setScreenCenter( const QPoint& point )
{
  if ( nullptr == document_ ) {
    return;
  }
  document_->setOffCenterMap( QPoint(0,0) );
  document_->setScreenCenter(QPointF(point));
}

bool MapScene::eventFilter( QObject* watched, QEvent* ev )
{
  EventHandler* handler = qobject_cast<EventHandler*>(watched);
  if ( nullptr == handler ) {
    error_log << QObject::tr("Неизвестная ошибка");
    return false;
  }
  if ( nullptr == document_ ) {
    return false;
  }
  if ( MapEvent::MapChanged == ev->type() ) {
    return false;
  }
  if ( DocumentEvent::DocumentChanged == ev->type() ) {
    DocumentEvent* event = reinterpret_cast<DocumentEvent*>(ev);
    switch ( event->changeType() ) {
      case DocumentEvent::Changed:
        state_ = kRenderLayers;
        QGraphicsScene::invalidate( QGraphicsScene::sceneRect(), QGraphicsScene::BackgroundLayer );
        break;
      case DocumentEvent::IncutChanged:
      case DocumentEvent::LegendChanged:
        QGraphicsScene::invalidate( QGraphicsScene::sceneRect(), QGraphicsScene::BackgroundLayer );
        break;
      default:
        break;
    }
  }
  if ( LayerEvent::LayerChanged == ev->type() ) {
    LayerEvent* event = reinterpret_cast<LayerEvent*>(ev);
    if (
        false == document_->hasLayer(event->layer())
        && LayerEvent::Deleted != event->changeType() ) {
  //    error_log << QObject::tr("Ошибка. Слой %1 не найден. Событие %2")
  //      .arg( event->layer() )
  //      .arg( event->changeType() );
      return false;
    }
    switch ( event->changeType() ) {
      case LayerEvent::Moved:
        {
          state_ = kRenderLayers;
          QGraphicsScene::invalidate( QGraphicsScene::sceneRect(), QGraphicsScene::BackgroundLayer );
        }
        break;
      case LayerEvent::Visibility:
        {
          state_ = kRenderLayers;
          QGraphicsScene::invalidate( QGraphicsScene::sceneRect(), QGraphicsScene::BackgroundLayer );
        }
        break;
      case LayerEvent::Added:
        {
          state_ = kRenderLayers;
          QGraphicsScene::invalidate( QGraphicsScene::sceneRect(), QGraphicsScene::BackgroundLayer );
        }
        break;
      case LayerEvent::Deleted:
        state_ = kRenderLayers;
        QGraphicsScene::invalidate( QGraphicsScene::sceneRect(), QGraphicsScene::BackgroundLayer );
        break;
      case LayerEvent::ObjectChanged: {
        QRect r = event->changerect();
        if ( true == r.isEmpty() ) {
          r = sceneRect().toRect();
        }
        state_ = kRenderLayers;
        QGraphicsScene::invalidate( r, QGraphicsScene::BackgroundLayer );
        }
        break;
      case LayerEvent::Activity:
        QGraphicsScene::invalidate( QGraphicsScene::sceneRect(), QGraphicsScene::BackgroundLayer );
      break;
      default:
        break;
    }
  }
  return false;
}

bool MapScene::isMapItem( QGraphicsItem* item )
{
  if ( nullptr == item ) {
    return false;
  }
  return false;
}

void MapScene::addAction(Action* action, int pos, const QPoint& pnt )
{
  addAction(action);
  for ( int i = 0, sz = spaces_.size(); i < sz; ++i ) {
    if ( pos == spaces_[i].pos() ) {
      spaces_[i].addAction( pnt, action );
    }
  }
}

void MapScene::addAction(Action* action )
{
  if ( nullptr != action && false == actions_.contains(action) ) {
    actions_.append(action);
    action->setScene(this);
  }
}

Action* MapScene::getAction(const QString& name) const
{
  Action* a = nullptr;
  for ( int i = 0, sz = actions_.size(); i < sz; ++i ) {
    if ( name == actions_[i]->name() ) {
      a = actions_[i];
      break;
    }
  }
  return a;
}

Action* MapScene::takeAction( const QString& name )
{
  Action* a = getAction(name);
  removeAction(a);
  return a;
}

void MapScene::removeAction( Action* a )
{
  if ( nullptr != a ) {
    a->setScene(nullptr);
    actions_.removeAll(a);
    baseactions_.removeAll(a);
    for ( int i = 0, sz = spaces_.size(); i < sz; ++i ) {
      if ( true == spaces_[i].hasAction(a) ) {
        spaces_[i].takeAction(a);
        break;
      }
    }
  }
}

bool MapScene::hasAction( const QString& n ) const
{
 // debug_log<<"search"<< n;
  for ( int i = 0, sz = actions_.size(); i < sz; ++i ) {
 //   debug_log<<"her"<< actions_[i]->name();
    if ( actions_[i]->name() == n ) {
      return true;
    }
  }
  return false;
}

void MapScene::arrangeActions()
{
  for ( int i = 0, sz = spaces_.size(); i < sz; ++i ) {
    spaces_[i].arrangeActions();
  }
}

void MapScene::drawItems( QPainter* p, int numitems, QGraphicsItem* items[], const QStyleOptionGraphicsItem opt[], QWidget* wgt )
{
  if ( nullptr != document_ && nullptr != document_->ramka() ) {
    QVector<QGraphicsItem*> visibleitems;
    for ( int i = 0, sz = actions_.size(); i < sz; ++i ) {
      Action* a = actions_[i];
      if ( nullptr != a->item() ) {
        visibleitems.append( a->item() );
      }
    }
    for ( int i = 0, sz = visibleitems_.size(); i < sz; ++i ) {
      visibleitems.append(visibleitems_[i]);
    }
    QVector<QGraphicsItem*> otheritems;
    QVector<QGraphicsItem*> docitems;
    for ( int i = 0, sz = numitems; i < sz; ++i ) {
      QGraphicsItem* item = items[i];
      if ( document_->hasItem(item) ) {
        docitems.append(item);
      }
      else if ( false == visibleitems.contains( items[i] ) ) {
        otheritems.append(items[i]);
      }
    }
    p->save();
    QPolygon poly = document_->ramka()->calcRamka(0, -document_->ramkaWidth() );
    QPainterPath path;
    path.addPolygon(poly);
    p->setClipPath(path);
    QGraphicsScene::drawItems( p, otheritems.size(), otheritems.data(), opt, wgt );
    if ( nullptr != document_->itemsLayer() && true == document_->itemsLayer()->visible() ) {
      QGraphicsScene::drawItems( p, docitems.size(), docitems.data(), opt, wgt );
    }
    p->restore();
    QGraphicsScene::drawItems( p, visibleitems.size(), visibleitems.data(), opt, wgt );
  }
  else {
    QGraphicsScene::drawItems( p, numitems, items, opt, wgt);
  }
}

void MapScene::drawBackground( QPainter* p, const QRectF& r )
{
  Q_UNUSED(r);
  if ( nullptr == document_ ) {
    return;
  }
  p->fillRect(QGraphicsScene::sceneRect(),document_->backgroundColor());
  QRect target = document_->documentRect();
  if ( kRenderCachePicture == state_ ) {
    p->save();
    p->setRenderHint( QPainter::SmoothPixmapTransform, true );
    if ( nullptr != document_->ramka() ) {
      QPolygon poly = document_->ramka()->calcRamka(0,0);
      QPainterPath path;
      path.addPolygon(poly);
      p->setClipPath(path);
    }
    QTransform tr = cachedTransform();
    p->setTransform(tr*p->transform() );
    p->drawPixmap( document_->mapRect().topLeft(), cacheimage_ );
    p->restore();
  }
  else {
    p->save();
    cacheimage_ = QPixmap( document_->mapsize() );
    cacheimage_.fill( QColor( 255,255,255,0) );
    QPainter painter(&cacheimage_);
    painter.translate( -sceneRect().topLeft() );
    QApplication::setOverrideCursor( Qt::WaitCursor );
    document_->setIsolineRamka( document_->calcIsolineRamka( -60, -60  ) );

    if ( nullptr != document_->ramka() ) {
      QPolygon poly = document_->ramka()->calcRamka(0,0);
      QPainterPath path;
      path.addPolygon(poly);
      painter.setClipPath(path);
    }
    document_->freezeText( &painter, target );

    for ( auto l : document_->layers() ) {
      if ( l == document_->itemsLayer() ) {
        continue;
      }
      if ( true == document_->property().cache() && true == l->cache() ) {
        continue;
      }
      document_->drawLayer( l->uuid(), &painter, target );
    }
    document_->defreezeText( &painter );
    QApplication::restoreOverrideCursor();
    p->drawPixmap( sceneRect().topLeft(), cacheimage_ );
    cachedbunch_.clear();
    cachedbunch_.append( screen2coord( QGraphicsScene::sceneRect().center().toPoint() + QPoint(-50,-50) ) );
    cachedbunch_.append( screen2coord( QGraphicsScene::sceneRect().center().toPoint() + QPoint(50,-50) ) );
    cachedbunch_.append( screen2coord( QGraphicsScene::sceneRect().center().toPoint() + QPoint(50,50) ) );
    cachedbunch_.append( screen2coord( QGraphicsScene::sceneRect().center().toPoint() + QPoint(-50,50) ) );
    p->restore();
  }
  document_->drawRamka(p);
//  p->drawPolygon( document_->isolineRamka() ); TEST
  state_ = kRenderCachePicture;

}

void MapScene::drawForeground( QPainter* p, const QRectF& r )
{
  if ( nullptr == document_ ) {
    return;
  }
  QGraphicsScene::drawForeground( p, r );
  document_->drawLegend(p);

  document_->drawIncut(p);
}

void MapScene::wheelEvent( QGraphicsSceneWheelEvent* e )
{
  QGraphicsScene::wheelEvent(e);
  e->setAccepted(true);
}

void MapScene::keyPressEvent (QKeyEvent* e )
{
  QGraphicsScene::keyPressEvent(e);
  e->setAccepted(true);
}

void MapScene::reloadActionPlugins()
{
  for ( int i = 0, sz = plugroups_.size(); i < sz; ++i ) {
    const QString& n = plugroups_[i];
    QDir dir( MnCommon::pluginPath("") + n );
    if ( false == dir.exists() ) {
      warning_log << QObject::tr("Директория %1 с плагинами не обнаружена")
                     .arg( dir.absolutePath() );
      continue;
    }
    QStringList files = dir.entryList( QStringList() << "*.so" << "*.dll" );
    for ( int j = 0, jsz = files.size(); j < jsz; ++j ) {
      QString pname( dir.absoluteFilePath(files[j]));
      QPluginLoader loader( pname, this );
      if ( false == loader.isLoaded() ) {
        if ( false == loader.load() ) {
          error_log << QObject::tr("Не удалось загрузить плагин для сцены %1.\n\tОшибка = %2")
            .arg(pname)
            .arg(loader.errorString());
          continue;
        }
      }
      QObject* o =  loader.instance();
      ActionPlugin* ap = qobject_cast<meteo::map::ActionPlugin*>(o);
      if ( nullptr == ap ) {
        error_log << QObject::tr("Не удалось создать объект плагина %1.\n\tОшибка = %2").arg(pname).arg(loader.errorString());
        delete o;
        continue;
      }

      if (hasAction(ap->actioName())) {
        delete o;
        continue;
      }

      Action* a = ap->create(this);
      if ( nullptr != a ) {
        if ( false == a->hasCorner() ) {
          addAction(a);
        }
        else {
          addAction( a, a->corner(), a->cornerAddress() );
        }
      }
      else {
        error_log << QObject::tr("Плагин не создан");
      }
      delete o;
    }
  }
}

QTransform MapScene::cachedTransform() const
{
  QTransform tr;
  QPolygon mapscreen;
  if ( 4 != cachedbunch_.size() ) {
    return tr;
  }
//  for ( int i = 0; i < 3; ++i ) {
  for ( int i = 0; i < 4; ++i ) {
    QPoint pnt = coord2screen( cachedbunch_[i] );
    mapscreen.append( pnt );
  }

  QRect screct = QGraphicsScene::sceneRect().toRect();
  QPolygon scenescreen;
  scenescreen.append( screct.center() + QPoint(-50,-50)  );
  scenescreen.append( screct.center() + QPoint(50,-50)  );
  scenescreen.append( screct.center() + QPoint(50,50)  );
  scenescreen.append( screct.center() + QPoint(-50,50)  );
  bool res = QTransform::quadToQuad( scenescreen, mapscreen, tr );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось посчитать матрицу документа");
  }
  return tr;

//  tr = QTransform(
//      1, screct.center().x(), screct.center().y(),
//      1, (screct.center() + QPoint(100,0)).x(), (screct.center() + QPoint(100,0)).y(),
//      1, (screct.center() + QPoint(0,100)).x(), (screct.center() + QPoint(0,100)).y()
//      );
//  tr = tr.inverted();
//
//  QVector<qreal> a = QVector<qreal>()
//    << tr.m11()*mapscreen[0].x() + tr.m12()*mapscreen[1].x() + tr.m13()*mapscreen[2].x()
//    << tr.m21()*mapscreen[0].x() + tr.m22()*mapscreen[1].x() + tr.m23()*mapscreen[2].x()
//    << tr.m31()*mapscreen[0].x() + tr.m32()*mapscreen[1].x() + tr.m33()*mapscreen[2].x();
//
//
//  QVector<qreal> b = QVector<qreal>()
//    << tr.m11()*mapscreen[0].y() + tr.m12()*mapscreen[1].y() + tr.m13()*mapscreen[2].y()
//    << tr.m21()*mapscreen[0].y() + tr.m22()*mapscreen[1].y() + tr.m23()*mapscreen[2].y()
//    << tr.m31()*mapscreen[0].y() + tr.m32()*mapscreen[1].y() + tr.m33()*mapscreen[2].y();
//
//  return QTransform( a[1], b[1], a[2], b[2], a[0], b[0] );
}

internal::ActionSpace* MapScene::space( int pos )
{
  for ( int i = 0, sz  = spaces_.size(); i < sz; ++i ) {
    if ( spaces_[i].pos() == pos ) {
      return &( spaces_[i] );
    }
  }
  return nullptr;
}

void MapScene::initBaseActions()
{
  Action* action = new TranslateAction(this);
  baseactions_.append(action);

  action = new ScaleAction(this);
  addAction( action, kTopLeft, QPoint( 0, 0 ) );
  baseactions_.append(action);
  if ( nullptr != document_
      && ( meteo::STEREO == document_->projection()->type() || ( meteo::CONICH == document_->projection()->type()) ) ) {
    action = new RotateAction(this);
    addAction( action, kTopLeft, QPoint( 3, 0 ) );
    baseactions_.append(action);
  }
  action = new FocusAction(this);
  addAction( action, kTopLeft, QPoint( 2, 0 ) );
  baseactions_.append(action);

  action = new CoordsAction(this);
  baseactions_.append(action);
}

void MapScene::initActions()
{
  Action* action = new ContextMenuEventAction(this);
  addAction(action);

//  action = new DebugAction(this);
//  addAction( action, kBottomLeft, QPoint( 0,-1 ) );

  action = new LayersAction(this);
  addAction( action, kTopLeft, QPoint(0,2) );

 /* if ( nullptr != document_ && document_->property().doctype() == map::proto::kGeoMap ) {
    action = new SavesxfAction(this);
    addAction(action);

    action = new OpenSxfaction(this);
    addAction(action);
  }*/

  action = new PrintdocAction(this);
  addAction(action);

  action = new SavebmpAction(this);
  addAction(action);

  action = new MouseCoordAction(this);
  addAction( action, action->corner(), action->cornerAddress() );
  action = new IncutAction(this);
  addAction(action);

  action = new LegendAction(this);
  addAction(action);
  action = new HideButtonsAction(this);
  addAction(action);
  action = new InformAction(this);
  addAction(action);
  action = new InsertAction(this);
  addAction(action);
}

const QStringList& MapScene::pluginGroups() const
{
  return plugroups_;
}

void MapScene::setPluginGroups(const QStringList& groups)
{
  plugroups_ = groups;
  reloadActionPlugins();
}

void MapScene::addPluginGroup(const QString& gr)
{
  if (false == plugroups_.contains(gr)) {
    plugroups_.append(gr);
    reloadActionPlugins();
  }
}

void MapScene::rmPluginGroup(const QString& gr)
{
  if (true == plugroups_.contains(gr)) {
    plugroups_.removeAll(gr);
    reloadActionPlugins();
  }
}

void MapScene::addVisibleItem( QGraphicsItem* item )
{
  if ( nullptr != item && false == visibleitems_.contains(item) ) {
    visibleitems_.append(item);
  }
}

void MapScene::removeVisibleItem( QGraphicsItem* item )
{
  visibleitems_.removeAll(item);
}

namespace internal {

ActionSpace::ActionSpace( int p, MapScene* scene )
  : pos_(p),
  size_( QSize(0,0) ),
  cellsize_(40),
  scene_(scene)
{
}

ActionSpace::~ActionSpace()
{
}

void ActionSpace::addAction( const QPoint& pnt, Action* a )
{
  if ( nullptr == a->item() ) {
    return;
  }
  a->setHasCorner(true);
  a->setCorner(pos_);
  QPoint npos = pnt;
  if ( QPoint(-1,-1) == pnt ) {
    npos = QPoint( size_.width(), 0 );
  }
  a->setCornerAddress(npos);
  actions_.insert( npos, a );
  int width = 0;
  int height = 0;
  QMapIterator<QPoint, Action*> it(actions_);
  while ( true == it.hasNext() ) {
    it.next();
    const QPoint& p = it.key();
    Action* a = it.value();
    if ( p.x() + a->size().width()  > width ) {
      width = p.x() + a->size().width() ;
    }
    if ( p.y() + a->size().height() > height ) {
      height = p.y() + a->size().height() ;
    }
  }
  size_ = QSize( width, height );
  arrangeActions();
}

Action* ActionSpace::takeAction( const QString& name )
{
  Action* a = nullptr;
  if ( false == hasAction(name) ) {
    return a;
  }
  QPoint pnt;
  QMapIterator< QPoint, Action* > it(actions_);
  while ( true == it.hasNext() ) {
    it.next();
    if ( it.value()->name() == name ) {
      a = it.value();
      pnt = it.key();
      break;
    }
  }
  if ( nullptr != a ) {
    actions_.remove(pnt);
  }
  if ( nullptr != scene_ && nullptr != a && nullptr != a->item() ) {
    scene_->removeItem( a->item() );
  }
  arrangeActions();
  return a;
}

void ActionSpace::takeAction( Action* a )
{
  if ( false == hasAction(a) ) {
    return;
  }
  QPoint pnt;
  QMapIterator< QPoint, Action* > it(actions_);
  while ( true == it.hasNext() ) {
    it.next();
    if ( it.value() == a ) {
      pnt = it.key();
      break;
    }
  }
  if ( nullptr != a ) {
    actions_.remove(pnt);
  }
  if ( nullptr != scene_ && nullptr != a && nullptr != a->item() ) {
    scene_->removeItem( a->item() );
  }
  arrangeActions();
}

Action* ActionSpace::takeAction( const QPoint& pos )
{
  if ( false == actions_.contains(pos) ) {
    return nullptr;
  }
  Action* a = actions_[pos];
  if ( nullptr != scene_ && nullptr != a && nullptr != a->item() ) {
    scene_->removeItem( a->item() );
  }
  actions_.remove(pos);
  arrangeActions();
  return a;
}

bool ActionSpace::hasAction( const QString& name ) const
{
  QMapIterator< QPoint, Action* > it(actions_);
  while ( true == it.hasNext() ) {
    it.next();
    if ( it.value()->name() == name ) {
      return true;
    }
  }
  return false;
}

bool ActionSpace::hasAction( Action* a ) const
{
  QMapIterator< QPoint, Action* > it(actions_);
  while ( true == it.hasNext() ) {
    it.next();
    if ( it.value() == a ) {
      return true;
    }
  }
  return false;
}

void ActionSpace::arrangeActions()
{
  if ( -1 == pos_ ) {
    return;
  }
  if ( false == scene_->hasMapview() ) {
    return;
  }
  QSize s = size();
  QPoint topleft;
  switch ( pos_ ) {
    case kTopLeft:
      topleft = scene_->mapview()->mapToScene( QPoint( 25, 25 ) ).toPoint();
      break;
    case kTopRight:
      topleft = scene_->mapview()->mapToScene( QPoint( scene_->mapview()->viewport()->width() - s.width() - 25, 25 ) ).toPoint();
      break;
    case kBottomLeft:
      topleft = scene_->mapview()->mapToScene( QPoint( 25, scene_->mapview()->viewport()->height() - s.height() - 17 ) ).toPoint();
      break;
    case kBottomRight:
      topleft = scene_->mapview()->mapToScene( QPoint( scene_->mapview()->viewport()->width() - s.width() - 3, scene_->mapview()->viewport()->height() - s.height() - 3 ) ).toPoint();
      break;
    default:
      return;
  }

  QMap< QPoint, Action* >::iterator it = actions_.begin();
  QMap< QPoint, Action* >::iterator end = actions_.end();
  for ( ; it != end; ++it ) {

    Action* a = it.value();
    if ( nullptr == a->item() ) {
      return;
    }
    QPoint pnt =  it.key();
    a->item()->setPos( topleft + pnt*cellsize_ );
  }
}

QSize ActionSpace::size() const
{
  return QSize( size_*cellsize_ );
}

QRect ActionSpace::rect() const
{
  if ( false == scene_->hasMapview() ) {
    return QRect();
  }
  QSize s = size();
  QPoint topleft;
  switch ( pos_ ) {
    case kTopLeft:
      topleft = scene_->mapview()->mapToScene( QPoint( 3, 3 ) ).toPoint();
      break;
    case kTopRight:
      topleft = scene_->mapview()->mapToScene( QPoint( scene_->mapview()->viewport()->width() - s.width() - 3, 3 ) ).toPoint();
      break;
    case kBottomLeft:
      topleft = scene_->mapview()->mapToScene( QPoint( 3, scene_->mapview()->viewport()->height() - s.height() - 3 ) ).toPoint();
      break;
    case kBottomRight:
      topleft = scene_->mapview()->mapToScene( QPoint( scene_->mapview()->viewport()->width() - s.width() - 3, scene_->mapview()->viewport()->height() - s.height() - 3 ) ).toPoint();
      break;
    default:
      return QRect();
  }
  return QRect( topleft, size() );
}

}
}

}

bool operator<( const QPoint& p1, const QPoint& p2 )
{
  if ( p1.x() < p2.x() ) { return true; } if ( p1.y() < p2.y() ) { return true; } return false;
}
