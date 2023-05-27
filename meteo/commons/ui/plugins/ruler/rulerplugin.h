#ifndef RULERPLUGIN_H
#define RULERPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include "ruleraction.h"

namespace meteo{
namespace map{

class RulerPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "ruler" FILE "ruler.json" )
  public:
    RulerPlugin();
    ~RulerPlugin(){}
    Action* create( MapScene* sc = 0 ) const;
};

}
}
#endif
