#include "plugin.h"

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/global/weatherloader.h>

#include <qaction.h>

#include "savedoc.h"
#include "opendoc.h"

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
  savedoc_(0),
  opendoc_(0)
{
  auto menuPath = QList<QPair<QString, QString>>
      ({QPair<QString, QString>("document", QObject::tr("Документ"))});

  auto title = QPair<QString, QString>("save", QObject::tr("Сохранить"));
  QAction* a = scene_->mapview()->window()->addActionToMenu(title, menuPath);
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenSaveDoc() ) );

  title = QPair<QString, QString>("opendoc", QObject::tr("Открыть"));
  a = scene_->mapview()->window()->addActionToMenu(title, menuPath);
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenOpenDoc() ) );

  title = QPair<QString, QString>("closedoc", QObject::tr("Закрыть"));
  a = scene_->mapview()->window()->addActionToMenu(title, menuPath);
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotCloseMapWindow() ) );
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
  if ( 0 == savedoc_ ) {
    savedoc_ = new SaveDoc(mainwidget);
    QObject::connect( savedoc_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotSaveDocDestroyed( QObject* ) ) );
  }
  savedoc_->updateLayers();
  savedoc_->show();
  savedoc_->showNormal();
}

void SaveDocAction::slotSaveDocDestroyed( QObject* o )
{
  if ( o == savedoc_ ) {
    savedoc_ = 0;
  }
}

void SaveDocAction::slotOpenOpenDoc()
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
  if ( 0 == opendoc_ ) {
    opendoc_ = new OpenDoc(mainwidget);
    QObject::connect( opendoc_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotOpenDocDestroyed( QObject* ) ) );
  }
  opendoc_->show();
  opendoc_->showNormal();
}

void SaveDocAction::slotOpenDocDestroyed( QObject* o )
{
  if ( o == opendoc_ ) {
    opendoc_ = 0;
  }
}

void SaveDocAction::slotCloseMapWindow()
{
  meteo::map::MapWindow* mainwidget = scene_->mapview()->window();
  if ( 0 != mainwidget ) {
    mainwidget->close();
  }
}

}
}
