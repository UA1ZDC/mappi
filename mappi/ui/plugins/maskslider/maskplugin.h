#ifndef MASKPLUGIN_H
#define MASKPLUGIN_H


#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class MaskPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "maskaction" FILE "maskplugin.json" )
public:
  MaskPlugin() : ActionPlugin("maskaction") {}
  virtual ~MaskPlugin () {}

  Action* create( MapScene* sc = 0 ) const;
};

} //map
} //meteo

#endif // MASKPLUGIN_H
