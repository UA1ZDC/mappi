#ifndef ReobanalPlugin_H
#define ReobanalPlugin_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include "reobanalaction.h"

namespace meteo{
namespace map{

  class ReobanalPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "reobanal" FILE "reobanal.json" )
public:
  ReobanalPlugin();
  ~ReobanalPlugin(){}
  Action* create( MapScene* sc = nullptr ) const;
};

}
}
#endif
