#ifndef TestPlugin_H
#define TestPlugin_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include "synopaction.h"

namespace meteo{
namespace map{

class SynopPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "synopaction" FILE "synopaction.json" )
  public:
    SynopPlugin();
    ~SynopPlugin(){}
    Action* create( MapScene* sc = nullptr ) const;
};

}
}
#endif // TestPlugin_H
