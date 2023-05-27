#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_UHDWINDOW_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_UHDWINDOW_H

#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/meteomenu.pb.h>

#include <QKeyEvent>
#include <qevent.h>
#include <qmainwindow.h>

class UhdWindow : public meteo::map::MapWindow
{
  Q_OBJECT

public:
  UhdWindow(meteo::app::MainWindow *window, const meteo::map::proto::Document& blank);
  virtual ~UhdWindow(){}
  void closeWindow();
  void setFlExit(bool flag);

private:
  bool flExit_=false;
  void keyPressEvent(QKeyEvent* event);
  void closeEvent(QCloseEvent* ev);
};

#endif // MAPPI_UI_PLUGINS_UHDCONTROL_UHDWINDOW_H
