#include "undoredo.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace map {

AddObjectCommand::AddObjectCommand(const QString& layerUuid, Object* obj, MapScene* scene)
{
  obj_ = obj;
  scene_ = scene;
  layerUuid_ = layerUuid;

  obj_->setVisible(false);
}

AddObjectCommand::~AddObjectCommand()
{
  delete obj_;
}

void AddObjectCommand::undo()
{
  Layer* l = scene_->document()->layerByUuid(layerUuid_);
  if ( 0 == l ) {
    return;
  }

  Object* o = l->objectByUuid(objectUuid_);
  if ( 0 == o ) {
    return;
  }

  delete o;
}

void AddObjectCommand::redo()
{
  Layer* l = scene_->document()->layerByUuid(layerUuid_);

  if ( 0 == l ) {
    return;
  }

  Object* o = obj_->copy(l);
  o->setVisible(true);
  objectUuid_ = o->uuid();
}

} // map
} // meteo
