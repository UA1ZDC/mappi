#ifndef MSGVIEWERPLUGIN_H
#define MSGVIEWERPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/msgviewer/msgviewer.h>

class PluginHandler: public QObject
{
  Q_OBJECT

private slots:
  void slotOpenWindow();
  void slotWidgetDeleted();

public:
  PluginHandler(meteo::app::MainWindow* mw);
  ~PluginHandler();
  QWidget* createWidget(meteo::app::MainWindow* mw, const QString& option);

private:
  meteo::app::MainWindow* mw_;
  meteo::MsgViewer* widget_;

};

#endif // MSGVIEWERPLUGIN_H
