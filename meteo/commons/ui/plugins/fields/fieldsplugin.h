#ifndef FIELDSPLUGIN_H
#define FIELDSPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include "fieldsaction.h"

namespace meteo{
namespace map{

class FieldsPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "fieldsplug" FILE "fieldsplug.json" )
  public:
    FieldsPlugin();
    ~FieldsPlugin(){}
    Action* create( MapScene* sc = 0 ) const;
};

}
}
#endif // FIELDSPLUGIN_H
