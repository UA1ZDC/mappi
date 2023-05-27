#ifndef TRANSPARENCYPLUGIN_H
#define TRANSPARENCYPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class TransparencyPlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "transparencyaction" FILE "transparencyplugin.json" )
public:
  TransparencyPlugin(): ActionPlugin("transparencyaction") {}
  virtual ~TransparencyPlugin() {}

  Action* create( MapScene* sc = 0) const;
};

} //map
} //meteo

#endif // TRANSPARENCYPLUGIN_H
