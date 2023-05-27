#ifndef METEO_COMMONS_UI_PLUGINS_RENAMEDOC_H
#define METEO_COMMONS_UI_PLUGINS_RENAMEDOC_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class Plugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "renamedoc" FILE "renamedoc.json" )
  public:
    Plugin();
    ~Plugin();

    Action* create( MapScene* scene ) const ;
};

class RenameDoc : public Action
{
  Q_OBJECT
  public:
    RenameDoc( MapScene* scene );
    ~RenameDoc();

  private slots:
    void slotRenameDoc();
};

}
}

#endif
