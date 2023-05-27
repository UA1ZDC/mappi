#ifndef METEO_COMMONS_UI_PLUGINS_ALPHANUMGENPLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_ALPHANUMGENPLUGIN_H

#include <qdialog.h>

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

namespace meteo
{
  class AlphanumgenPlugin : public app::MainWindowPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "alphanumgen" FILE "alphanumgen.json" )
    public :
    AlphanumgenPlugin();
    ~AlphanumgenPlugin();

    public slots :
    void createKn01Dialog();
    void createKn03Dialog();
    void createKn04Dialog();
  };
}


#endif
