#include "translateaction.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qdebug.h>
#include <qscrollbar.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

#include "../mapview.h"
#include "../mapscene.h"

namespace meteo {
namespace map {

TranslateAction::TranslateAction( MapScene* scene)
  : Action(scene, "translateaction" ),
  sceneGrabbed_(false)
{
}

TranslateAction::~TranslateAction()
{
}

void TranslateAction::mousePressEvent( QMouseEvent* e )
{
  if ( false == hasView() ) {
    return;
  }
  if ( false == (e->buttons() & Qt::LeftButton) ) {
    return;
  }
  view()->setDragMode(QGraphicsView::ScrollHandDrag);
  sceneGrabbed_ = true;
  oldpos_= QPoint( view()->horizontalScrollBar()->value(), view()->verticalScrollBar()->value() );
  newpos_ = oldpos_;
}

void TranslateAction::mouseReleaseEvent( QMouseEvent* e )
{
  Q_UNUSED(e);
  if ( false == hasView() ) {
    return;
  }
  if ( false == sceneGrabbed_ ) {
    return;
  }
  QPoint pos = QPoint( view()->horizontalScrollBar()->value(), view()->verticalScrollBar()->value() );
  if ( pos == oldpos_ ) {
    sceneGrabbed_ = false;
    view()->setDragMode(QGraphicsView::NoDrag);
    return;
  }
  QSize vsz = view()->viewport()->size();
  QPoint point = QPoint( vsz.width(), vsz.height() )/2;
  point = view()->mapToScene(point).toPoint();
  view()->setDragMode(QGraphicsView::NoDrag);
  view()->scrollToCenter();
  view()->setCacheMode( QGraphicsView::CacheNone );
  scene_->setScreenCenter(point);
  sceneGrabbed_ = false;
  scene_->setDrawState( MapScene::kRenderLayers );
  view()->setCacheMode( QGraphicsView::CacheBackground );
}

void TranslateAction::mouseMoveEvent( QMouseEvent* e )
{
  Q_UNUSED(e);
  if ( false == hasView() ) {
    return;
  }
  if ( true == sceneGrabbed_ ) {
    QPoint pos = QPoint( view()->horizontalScrollBar()->value(), view()->verticalScrollBar()->value() );
    if ( pos != oldpos_ && pos != newpos_ ) {
      scene_->setDrawState( MapScene::kRenderCachePicture );
      view()->setCacheMode( QGraphicsView::CacheNone );
      newpos_ = pos;
      QPoint off =  -oldpos_ + newpos_;
      scene_->document()->setOffCenterMap(off);
      view()->setCacheMode( QGraphicsView::CacheBackground );
    }
  }
}



}
}
