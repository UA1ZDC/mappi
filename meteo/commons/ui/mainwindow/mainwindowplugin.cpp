#include "mainwindowplugin.h"

#include <qstring.h>

#include <cross-commons/debug/tlog.h>

#include "widgethandler.h"
#include "mainwindow.h"

namespace meteo {
namespace app {

MainWindowPlugin::MainWindowPlugin(const QString& n)
  : QObject(WidgetHandler::instance()->mainwindow())
  , window_(WidgetHandler::instance()->mainwindow())
  , name_(n)
{
}

MainWindowPlugin::~MainWindowPlugin()
{
}

/*!
 * \brief readConf - Чтение конфигурационного файла пункта меню
 * \param fname - Путь к файлу конфигурации
 * \return pair<Строка с конфигурацией, успех чтения>
 */
const QPair<QString, bool> MainWindowPlugin::readConf(const QString& fname)
{
  bool isOk = false;
  QString conf;
  if(QFile::exists(fname)) {
    QFile file(fname);
    if(file.open(QIODevice::ReadOnly)) {
      conf = QString::fromUtf8(file.readAll());
      isOk = true;
    }
  }
  return { conf, isOk };
}

} // app
} // meteo
