#include "verticalcutaction.h"

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/actions/arrow.h>

namespace meteo {
namespace map {

const QString VerticalCutAction::kName = QObject::tr("vertical_cut_action");

VerticalCutAction::VerticalCutAction(MapScene* scene)
  : TraceAction(scene, kName)
{
  connect( this, SIGNAL(itemAdded(int,GeoPoint)), SLOT(slotItemAdded(int,GeoPoint)) );
}

VerticalCutAction::~VerticalCutAction()
{
}

void VerticalCutAction::addActionsToMenu(Menu* menu) const
{
  if ( nullptr == menu ) {
    return;
  }
  if ( false == traces_.contains(current_) ) {
    return;
  }
  if ( traces_[current_].count() <= 0 ) {
    return;
  }

  if ( !menu->isEmpty() ) {
    menu->addSeparator();
  }

  QAction* act = menu->addAction("Удалить траекторию");
  connect( act, SIGNAL(triggered()), SLOT(slotRemoveTrace()) );
  TraceAction::addActionsToMenu(menu);
}

void VerticalCutAction::appendNode(const GeoPoint& point, const QString& title, bool move)
{
  TraceItem* item  = addNode(point);
  item->updateText(title);
  item->setFlag(QGraphicsItem::ItemIsMovable, move);
  item->setShowDistance(false);
}

void VerticalCutAction::clearTrace() {
  slotRemoveTrace();
}

void VerticalCutAction::slotRequestCoord(bool enable)
{
  if ( !hasView() ) { return; }

  if ( enable ) {
    activate();
  }
  else {
    slotRemoveTrace();
    deactivate();
  }
}

void VerticalCutAction::slotRemovePath()
{
  slotRemoveTrace();
}

void VerticalCutAction::slotItemAdded(int num, const GeoPoint& point)
{
  Q_UNUSED( point );

  if ( num >= traces_[current_].size() ) {
    return;
  }

  traces_[current_][num]->setShowDistance(false);
}

void VerticalCutAction::slotPlaceChanged(int num, const GeoPoint& point, const QString& text)
{
  if ( num >= traces_[current_].size() ) {
    return;
  }
  traces_[current_][num]->setGeoPoint(point);
  traces_[current_][num]->updateText(text);
  for ( auto arrow : traces_[current_][num]->arrows() ){
    if ( nullptr == arrow ) {
      continue;
    }
    arrow->updatePosition();
    //arrow->update();
  }
}

} // map
} // meteo
