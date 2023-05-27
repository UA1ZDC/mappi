#include "plugin.h"

#include <qmetaobject.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/docoptions.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include "termselect.h"
#include "docdlg.h"
#include "ui_blankparams.h"

namespace meteo {
namespace map {
namespace weather {

/*!
 * \brief kMenuPath - Путь к пункту меню { id: "document", title: "Документ"} ->
 * Для добавления вложенности нужно добавить пару в список
*/
static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("document", QObject::tr("Документ"))
});
static const auto kMenuMapsPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("document", QObject::tr("Документ")),
    //  QPair<QString, QString>("maps", QObject::tr("Карты"))
});
static const auto kMenuBaricPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("document", QObject::tr("Документ")),
});
static const auto kMenuSnowPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("document", QObject::tr("Документ")),
});

Plugin::Plugin()
  : MainWindowPlugin("weatherplugin")
{
  internal::WeatherLoader* wl = WeatherLoader::instance();
  if(nullptr == wl) {
    error_log << QObject::tr("Загрузчик шаблонов карт не инициализирован. Построение карт погоды невозможно");
    return;
  }
  {
    QAction* action = mainwindow()->findAction("emptymap");
    if(nullptr != action) {
      action->setDisabled(false);
      QObject::connect(action, &QAction::triggered, this, &Plugin::slotOpenEmptyMap);
    }
  }

  {
    QAction* action = mainwindow()->findAction("emptymapparams");
    if(nullptr != action) {
      QObject::connect(action, &QAction::triggered, this, &Plugin::slotOpenBlankParam);
    }
  }

  {
    QAction* action = mainwindow()->findAction("mapopen");
    if ( nullptr != action ){
      QObject::connect(action, &QAction::triggered, this, &Plugin::slotOpenMapFile );
    }
  }
}

Plugin::~Plugin()
{
}

void Plugin::slotOpenMapFile()
{
  auto mapWindow = createMapWindow();

  auto action = mapWindow->findActionInMenu("opendoc");
  if ( nullptr == action) {
    warning_log.msgBox() << QObject::tr("Не удается найти действие 'Сохранить' на карте. Проверьте его наличие в меню Документ карты. ");
    return;
  }
  action->trigger();
}

MapWindow* Plugin::createMapWindow()
{  
  meteo::map::proto::Document blank = meteo::global::lastBlankParams(proto::kGeoMap);  
  QString loaderName = "geo.old";
  if (blank.has_geoloader()) {
    loaderName = QString::fromStdString(blank.geoloader());
  }
  MapWindow* window = new MapWindow( mainwindow(), blank, loaderName, MnCommon::etcPath() + "/document.menu.conf" );

  window->addPluginGroup("meteo.map.common");
  window->addPluginGroup("meteo.map.weather");
  mainwindow()->toMdi(window);
  return window;
}

void Plugin::slotOpenTermSelect()
{
  QAction* a = qobject_cast<QAction*>(sender());
  if(nullptr == a) {
    error_log << QObject::tr("Неизвестная ошибка. Неизвестный отправитель ответа %1")
      .arg( sender()->metaObject()->className() );
    return;
  }
  MapWindow* window = createMapWindow();
  a = window->menuActionByName( a->objectName() );
  if (nullptr != a) {
    a->trigger();
  }
}

void Plugin::slotOpenEmptyMap()
{
  auto blank = meteo::global::lastBlankParams(proto::kGeoMap);
  meteo::global::saveLastBlankParams(blank);
  createMapWindow();
}

void Plugin::slotOpenBlankParam()
{
  QDialog* dlg = new DocDlg( mainwindow() );
  Ui::BlankParams ui;
  ui.setupUi(dlg);
  ui.blankedit->setDoc( meteo::global::lastBlankParams(proto::kGeoMap) );
  ui.blankedit->loadMapSettings();
  int res = dlg->exec();
  meteo::map::proto::Document opt = ui.blankedit->doc();
  if ( QDialog::Accepted != res ) {
    delete dlg;
    return;
  }
  ui.blankedit->saveMapSettings();  
  meteo::global::saveLastBlankParams(opt);
  delete dlg;
  slotOpenEmptyMap();
}

MenuSlot::MenuSlot( MapWindow* window )
  : QObject(window),
  window_(window),
  term_(nullptr)
{
}

MenuSlot::~MenuSlot()
{
  delete term_; term_ = nullptr;
}

void MenuSlot::slotOpenTermSelect()
{
  QAction* a = qobject_cast<QAction*>(sender());
  if (nullptr == a) {
    error_log << QObject::tr("Неизвестная ошибка. Неизвестный отправитель ответа %1")
      .arg(sender()->metaObject()->className());
    return;
  }
  internal::WeatherLoader* wl = WeatherLoader::instance();
  if (nullptr == wl) {
    error_log << QObject::tr("Загрузчик шаблонов карт не инициализирован. Построение карт погоды невозможно");
    return;
  }

  if (nullptr == term_) {
    term_ = new TermSelect( wl->weathermaps(), window_ );
  }
  QString title = a->text().remove('&');
  term_->setCurrentMapType(title);
  term_->showNormal();
}




}
}
}
