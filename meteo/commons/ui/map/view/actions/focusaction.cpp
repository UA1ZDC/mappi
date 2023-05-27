#include "focusaction.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include "../actionbutton.h"
#include "../mapview.h"
#include "../mapscene.h"
#include "../widgetitem.h"

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include <unistd.h>

namespace {
  const QIcon focusButtonIcon() { return QIcon(":/meteo/icons/tools/center.png"); }
  const QString focusButtonToolTip() { return QString::fromUtf8("Центрирование"); }
}

namespace meteo {
namespace map {

FocusAction::FocusAction( MapScene* scene )
  : Action(scene, "focusaction" ),
  focusBtn_( new ActionButton ),
  active_(false),
  started_(false)
{
  focusBtn_->setToolTip(focusButtonToolTip());
  focusBtn_->setIcon(focusButtonIcon());
  focusItem_ = scene_->addWidget(focusBtn_);
  item_ = focusItem_;
  QObject::connect( focusBtn_, SIGNAL(toggled(bool)), this, SLOT( slotActivate(bool) ) );

}

FocusAction::~FocusAction()
{
//  delete focusBtn_; focusBtn_ = 0;
}

void FocusAction::mouseReleaseEvent(QMouseEvent *e)
{
  if (   ( true == focusItem_->isUnderMouse() )
      || ( false == hasView() || 0 == parent() )
      || ( false == active_ )
      || ( 0 == scene_->document() )
     ) {
    return;
  }

  if ( true == started_ ) {
    started_ = false;
    Action::mouseReleaseEvent(e);
    return;
  }

  started_ = true;
  QPoint pnt = scene_->mapview()->mapToScene( e->pos() ).toPoint();
  view()->setCacheMode( QGraphicsView::CacheNone );
  GeoPoint geopnt = scene_->document()->screen2coord(pnt);

  if (meteo::STEREO == scene_->document()->projection()->type() || meteo::CONICH == scene_->document()->projection()->type() ) {
    setRotateAngle( geopnt.lon() );
  }

  scene_->document()->setScreenCenter(geopnt);
  scene_->setDrawState( MapScene::kRenderLayers );
  view()->setCacheMode( QGraphicsView::CacheBackground );
  Action::mouseReleaseEvent(e);
  if ( nullptr != focusBtn_ ) {
    focusBtn_->toggle();
  }
  return;
}

void FocusAction::keyPressEvent( QKeyEvent* e )
{
  if ( false == active_ ) {
    return;
  }
  if ( Qt::Key_Escape != e->key() ) {
    return;
  }
  focusBtn_->toggle();
}

void FocusAction::slotActivate(bool act)
{
  if ( false == hasView() || 0 == parent() || 0 == scene_->document() ) {
    return;
  }
  if ( true == act ) {
    oldcursor_ = scene_->mapview()->cursor();
    scene_->mapview()->setCursor(Qt::CrossCursor);
  }
  else {
    scene_->mapview()->setCursor(oldcursor_);
  }
  active_ = act;
}

void FocusAction::setRotateAngle( double angle )
{
  angle = (angle - scene_->document()->projection()->getMapCenter().lon())*scene_->document()->projection()->hemisphere();
  while ( M_PI < angle ) {
    angle -= M_PI*2;
  }
  while ( -M_PI > angle ) {
    angle += M_PI*2;
  }
  angle *= RAD2DEG;
  scene_->document()->setRotateAngle(angle);
}

}
}
