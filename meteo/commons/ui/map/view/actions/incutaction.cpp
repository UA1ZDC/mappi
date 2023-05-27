#include "incutaction.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qpalette.h>
#include <qgraphicslinearlayout.h>
#include <qgraphicsproxywidget.h>
#include <qdebug.h>
#include <qmatrix.h>
#include <qmath.h>
#include <qcoreapplication.h>
#include <qcursor.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../mapwindow.h"
#include "../widgetitem.h"
#include "../menu.h"

namespace meteo {
namespace map {

static const auto kMenuItem = QPair<QString, QString>("incut", QObject::tr("Врезка"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({QPair<QString, QString>("view", QObject::tr("Вид"))});

IncutAction::IncutAction( MapScene* scene )
  : Action(scene, "incutaction" ),
  incutmenu_( new QMenu( QObject::tr("Врезка") ) )
{
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-top-left.png"), QObject::tr("Слева вверху"), this, SLOT( slotShowIncutTopLeft() ) );
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-top-right.png"), QObject::tr("Справа вверху"), this, SLOT( slotShowIncutTopRight() ) );
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-bottom-left.png"), QObject::tr("Слева внизу"), this, SLOT( slotShowIncutBottomLeft() ) );
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-bottom-right.png"), QObject::tr("Справа внизу"), this, SLOT( slotShowIncutBottomRight() ) );
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-top-center.png"), QObject::tr("Наверху"), this, SLOT( slotShowIncutTop() ) );
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-bottom-center.png"), QObject::tr("Внизу"), this, SLOT( slotShowIncutBottom() ) );
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-mid-left.png"), QObject::tr("Слева"), this, SLOT( slotShowIncutLeft() ) );
  incutmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-mid-right.png"), QObject::tr("Справа"), this, SLOT( slotShowIncutRight() ) );

  QMenu* sostavmenu = incutmenu_->addMenu( QObject::tr("Состав") );
  mapshow_ = sostavmenu->addAction( QObject::tr("Наименование карты"), this, SLOT( slotShowMap( bool ) ) );
  mapshow_->setCheckable(true);
  wmoshow_ = sostavmenu->addAction( QObject::tr("Заголовок ВМО"), this, SLOT( slotShowWMO( bool ) ) );
  wmoshow_->setCheckable(true);
  authorshow_ = sostavmenu->addAction( QObject::tr("Составитель"), this, SLOT( slotShowAuthor( bool ) ) );
  authorshow_->setCheckable(true);
  dateshow_ = sostavmenu->addAction( QObject::tr("Дата"), this, SLOT( slotShowDate( bool ) ) );
  dateshow_->setCheckable(true);
  termshow_ = sostavmenu->addAction( QObject::tr("Срок прогноза"), this, SLOT( slotShowTerm( bool ) ) );
  termshow_->setCheckable(true);
  scaleshow_ = sostavmenu->addAction( QObject::tr("Масштаб"), this, SLOT( slotShowMashtab( bool ) ) );
  scaleshow_->setCheckable(true);
  projshow_ = sostavmenu->addAction( QObject::tr("Проекция"), this, SLOT( slotShowProjection( bool ) ) );
  projshow_->setCheckable(true);
  baselayershow_= sostavmenu->addAction( QObject::tr("Базовые слои"), this, SLOT( slotShowBaseLayer( bool ) ) );
  baselayershow_->setCheckable(true);
  layershow_= sostavmenu->addAction( QObject::tr("Cлои"), this, SLOT( slotShowLayer( bool ) ) );
  layershow_->setCheckable(true);
  sostavaction_ = sostavmenu->menuAction();

  hideaction_ = incutmenu_->addAction( QObject::tr("Сохранить настройки врезки"), this, SLOT( slotSaveIncut() ) );
  hideaction_ = incutmenu_->addAction( QObject::tr("Скрыть"), this, SLOT( slotHideIncut() ) );
  QAction* act = scene_->mapview()->window()->addActionToMenu(kMenuItem, kMenuPath);
  if ( 0 != act ) {
    QMenu* pm = qobject_cast<QMenu*>( act->parentWidget() );
    if ( 0 != pm ) {
      QObject::connect( pm, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShowParentMenu() ) );
    }
    act->setMenu(incutmenu_);
  }
}

IncutAction::~IncutAction()
{
  delete this->incutmenu_;
}

void IncutAction::addActionsToMenu( Menu* menu ) const
{
  if ( false == hasIncuts() || false == incut()->visible() ) {
    hideaction_->setDisabled(true);
    sostavaction_->setVisible(false);
  }
  else {
    Incut* i = incut();
    hideaction_->setEnabled(true);
    sostavaction_->setVisible(true);
    mapshow_->setChecked( i->mapVisible() );
    wmoshow_->setChecked( i->wmoVisible() );
    authorshow_->setChecked( i->authorVisible() );
    dateshow_->setChecked( i->dateVisible() );
    termshow_->setChecked( i->termVisible() );
    scaleshow_->setChecked( i->scaleVisible() );
    projshow_->setChecked( i->projectionVisible() );
    baselayershow_->setChecked( i->baseLayerVisible() );
    layershow_->setChecked( i->layerVisible() );
  }
  menu->addDocumentAction( incutmenu_->menuAction() );
}

bool IncutAction::hasIncuts() const
{
  if ( 0 == scene_->document() ) {
    return false;
  }
  QList<Incut*> list = scene_->document()->incuts();
  if ( 0 == list.size() ) {
    return false;
  }
  return true;
}

Incut* IncutAction::incut() const
{
  if ( 0 == scene_->document() ) {
    return 0;
  }
  QList<Incut*> list = scene_->document()->incuts();
  if ( 0 == list.size() ) {
    return 0;
  }
  return list[0];
}

void IncutAction::showIncut( Position pos )
{
  Incut* i = 0;
  if ( false == hasIncuts() ) {
    i = scene_->document()->addIncut(pos);
    i->setVisible(true);
  }
  else {
    i = scene_->document()->incuts()[0];
    i->setPositionOnDocument(pos);
    i->setVisible(true);
  }
}

void IncutAction::slotSaveIncut()
{
  if ( false == hasIncuts() ) {
    return;
  }
  scene_->document()->incuts()[0]->saveLastParams();
}

void IncutAction::slotHideIncut()
{
  if ( false == hasIncuts() ) {
    return;
  }
  scene_->document()->incuts()[0]->setVisible(false);
}

void IncutAction::slotShowIncutTopLeft()
{
  showIncut( kTopLeft );
}

void IncutAction::slotShowIncutTopRight()
{
  showIncut( kTopRight );
}

void IncutAction::slotShowIncutBottomLeft()
{
  showIncut( kBottomLeft );
}

void IncutAction::slotShowIncutBottomRight()
{
  showIncut( kBottomRight );
}

void IncutAction::slotShowIncutTop()
{
  showIncut( kTopCenter );
}

void IncutAction::slotShowIncutBottom()
{
  showIncut( kBottomCenter );
}

void IncutAction::slotShowIncutLeft()
{
  showIncut( kLeftCenter );
}

void IncutAction::slotShowIncutRight()
{
  showIncut( kRightCenter );
}

void IncutAction::slotShowMap( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setMapName(fl);
}

void IncutAction::slotShowWMO( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setWMOHeader(fl);
}

void IncutAction::slotShowAuthor( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setAuthor(fl);
}

void IncutAction::slotShowDate( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setDate(fl);
}

void IncutAction::slotShowTerm( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setTerm(fl);
}

void IncutAction::slotShowMashtab( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setScale(fl);
}

void IncutAction::slotShowProjection( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setProjection(fl);
}

void IncutAction::slotShowBaseLayer( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setBaseLayerVisible(fl);
}

void IncutAction::slotShowLayer( bool fl )
{
  Incut* i = incut();
  if ( 0 == i ) {
    return;
  }
  i->setLayerVisible(fl);
}

void IncutAction::slotAboutToShowParentMenu()
{
  if ( false == hasIncuts() || false == incut()->visible() ) {
    hideaction_->setDisabled(true);
    sostavaction_->setVisible(false);
  }
  else {
    Incut* i = incut();
    hideaction_->setEnabled(true);
    sostavaction_->setVisible(true);
    mapshow_->setChecked( i->mapVisible() );
    wmoshow_->setChecked( i->wmoVisible() );
    authorshow_->setChecked( i->authorVisible() );
    dateshow_->setChecked( i->dateVisible() );
    termshow_->setChecked( i->termVisible() );
    scaleshow_->setChecked( i->scaleVisible() );
    projshow_->setChecked( i->projectionVisible() );
    baselayershow_->setChecked( i->baseLayerVisible() );
    layershow_->setChecked( i->layerVisible() );
  }
}

}
}
