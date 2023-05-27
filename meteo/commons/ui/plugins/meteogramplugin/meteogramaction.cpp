#include "meteogramaction.h"

#include <qgraphicsproxywidget.h>
#include <qevent.h>

#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/meteogram/meteogramwindow.h>
#include <meteo/commons/proto/meteo.pb.h>


namespace meteo {

MeteogramAction::MeteogramAction(map::MapScene* scene , map::Document* doc)
  : Action(scene, "meteogram_action" ),
    btn_( new ActionButton ),
    meteogramWindow_(nullptr)
{
  document_ = doc;
  btn_->setToolTip( QObject::tr("Параметры") );
  btn_->setIcon( QIcon(":/meteo/icons/map/settings.png") );
  btn_->toggle();
  QObject::connect( btn_, SIGNAL( toggled(bool) ), this, SLOT( slotShow(bool) ) );
  btnitem_ = scene_->addWidget(btn_);
  btnitem_->setZValue(15000);
  item_ = btnitem_;
}

MeteogramAction::~MeteogramAction()
{
  delete meteogramWindow_;
  meteogramWindow_ = nullptr;
}

void MeteogramAction::slotShow(bool show)
{
  if ( !show ) {
    meteogramWindow_->hide();
  }
  else {
    if ( nullptr == meteogramWindow_ ) {
      meteogramWindow_ = new MeteogramWindow(view()->window(), document_);
      meteogramWindow_->installEventFilter(this);
    }
    meteogramWindow_->showNormal();
  }
}
/*
void MeteogramAction::addActionsToMenu( map::Menu* menu ) const
{
  if(nullptr == menu ||nullptr == scene_
    || nullptr == scene_->document()
    || nullptr == scene_->document()->activeLayer()
    ||nullptr == view()) {
    return;
  }
  map::Document* d = scene_->document();
  map::Layer* l = d->activeLayer();
  if ( nullptr == l ) {
    error_log << QObject::tr("Активный слой не установлен.");
    return;
  }

  switch (l->type()) {
    case map::proto::kLayerPunch:{
      d->setSearchDistance(100);
      QPoint mousepos(0,0) ;//= e->pos();
      mousepos = view()->mapToScene(mousepos).toPoint();
      map::Puanson* p = d->objectNearScreenPoint<map::Puanson>( mousepos, l );
      if ( 0 == p ) {
        d->restoreSearchDistance();
        return;
      }
      //showPunchMeteodata(p);
      d->restoreSearchDistance();}
      break;
    case map::proto::kLayerIso:
     if ( true != l->hasField() ) {
        return ;
      }
      break;
    default:
      return;
    }

  QMenu* m =  menu->addMenu(QObject::tr("Анализ"));
  if(nullptr != m ){
    m->addSeparator();
    m->addAction("Метеограмма", this, SLOT(slotShow()));
    menu->addAnalyseAction(m->menuAction());
  }
}
*/

bool MeteogramAction::eventFilter(QObject* obj, QEvent* event)
{
  Q_UNUSED(obj);
  if (QEvent::Hide == event->type()) {
    if (btn_->isChecked()) {
      btn_->blockSignals(true);
      btn_->toggle();
      btn_->blockSignals(false);
    }
  }
  return false;
}

} //meteo
