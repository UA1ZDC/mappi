#ifndef METEO_COMMONS_UI_DOCUMENTVIEWER_PLUGINHANDLER_H
#define METEO_COMMONS_UI_DOCUMENTVIEWER_PLUGINHANDLER_H

#include <qobject.h>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

#include <meteo/commons/ui/documentviewer/wgtdocview/wgtdocview.h>

namespace meteo {
namespace documentviewer {

class DocsListWidget;

class PluginHandler : public QObject
{
  Q_OBJECT

public:
  PluginHandler(meteo::app::MainWindow* mw);
  virtual ~PluginHandler();

private slots:
  void slotOpenFaxWindow();
  void slotWidgetDesctroyed();
  void slotOpenLocalDocsWindow();
  void slotOpenSateliteImagesWindow();
  void slotOpenMapImages();
  void slotOpenFormalDocuments();

private:
  meteo::app::MainWindow* mw_;
  WgtDocView *faxWidget_;
  WgtDocView *localDocWidget_;
  WgtDocView *sateliteImageWidget_;
  WgtDocView *mapImageWidget_;
};

}
}

#endif
