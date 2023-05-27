#include "plugin.h"

#include <vector>
#include <map>
#include <qaction.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geovector.h>

#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/global/weatherloader.h>

#include "nabludenia.h"

namespace meteo {
namespace map {

Plugin::Plugin()
  : ActionPlugin("nabludeniaction")
{
}

Plugin::~Plugin()
{
}

Action* Plugin::create(MapScene* scene) const
{
  return new NabludeniaAction(scene);
}

NabludeniaAction::NabludeniaAction(MapScene* scene)
  : Action(scene, "nabludeniaction"),
  widget_(nullptr)
{
  internal::WeatherLoader* wl = WeatherLoader::instance();
  if ( nullptr == wl ) {
    error_log << QObject::tr("Загрузчик шаблонов карт не инициализирован. Построение карт погоды невозможно");
    return;
  }
  auto menulist = QList< QPair< QString, QString >>()
    << qMakePair( QObject::tr("deposition"), QObject::tr("Наноска") )
    << qMakePair( QObject::tr("data"), QObject::tr("Данные") );
  const QMap< QString, proto::Map>& types = wl->punchmaps();
  QMapIterator< QString, proto::Map> it(types);
  while ( true == it.hasNext() ) {
    it.next();
    QString id = QString::fromStdString( it.value().name() );
    QString title = QString::fromStdString( it.value().title() );
    QAction* a = scene_->mapview()->window()->addActionToMenu( qMakePair( id, title ), menulist );
    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenNabludenia() ) );
  }
}

NabludeniaAction::~NabludeniaAction()
{
}

void NabludeniaAction::slotOpenNabludenia()
{
  meteo::map::MapWindow* mainwidget = scene_->mapview()->window();
  if ( nullptr == mainwidget ) {
    return;
  }

  QAction* action = qobject_cast<QAction*>(sender());
  if ( nullptr == action ) {
    error_log << QObject::tr("Неизвестная ошибка. Неизвестный отправитель ответа %1")
      .arg(sender()->metaObject()->className());
    return;
  }

  if ( nullptr == widget_ ) {
    widget_ = new Nabludenia(mainwidget);
    QObject::connect(widget_, &Nabludenia::destroyed, this, &NabludeniaAction::slotNabludeniaDestroyed);
  }

  internal::WeatherLoader* wl = WeatherLoader::instance();
  if ( nullptr == wl ) {
    error_log << QObject::tr("Загрузчик шаблонов карт не инициализирован. Построение карт погоды невозможно");
  }

  QString actname = action->text();
  actname.replace("&", "");
  widget_->setCurrentMap(actname);
  widget_->show();
  widget_->showNormal();
}

void NabludeniaAction::slotNabludeniaDestroyed( QObject* o )
{
  if ( o == widget_ ) {
    widget_ = nullptr;
  }
}

}
}
