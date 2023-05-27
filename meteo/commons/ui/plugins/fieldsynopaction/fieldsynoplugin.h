#ifndef FIELDSYNOPLUGIN_H
#define FIELDSYNOPLUGIN_H
#include <meteo/commons/ui/map/view/actions/action.h>
#include "fieldsynopaction.h"

namespace meteo{
namespace map{

class FieldSynoPlugin : public ActionPlugin
{
  Q_OBJECT
  public:
    FieldSynoPlugin();
    ~FieldSynoPlugin();
    Action* create( MapScene* sc = 0 ) const;
};

}
}
#endif // FIELDSYNOPLUGIN_H
