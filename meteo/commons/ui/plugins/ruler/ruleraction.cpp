#include "ruleraction.h"

#include <qgraphicsproxywidget.h>

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/map/view/menu.h>

namespace meteo {
namespace map {

RulerAction ::RulerAction ( MapScene* scene )
  : TraceAction(scene, "ruleaction" ),
    btn_( new ActionButton )
{
  btn_->setToolTip( QObject::tr("Линейка") );
  btn_->setIcon( QIcon(":/meteo/icons/map/ruler.png") );
  item_ = scene_->addWidget(btn_);
  item_->setZValue(10001);
  scene->addAction(this, kTopLeft, QPoint(0,1));
//  setHasCorner(true);
//  setCorner(kBottomLeft);
//  setCornerAddress(QPoint( 0,1 ));
}

void RulerAction::addActionsToMenu( Menu* menu ) const
{
  if(0 == menu ) return;
  if( !menu->isEmpty()){  menu->addSeparator();  }

  if( false == traces_.contains(current_) ){
    return;
  }

  if( traces_[current_].count() <= 0){
    return;
  }

  QAction* act = menu->addAction("Удалить линейку");
  connect( act, SIGNAL(triggered()), SLOT(slotRemoveTrace()) );
  TraceAction::addActionsToMenu(menu);
}

TraceItem*RulerAction::addNode(const GeoPoint& gp, bool reverse,
                               TraceItem::CollideItemOption collide)
{
  TraceItem* item = TraceAction::addNode(gp, reverse, collide);
  if( 0 != item ){
    item->updateDistance();
  }
  return item;
}

TraceItem*RulerAction::insertNode(const GeoPoint& gp, TraceItem* after, TraceItem* before,
                                  TraceItem::CollideItemOption collide)
{
  TraceItem* item = TraceAction::insertNode(gp, after, before, collide);
  if( 0 != item ){
    item->updateDistance();
  }
  return item;
}

void RulerAction::mouseReleaseEvent(QMouseEvent* e)
{
  if( true == btn_->isChecked() ){
    activate();
  }else{
    deactivate();
  }
  TraceAction::mouseReleaseEvent(e);
}

bool RulerAction::eventFilter( QObject* o, QEvent* e )
{
  return TraceAction::eventFilter( o, e );;
}

}
}
