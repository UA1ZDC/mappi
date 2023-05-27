#ifndef MONTAGEPLUGIN_H
#define MONTAGEPLUGIN_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class MontagePlugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "montageaction" FILE "montageplugin.json" )
public:
  MontagePlugin() : ActionPlugin("montageaction") {}
  virtual ~MontagePlugin () {}

  Action* create( MapScene* sc = 0 ) const;
};

} //map
} //meteo

#endif // MONTAGEPLUGIN_H
