#ifndef MAPPI_UI_PLUGINS_SESSIONVIEWERPLUGIN_SESSIONVIEWERWIDGET_H
#define MAPPI_UI_PLUGINS_SESSIONVIEWERPLUGIN_SESSIONVIEWERWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/layer.h>

namespace Ui {
class SessionViewerWidget;
}

namespace meteo {
namespace map {

class ThematicListWidget;

class SessionViewerWidget : public QWidget
{
  Q_OBJECT

public:
  explicit SessionViewerWidget(QWidget *parent = 0);
  virtual ~SessionViewerWidget();

private:
  void drawDelim();

  meteo::map::proto::Document initMap();

private:
  Ui::SessionViewerWidget* ui_;
  meteo::map::MapWindow* map_;
  meteo::map::Layer* delimLayer_;
  QStringList channels_;
};

} //map
} //meteo

#endif // MAPPI_UI_PLUGINS_SESSIONVIEWERPLUGIN_SESSIONVIEWERWIDGET_H
