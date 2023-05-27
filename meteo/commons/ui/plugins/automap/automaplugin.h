#ifndef AUTOMAPLUGIN_H
#define AUTOMAPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/automap/automap.h>

namespace meteo{
namespace app {

class AutoMapPlugin : public MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "automap" FILE "automap.json" )
  public:
    AutoMapPlugin();
    ~AutoMapPlugin();

private:
    meteo::map::AutoMap* wgt_;

  private slots:
    void slotOpenAutomap();
    void slotWgtDel();
};

}
}
#endif

