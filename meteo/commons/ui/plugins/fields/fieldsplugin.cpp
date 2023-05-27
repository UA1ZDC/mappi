#include "fieldsplugin.h"

namespace meteo{
namespace map{

FieldsPlugin::FieldsPlugin() : ActionPlugin("fieldsaction")
{
}

Action*FieldsPlugin::create(MapScene* sc) const
{
  return new FieldsAction(sc);
}

}
}
