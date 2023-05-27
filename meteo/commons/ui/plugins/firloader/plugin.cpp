#include "plugin.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/global/weatherloader.h>

#include "firloader.h"

namespace meteo {
namespace map {

Plugin::Plugin()
  : ActionPlugin("firloaderaction")
{
}

Plugin::~Plugin()
{
}

Action* Plugin::create( MapScene* scene ) const
{
  return new FirAction(scene);
}

FirAction::FirAction( MapScene* scene )
  : Action( scene, "firloaderaction" ),
  widget_(0)
{
  QStringList menulist = QStringList() << QObject::tr("Вид");
  QAction* a = scene_->mapview()->window()->addActionToMenu( QObject::tr("Загрузить РПИ"), menulist );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenFir() ) );
}

FirAction::~FirAction()
{
}

void FirAction::slotOpenFir()
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
    widget_ = new FirLoader( mainwidget );
    QObject::connect( widget_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotFirDestroyed( QObject* ) ) );
  }
  widget_->show();
}

void FirAction::slotFirDestroyed( QObject* o )
{
  if ( o == widget_ ) {
    widget_ = 0;
  }
}

Q_EXPORT_PLUGIN2( firplug, Plugin )

}
}
