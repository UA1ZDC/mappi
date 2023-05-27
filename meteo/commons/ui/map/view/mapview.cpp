#include "mapview.h"

#include <qdebug.h>
#include <qbrush.h>
#include <qscrollbar.h>
#include <qcoreapplication.h>
#include <qgraphicsgridlayout.h>
#include <qgraphicseffect.h>
#include <qcoreapplication.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/incut.h>

#include "mapscene.h"
#include "action.h"
#include "mapwindow.h"

namespace meteo {
namespace map {

MapView::MapView( MapWindow* p )
  : QGraphicsView(p),
  mapscene_(nullptr),
  window_(p)
{
  setFrameShape( QFrame::NoFrame );
  QGraphicsView::setRenderHint( QPainter::SmoothPixmapTransform, true );
  QGraphicsView::setRenderHint( QPainter::Antialiasing, true );
  QGraphicsView::setOptimizationFlags(
      QGraphicsView::DontSavePainterState
      | QGraphicsView::DontAdjustForAntialiasing
      | QGraphicsView::IndirectPainting );
  QGraphicsView::setViewportUpdateMode( QGraphicsView::SmartViewportUpdate );
  QGraphicsView::setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  QGraphicsView::setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  QGraphicsView::setMouseTracking(true);
  QGraphicsView::setCacheMode( QGraphicsView::CacheBackground );
}

MapView::~MapView()
{
  if ( nullptr != mapscene_ ) {
    mapscene_->setMapview(nullptr);
  }
}

void MapView::scrollToCenter()
{
  int hmin = horizontalScrollBar()->minimum();
  int hmax = horizontalScrollBar()->maximum();
  int vmin = verticalScrollBar()->minimum();
  int vmax = verticalScrollBar()->maximum();
  int xpos = hmin + (hmax - hmin)/2;
  int ypos = vmin + (vmax - vmin)/2;
  horizontalScrollBar()->setValue(xpos);
  verticalScrollBar()->setValue(ypos);
}

void MapView::setMapscene( MapScene* sc )
{
  if ( nullptr != mapscene_ ) {
    mapscene_->muteEvents();
  }
  mapscene_ = sc;
  QGraphicsView::setScene(mapscene_);
  if ( nullptr != mapscene_ ) {
    mapscene_->setMapview(this);
    if ( nullptr != mapscene_->document() ) {
      resize( mapscene_->document()->documentsize().width(), mapscene_->document()->documentsize().height());
    }
    scrollToCenter();
    mapscene_->arrangeActions();
    mapscene_->turnEvents();
  }
  emit sceneChanged(mapscene_);
}

void MapView::contextMenuEvent( QContextMenuEvent* e )
{
  QGraphicsView::contextMenuEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->contextMenuEvent(e);
   }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->contextMenuEvent(e);
  }
}

void MapView::dragEnterEvent( QDragEnterEvent* e )
{

  QGraphicsView::dragEnterEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dragEnterEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dragEnterEvent(e);
  }
}

void MapView::dragLeaveEvent( QDragLeaveEvent* e )
{
  QGraphicsView::dragLeaveEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dragLeaveEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dragLeaveEvent(e);
  }
}

void MapView::dragMoveEvent( QDragMoveEvent* e )
{
  QGraphicsView::dragMoveEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dragMoveEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dragMoveEvent(e);
  }
}

void MapView::dropEvent( QDropEvent* e )
{
  QGraphicsView::dropEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dropEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->dropEvent(e);
  }
}

void MapView::focusInEvent( QFocusEvent* e )
{
  QGraphicsView::focusInEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->focusInEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->focusInEvent(e);
  }
}

void MapView::focusOutEvent( QFocusEvent* e )
{
  QGraphicsView::focusOutEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->focusOutEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->focusOutEvent(e);
  }
}

void MapView::inputMethodEvent( QInputMethodEvent* e )
{
  QGraphicsView::inputMethodEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->inputMethodEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->inputMethodEvent(e);
  }
}

void MapView::keyPressEvent( QKeyEvent* e )
{
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->keyPressEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->keyPressEvent(e);
  }
  QGraphicsView::keyPressEvent(e);
}

void MapView::keyReleaseEvent( QKeyEvent* e )
{
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->keyReleaseEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->keyReleaseEvent(e);
  }
}

void MapView::mouseDoubleClickEvent( QMouseEvent* e )
{
  QGraphicsView::mouseDoubleClickEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mouseDoubleClickEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mouseDoubleClickEvent(e);
  }
}

void MapView::mouseMoveEvent( QMouseEvent* e )
{
  QGraphicsView::mouseMoveEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mouseMoveEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mouseMoveEvent(e);
  }
  if ( Qt::NoButton != e->button() ) {
    mapscene_->arrangeActions();
  }
}

void MapView::mousePressEvent( QMouseEvent* e )
{
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mousePressEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mousePressEvent(e);
  }
  QGraphicsView::mousePressEvent(e);
}

void MapView::mouseReleaseEvent( QMouseEvent* e )
{
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mouseReleaseEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->mouseReleaseEvent(e);
  }
  QGraphicsView::mouseReleaseEvent(e);
}

void MapView::paintEvent( QPaintEvent* e )
{
  QGraphicsView::paintEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->paintEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();;
    if ( true == a->disabled() ) {
      continue;
    }
    a->paintEvent(e);
  }
}

void MapView::resizeEvent( QResizeEvent* e )
{
//  if ( 0 != mapscene_ ) {
//    mapscene_->setDrawState(MapScene::kRenderCachePicture);
//  }
  QGraphicsView::resizeEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  scrollToCenter();

  if ( nullptr == mapscene_->document() ) {
    return;
  }
  mapscene_->document()->resizeDocument( QGraphicsView::size() );

  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->resizeEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->resizeEvent(e);
  }
  if ( nullptr != mapscene_->document()->eventHandler() ) {
    mapscene_->document()->eventHandler()->notifyDocumentChanges( DocumentEvent::Changed );
  }
//  mapscene_->setDrawState(MapScene::kRenderLayers);
}

void MapView::showEvent( QShowEvent* e )
{
  QGraphicsView::showEvent(e);
  if ( false == hasMapscene() ) {
    return;
  }
  scrollToCenter();
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->showEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->showEvent(e);
  }
  mapscene_->turnEvents();
}

void MapView::wheelEvent( QWheelEvent* e )
{
  if ( false == hasMapscene() ) {
    return;
  }
  QListIterator<Action*> it( mapscene_->baseactions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->wheelEvent(e);
  }
  it = QListIterator<Action*>( mapscene_->actions() );
  while ( true == it.hasNext() ) {
    Action* a = it.next();
    if ( true == a->disabled() ) {
      continue;
    }
    a->wheelEvent(e);
  }
  QGraphicsView::wheelEvent(e);
}

void MapView::scrollContentsBy( int dx, int dy )
{
  Q_UNUSED(dx);
  Q_UNUSED(dy);
  QGraphicsView::scrollContentsBy(dx,dy);
  if ( nullptr == mapscene_ ) {
    return;
  }
  mapscene_->arrangeActions();
}

}
}

