#ifndef FrontIdentPlugin_H
#define FrontIdentPlugin_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include "frontidentaction.h"

namespace meteo{
namespace map{

  class FrontIdentPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "frontident" FILE "frontident.json" )
public:
  FrontIdentPlugin();
  ~FrontIdentPlugin(){}
  Action* create( MapScene* sc = 0 ) const;
};

}
}
#endif
