#include "msgviewerplugin.h"
#include <meteo/commons/ui/mainwindow/mainwindow.h>

static const auto kMenuPath = QList<QPair<QString, QString>>(
{
  QPair<QString, QString>("gmi", QObject::tr("ГМИ"))
});

MsgViewerPlugin::MsgViewerPlugin():
  meteo::app::MainWindowPlugin("msgviewerplugin"),
  handler_(nullptr) {

  mainwindow()->addActionToMenu({"watchgmi", QObject::tr("Просмотр ГМИ")}, kMenuPath);

  handler_ = new PluginHandler(mainwindow());
}

MsgViewerPlugin::~MsgViewerPlugin()
{

}


QWidget* MsgViewerPlugin::createWidget(meteo::app::MainWindow* mw, const QString& option)
{
  if ( nullptr == handler_ ) {
    return nullptr;
  }
  return handler_->createWidget(mw, option);
}
