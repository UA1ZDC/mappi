#include "plugin.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/global/weatherloader.h>

#include <qaction.h>

#include "radarmap.h"

namespace meteo {
namespace map {

Plugin::Plugin()
  : ActionPlugin("radaraction")
{
}

Plugin::~Plugin()
{
}

Action* Plugin::create( MapScene* scene ) const
{
  return new RadarAction(scene);
}

RadarAction::RadarAction( MapScene* scene )
  : Action( scene, "radaraction" ),
  widget_(0)
{
  internal::WeatherLoader* wl = WeatherLoader::instance();
  if ( 0 == wl ) {
    error_log << QObject::tr("Загрузчик шаблонов карт не инициализирован. Построение карт погоды невозможно");
    return;
  }
  auto menulist = QList<QPair<QString, QString>>
      ({QPair<QString, QString>("deposition", QObject::tr("Наноска")),
        QPair<QString, QString>("radiolocdata", QObject::tr("Радиолокационные данные"))});
  const QMap< QString, proto::RadarColor>& types = wl->radarlibrary();
  QMapIterator< QString, proto::RadarColor> it(types);
  while ( true == it.hasNext() ) {
    it.next();
    auto id = QString("faxaction%1").arg(it.value().descr());
    QString title = QString::fromStdString( it.value().name() );
    QAction* a = scene_->mapview()->window()->addActionToMenu( {id, title}, menulist );
    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenRadar() ) );
  }
}

RadarAction::~RadarAction()
{
}

void RadarAction::slotOpenRadar()
{
  meteo::map::MapWindow* mainwidget = scene_->mapview()->window();
  if ( 0 == mainwidget ) {
    return;
  }
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( 0 == a ) {
    error_log << QObject::tr("Неизвестная ошибка. Неизвестный отправитель ответа %1")
      .arg( sender()->metaObject()->className() );
    return;
  }
  if ( 0 == widget_ ) {
    widget_ = new RadarMap( mainwidget );
    QObject::connect( widget_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotRadarDestroyed( QObject* ) ) );
  }
  widget_->setCurrentMap(a->text());
  widget_->show();
}

void RadarAction::slotRadarDestroyed( QObject* o )
{
  if ( o == widget_ ) {
    widget_ = 0;
  }
}

}
}
