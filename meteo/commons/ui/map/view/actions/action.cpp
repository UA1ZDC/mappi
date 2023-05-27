#include "action.h"

#include "../mapview.h"
#include "../mapscene.h"
#include "../menu.h"

#include <cross-commons/debug/tlog.h>


namespace meteo {
namespace map {

Action::Action(MapScene* scene, const QString& n )
  : QObject(scene),
  scene_(scene),
  wgt_(nullptr),
  item_(nullptr),
  name_(n),
  cellsize_(34),
  size_( QSize(1,1) ),
  hasaddress_(false),
  is_deleteble_(true)
{
}

Action::~Action()
{
  if(nullptr != scene_){
  scene_->removeAction(this);
  }
}

void Action::setScene( MapScene* sc ) {
  scene_ = sc;
}


void Action::addActionsToMenu( Menu* menu ) const
{
  addActionsToMenu( reinterpret_cast<QMenu*>(menu) );
}

void Action::addActionsToMenu( QMenu* menu ) const
{
  if ( nullptr == menu ) {
    error_log << QObject::tr("Нулевой указатель на меню");
    return;
  }
  for ( int i = 0, sz = actions_.size(); i < sz; ++i ) {
    menu->addAction(actions_[i]);
  }
}

bool Action::hasView() const
{
  if ( nullptr == scene_ || nullptr == scene_->mapview() ) {
    return false;
  }
  return true;
}

MapView* Action::view() const
{
  if ( nullptr == scene_ ) {
    return nullptr;
  }
  return scene_->mapview();
}

void Action::removeFromScene()
{
  if (nullptr == scene_) {
    return;
  }
  scene_->removeAction(this);
}


}
}
