#ifndef METEO_UKAZ_UI_PLUGINS_DOCSLISTPLUGIN_H
#define METEO_UKAZ_UI_PLUGINS_DOCSLISTPLUGIN_H

#include <qmap.h>

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

class QAction;

namespace meteo {

class DocsListWidget;

class DocsListPlugin : public meteo::app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "docslistplugin" FILE "docslistplugin.json" )

private slots:
  void slotOpenWindow();
  void slotWidgetDeleted();

public:
  DocsListPlugin();
  QWidget* createWidget( meteo::app::MainWindow* mw, const QString& option ) const;

private:
  DocsListWidget *widget_;
  QMap< QAction*,QString > args_;
};

} // meteo

#endif // METEO_UKAZ_UI_PLUGINS_DOCSLISTPLUGIN_H
