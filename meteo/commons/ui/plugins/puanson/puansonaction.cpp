#include "puansonaction.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qdebug.h>
#include <qplugin.h>
#include <qcursor.h>
#include <qmenu.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/layeritems.h>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/menu.h>

#include "puansonview.h"
#include "puansonitem.h"

namespace meteo {
namespace map {

PuansonAction::PuansonAction( MapScene* scene )
  : Action(scene, "puansonaction" ),
    view_(0),
    peep_(false),
    value_(false),
    peepaction_( new QAction(this) ),
    leaveaction_( new QAction( QObject::tr("Убрать все"), this ) ),
    peepunchaction_( new QAction(this) ),
    peepinfo_( new QAction( QObject::tr("Показать данные"), this ) ),
    //value_action_( new QAction(this) ),
    currentpunch_(0),
    foremove_(0)
{
  QObject::connect( peepaction_, SIGNAL( triggered() ), this, SLOT( slotPeep() ) );
  QObject::connect( peepunchaction_, SIGNAL( triggered() ), this, SLOT( slotPeepPunch() ) );
  QObject::connect( leaveaction_, SIGNAL( triggered() ), this, SLOT( slotPeepLeave() ) );
  QObject::connect( peepinfo_, SIGNAL( triggered() ), this, SLOT( slotShowPeepInfo() ) );
 // QObject::connect( value_action_, SIGNAL( triggered() ), this, SLOT( slotShowValue() ) );
  scene_->document()->eventHandler()->installEventFilter(this);
}

PuansonAction::~PuansonAction()
{
  delete peepinfo_; peepinfo_ = 0;
  delete peepunchaction_; peepunchaction_ = 0;
  delete leaveaction_; leaveaction_ = 0;
  delete peepaction_; peepaction_ = 0;
  delete view_; view_ = 0;
}

void PuansonAction::mouseDoubleClickEvent( QMouseEvent* e )
{
  if ( 0 == scene_->document() ) {
    error_log << QObject::tr("Сцена не установлена.");
    return;
  }
  if ( 0 == view() ) {
    error_log << QObject::tr("Сцена не установлена.");
    return;
  }
  Document* d = scene_->document();
  if ( 0 == d->activeLayer() ) {
    error_log << QObject::tr("Активный слой не установлен.");
    return;
  }
  Layer* l = d->activeLayer();
  if ( kLayerPunch != l->type() ) {
    return;
  }

  d->setSearchDistance(100);
  QPoint mousepos = view()->mapToScene(e->pos()).toPoint();
  Puanson* p = nullptr;
  QList<Puanson*> plist;
  if(true == value_)
  {
   plist = d->objectsNearScreenPoint<Puanson>( mousepos, l );
  }
  else
  {
   p = d->objectNearScreenPoint<Puanson>( mousepos, l );
  }
  if ( nullptr == p ) {
    d->restoreSearchDistance();
    return;
  }

  showPunchMeteodata(p);
  d->restoreSearchDistance();
  e->accept();
}

void PuansonAction::mouseMoveEvent( QMouseEvent* e )
{
  QPoint mousepos = view()->mapToScene(e->pos()).toPoint();
  for ( int i = 0, sz = peeps_.size(); i < sz; ++i ) {
    if ( true == peeps_[i]->boundingRect().contains(mousepos) ) {
      return;
    }
  }
  if ( false == peep_ ) {
    return;
  }
  if ( 0 == scene_->document() ) {
    error_log << QObject::tr("Сцена не установлена.");
    return;
  }
  if ( 0 == view() ) {
    error_log << QObject::tr("Сцена не установлена.");
    return;
  }
  Document* d = scene_->document();
  if ( 0 == d->activeLayer() ) {
    error_log << QObject::tr("Активный слой не установлен.");
    return;
  }
  Layer* l = d->activeLayer();
  if ( kLayerPunch != l->type() ) {
    return;
  }

  d->setSearchDistance(30);
  Puanson* p = d->objectNearScreenPoint<Puanson>( mousepos, l, 0, false );
  if ( 0 == p ) {
    if ( 0 != currentpunch_ ) {
      delete currentpunch_; currentpunch_ = 0;
    }
    d->restoreSearchDistance();
    return;
  }
  peepPuanson(p);
  d->restoreSearchDistance();
  e->accept();
}

void PuansonAction::addActionsToMenu( Menu* menu ) const
{
  Document* d = scene_->document();
  if ( 0 == d ) {
    return;
  }
  if ( 0 == d->activeLayer() ) {
    error_log << QObject::tr("Активный слой не установлен.");
    return;
  }
  Layer* l = d->activeLayer();
  if ( kLayerPunch != l->type() ) {
    return;
  }
  if ( false == peep_ ) {
    peepaction_->setText( QObject::tr("Подъём наноски") );
  }
  else {
    peepaction_->setText( QObject::tr("Отключить подъём наноски") );
  }
  /*if ( false == value_ ) {
    value_action_->setText( QObject::tr("Значение под курсором") );
  }
  else {
    value_action_->setText( QObject::tr("Отключить Значение под курсором") );
  }
  menu->addLayerAction(value_action_);
  */

  menu->addLayerAction(peepaction_);
  if ( 0 != currentpunch_ ) {
    peepunchaction_->setText( QObject::tr("Оставить поднятым") );
    menu->addObjectAction(peepunchaction_);
    foremove_ = 0;
  }
  else {
    QPoint mousepos = scene_->mapview()->mapToScene( scene_->mapview()->mapFromGlobal( QCursor::pos() ) ).toPoint();
    for ( int i = 0, sz = peeps_.size(); i < sz; ++i ) {
      if ( true == peeps_[i]->boundingRect().contains(mousepos) ) {
        peepunchaction_->setText( QObject::tr("Убрать") );
        menu->addObjectAction(peepinfo_);
        menu->addObjectAction(peepunchaction_);
        foremove_ = peeps_[i];
        break;
      }
    }
  }

  int cnt = 0;
  for ( int i = 0, sz = peeps_.size(); i < sz; ++i ) {
    if ( l == peeps_[i]->punch()->layer() ) {
      ++cnt;
    }
  }
  if ( 1 < cnt ) {
    menu->addLayerAction(leaveaction_);
  }
}

bool PuansonAction::eventFilter( QObject* o, QEvent* e )
{
  if ( o == scene_->document()->eventHandler() && LayerEvent::LayerChanged  == e->type() ) {
    LayerEvent* ev = reinterpret_cast<LayerEvent*>(e);
    if ( LayerEvent::Deleted == ev->changeType() ) {
      if ( 0 == scene_->document()->itemsLayer() || ev->layer() == scene_->document()->itemsLayer()->uuid() ) {
        peeps_.clear();
      }
      else  {
        QList<PuansonItem*> list;
        for ( int i = 0, sz = peeps_.size(); i < sz; ++i ) {
          PuansonItem* item = peeps_[i];
          if ( 0 == item->punch() || 0 == item->punch()->layer() || item->punch()->layer()->uuid() == ev->layer() ) {
            list.append(peeps_[i]);
          }
        }
        for ( int i = 0, sz = list.size(); i < sz; ++i ) {
          delete list[i];
          peeps_.removeAll(list[i]);
        }
      }
    }
  }
  return false;
}

void PuansonAction::showPunchMeteodata( Puanson* puanson )
{
  if ( 0 == puanson ) {
    return;
  }

  if ( 0 == view_ ) {
    view_ = new PuansonView( scene_->mapview()->window() );
  }
  view_->setPuanson(puanson);
  view_->show();
}

void PuansonAction::peepPuanson( Puanson* punch )
{
  if ( false == peep_ ) {
    return;
  }
  Document* document = scene_->document();
  if ( 0 == document ) {
    return;
  }
  if ( 0 != currentpunch_ ) {
    document->removeItem(currentpunch_);
    delete currentpunch_;
    currentpunch_ = 0;
  }
  currentpunch_ = new PuansonItem( punch, scene_ );
}

void PuansonAction::slotPeep()
{
  if ( false == peep_ ) {
    peep_ = true;
  }
  else {
    peep_ = false;
    if ( 0 != currentpunch_ ) {
      delete currentpunch_; currentpunch_ = 0;
    }
  }
}

void PuansonAction::slotPeepPunch()
{
  if ( 0 != foremove_ ) {
    peeps_.removeAll(foremove_);
    delete foremove_; foremove_ = 0;
    return;
  }
  else if (0 != currentpunch_ ) {
    peeps_.append(currentpunch_);
    currentpunch_ = 0;
  }
}

void PuansonAction::slotShowPeepInfo()
{
  if ( 0 != foremove_ && 0 != foremove_->punch() ) {
    showPunchMeteodata( foremove_->punch() );
  }
  foremove_ = 0;
}

void PuansonAction::slotPeepLeave()
{
  Document* d = scene_->document();
  if ( 0 == d->activeLayer() ) {
    error_log << QObject::tr("Активный слой не установлен.");
    return;
  }
  Layer* l = d->activeLayer();
  QList<PuansonItem*> list;
  for ( int i = 0, sz = peeps_.size(); i < sz; ++i ) {
    if ( l == peeps_[i]->punch()->layer() ) {
      list.append(peeps_[i]);
    }
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    peeps_.removeAll(list[i]);
    delete list[i];
  }
}

void PuansonAction::slotShowValue()
{
 /* if ( 0 == value_action_ ) {
    return;
  }
  ////value_action_->setCheckable(true);
  //value_action_->setEnabled(true);

  if ( false == value_ ) {
    value_ = true;
  }
  else {
    value_ = false;
    if ( 0 != currentpunch_ ) {
      delete currentpunch_; currentpunch_ = 0;
    }
  }*/

}

}
}
