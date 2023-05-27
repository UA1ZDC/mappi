#ifndef ProcIdentPlugin_H
#define ProcIdentPlugin_H

#include "procidentaction.h"

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class ProcIdentPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "procidentaction" FILE "procidentaction.json" )
  public:
    ProcIdentPlugin();
    ~ProcIdentPlugin();
    Action* create( MapScene* sc = nullptr ) const;
};

}
}

#endif
