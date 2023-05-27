#ifndef SESSIONVIEWERPLUGIN_H
#define SESSIONVIEWERPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include "sessionviewerwidget.h"

namespace meteo {
  namespace map {
    class Document;
  } // map
} //meteo

namespace meteo {
namespace map {


class SessionViewerPlugin: public meteo::app::MainWindowPlugin
{

  Q_OBJECT
Q_PLUGIN_METADATA( IID "sessionviewerplugin" FILE "sessionviewerplugin.json" )

public:
  SessionViewerPlugin();
  ~SessionViewerPlugin();
  QWidget* createWidget( meteo::app::MainWindow* mw, const QString& option = "");

private slots:
  void slotOpenWindow();
  void slotWidgetDeleted();

private:
  void initMap();

  SessionViewerWidget* widget_;
};

} //map
} //meteo

#endif // SESSIONVIEWERPLUGIN_H
