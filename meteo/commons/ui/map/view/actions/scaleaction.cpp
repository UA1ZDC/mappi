#include "scaleaction.h"
#include "../widgets/scalewidget.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qpalette.h>
#include <qgraphicslinearlayout.h>
#include <qgraphicsproxywidget.h>
#include <qdebug.h>
#include <qmatrix.h>
#include <qmath.h>
#include <qcoreapplication.h>
#include <qcursor.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../widgetitem.h"

namespace {
  const QIcon plusButtonIcon() { return QIcon(":/meteo/icons/map/zoom-in.png"); }
  const QIcon minusButtonIcon() { return QIcon(":/meteo/icons/map/zoom-out.png"); }
  const QString plusButtonToolTip() { return QString::fromUtf8("Приблизить"); }
  const QString minusButtonToolTip() { return QString::fromUtf8("Отдалить"); }
  const int kNextBtnStep = 6;
}

namespace meteo {
namespace map {

const int kMsecWait = 150;

ScaleAction::ScaleAction( MapScene* scene )
  : Action(scene, "scaleaction" ),
  tm_( new QTimer(this) ),
  plusBtn_( new ActionButton ),
  minusBtn_( new ActionButton )
{
  scaleSeparator_ = 120.0*5;
  maxScale_ = 1;
  minScale_ = 20;

  tm_->setSingleShot(true);
  setSize( QSize( size_.width()*2, size_.height() ) );

  plusBtn_->setToolTip(plusButtonToolTip());
  plusBtn_->setIcon(plusButtonIcon());
  plusBtn_->setCheckable(false);
  plusBtn_->setIconSize(QSize(20,20));

  minusBtn_->setToolTip(minusButtonToolTip());
  minusBtn_->setIcon(minusButtonIcon());
  minusBtn_->setCheckable(false);
  minusBtn_->setIconSize(QSize(20,20));

  plusItem_ = scene_->addWidget(plusBtn_);
  item_ = plusItem_;
  QGraphicsProxyWidget* minusItem = new QGraphicsProxyWidget(item_);
  minusItem->setWidget(minusBtn_);
  minusItem->setPos(QPoint(cellsize_+kNextBtnStep, 0));

  QObject::connect( tm_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  QObject::connect( plusBtn_, SIGNAL(clicked()), this, SLOT( slotZoomIn()) );
  QObject::connect( minusBtn_, SIGNAL(clicked()), this, SLOT( slotZoomOut()) );
}

ScaleAction::~ScaleAction()
{
  delete tm_; tm_ = 0;
}

void ScaleAction::wheelEvent( QWheelEvent* ev )
{
  tm_->start(kMsecWait);
  scene_->setDrawState( MapScene::kRenderCachePicture );
  scale(-ev->delta(), QCursor::pos());
  view()->setCacheMode( QGraphicsView::CacheNone );
  view()->setCacheMode( QGraphicsView::CacheBackground );
}

void ScaleAction::keyPressEvent(QKeyEvent* ev )
{
  if( ev->key() == Qt::Key_Up ){
    if( Qt::ControlModifier == ( Qt::ControlModifier & ev->modifiers() ) ){
      tm_->start(kMsecWait);
      scene_->setDrawState( MapScene::kRenderCachePicture );
      scale(-100, QCursor::pos());
      view()->setCacheMode( QGraphicsView::CacheNone );
      view()->setCacheMode( QGraphicsView::CacheBackground );
    }
  }
  else
  if( ev->key() == Qt::Key_Down ){
    if( Qt::ControlModifier == ( Qt::ControlModifier & ev->modifiers() ) ){
      tm_->start(kMsecWait);
      scene_->setDrawState( MapScene::kRenderCachePicture );
      scale(100, QCursor::pos());
      view()->setCacheMode( QGraphicsView::CacheNone );
      view()->setCacheMode( QGraphicsView::CacheBackground );
    }
  }
  Action::keyPressEvent(ev);
}

bool ScaleAction::scale( int sc, const QPoint& point  )
{
  if ( false == hasView() ) {
    return false;
  }
//  scene_->setDrawState( MapScene::kRenderLayers );
  double sceneScale = scene_->scale();
  double delta =  double(sc)/scaleSeparator_;
  sceneScale += delta;
  if ( sceneScale < maxScale_ ) {
    sceneScale = maxScale_;
  }
  if ( sceneScale > minScale_ ) {
    sceneScale = minScale_;
  }
  if ( true == MnMath::isEqual( sceneScale, scene_->scale() ) ) {
    return false;
  }
  QPoint pos = view()->mapToScene( view()->mapFromGlobal(point) ).toPoint();
  GeoPoint gp = scene_->screen2coord(pos);
  scene_->setScale( sceneScale );
  QPoint scrcntr = scene_->coord2screen(scene_->document()->screenCenter());
  QPoint newpos = scene_->coord2screen(gp);
  QPoint deltapos = pos - newpos;
  scene_->setScreenCenter( scrcntr - deltapos );
  return true;
}

void ScaleAction::slotTimeout()
{
  tm_->stop();
  scene_->setDrawState( MapScene::kRenderLayers );
  view()->setCacheMode( QGraphicsView::CacheNone );
  view()->setCacheMode( QGraphicsView::CacheBackground );
}

void ScaleAction::slotZoomIn()
{
  if ( false == hasView() || 0 == scene_ ) {
    return;
  }
  tm_->start(kMsecWait);
  scene_->setDrawState( MapScene::kRenderCachePicture );
  QPoint pos = view()->mapFromScene( view()->mapToGlobal(
                                      scene_->document()->coord2screen( scene_->document()->center()) )
                                    );
  scale(-240, pos );
  view()->setCacheMode( QGraphicsView::CacheNone );
  view()->setCacheMode( QGraphicsView::CacheBackground );
}

void ScaleAction::slotZoomOut()
{
  if ( false == hasView() || 0 == scene_ ) {
    return;
  }
  tm_->start(kMsecWait);
  scene_->setDrawState( MapScene::kRenderCachePicture );
  QPoint pos = view()->mapFromScene( view()->mapToGlobal(
                                      scene_->document()->coord2screen( scene_->document()->center()) )
                                    );
  scale(240, pos );
  view()->setCacheMode( QGraphicsView::CacheNone );
  view()->setCacheMode( QGraphicsView::CacheBackground );
}

}
}
