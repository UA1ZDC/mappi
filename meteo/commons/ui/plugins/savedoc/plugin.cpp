#include "plugin.h"

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/global/weatherloader.h>

#include <qaction.h>

#include "savedoc.h"

namespace meteo {
namespace map {

Plugin::Plugin()
  : ActionPlugin("savedocaction")
{
}

Plugin::~Plugin()
{
}

Action* Plugin::create( MapScene* scene ) const
{
  return new SaveDocAction(scene);
}

SaveDocAction::SaveDocAction( MapScene* scene )
  : Action( scene, "savedocaction" ),
  widget_(0)
{
  QAction* a = scene_->mapview()->window()->addActionToMenu({"savedoc", QObject::tr("Сохранение документа")});
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenSaveDoc() ) );
}

SaveDocAction::~SaveDocAction()
{
}

void SaveDocAction::slotOpenSaveDoc()
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
    widget_ = new SaveDoc(mainwidget);
    QObject::connect( widget_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotSaveDocDestroyed( QObject* ) ) );
  }
  widget_->updateLayers();
  widget_->show();
  widget_->showNormal();
}

void SaveDocAction::slotSaveDocDestroyed( QObject* o )
{
  if ( o == widget_ ) {
    widget_ = 0;
  }
}

}
}
