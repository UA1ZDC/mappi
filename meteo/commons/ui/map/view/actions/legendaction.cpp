#include "legendaction.h"

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
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/ui/map/layermrl.h>
#include "../mapwindow.h"
#include "../mapscene.h"
#include "../widgetitem.h"
#include "../menu.h"

namespace meteo {
namespace map {

static const auto kMenuItem = QPair<QString, QString>("legend", QObject::tr("Легенда"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({QPair<QString, QString>("view", QObject::tr("Вид"))});

LegendAction::LegendAction( MapScene* scene )
  : Action(scene, "legendaction" ),
  legendmenu_( new QMenu( QObject::tr("Легенда") ) )
{
  legendmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-top-center.png"), QObject::tr("Наверху"), this, SLOT( slotShowLegendTop() ) );
  legendmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-bottom-center.png"), QObject::tr("Внизу"), this, SLOT( slotShowLegendBottom() ) );
  legendmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-mid-left.png"), QObject::tr("Слева"), this, SLOT( slotShowLegendLeft() ) );
  legendmenu_->addAction( QIcon(":/meteo/icons/text/text-pos-mid-right.png"), QObject::tr("Справа"), this, SLOT( slotShowLegendRight() ) );

  hideaction_ = legendmenu_->addAction( QObject::tr("Сохранить настройки легенды"), this, SLOT( slotSaveLegend() ) );
  hideaction_ = legendmenu_->addAction( QObject::tr("Скрыть"), this, SLOT( slotHideLegend() ) );
  QAction* act = scene_->mapview()->window()->addActionToMenu(kMenuItem, kMenuPath);
  if ( 0 != act ) {
    act->setMenu(legendmenu_);
    QMenu* pm = qobject_cast<QMenu*>( act->parentWidget() );
    if ( 0 != pm ) {
      QObject::connect( pm, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShowParentMenu() ) );
    }
  }
}

LegendAction::~LegendAction()
{
  delete legendmenu_; legendmenu_ = 0;
}

void LegendAction::addActionsToMenu( Menu* menu ) const 
{
  legendmenu_->menuAction()->setEnabled(false);
  Document* doc = scene_->document();
  if ( 0 == doc ) {
    return;
  }
  Layer* l = Legend::legendLayer(doc);
  LayerIso* liso = maplayer_cast<LayerIso*>(l);
  if ( 0 != liso ) {
    if ( false == liso->hasGradient() ) {
      return;
    }
  }
  else {
    LayerMrl* lm = maplayer_cast<LayerMrl*>(l);
    if ( 0 == lm ) {
      return;
    }
  }
  legendmenu_->menuAction()->setEnabled(true);
  if ( 0 == legend() || false == legend()->visible() ) {
    hideaction_->setDisabled(true);
  }
  else {
    hideaction_->setEnabled(true);
  }
  menu->addDocumentAction( legendmenu_->menuAction() );
}

Legend* LegendAction::legend() const
{
  if ( 0 == scene_->document() ) {
    return 0;
  }
  return scene_->document()->legend();
}

void LegendAction::showLegend( Position pos )
{
  Legend* l = legend();
  Orientation o = kHorizontal;
  if ( kLeftCenter == pos || kRightCenter == pos ) {
    o = kVertical;
  }
  l->setOrient(o);
  l->setPositionOnDocument(pos);
  l->setVisible(true);
}

void LegendAction::slotSaveLegend()
{
  Legend* l = legend();
  if ( 0 == l ) {
    return;
  }
  l->saveLastParams();
}

void LegendAction::slotHideLegend()
{
  Legend* l = legend();
  if ( 0 == l ) {
    return;
  }
  l->setVisible(false);
}

void LegendAction::slotShowLegendTop()
{
  showLegend( kTopCenter );
}

void LegendAction::slotShowLegendBottom()
{
  showLegend( kBottomCenter );
}

void LegendAction::slotShowLegendLeft()
{
  showLegend( kLeftCenter );
}

void LegendAction::slotShowLegendRight()
{
  showLegend( kRightCenter );
}

void LegendAction::slotAboutToShowParentMenu()
{
  legendmenu_->menuAction()->setEnabled(false);
  Document* doc = scene_->document();
  if ( 0 == doc ) {
    return;
  }

  Layer* l = Legend::legendLayer(doc);
//
  LayerIso* liso = maplayer_cast<LayerIso*>(l);
  if ( 0 != liso ) {
    if ( false == liso->hasGradient() ) {
      return;
    }
  }
  else {
    LayerMrl* lm = maplayer_cast<LayerMrl*>(l);
    if ( 0 == lm ) {
      return;
    }
  }
  legendmenu_->menuAction()->setEnabled(true);
  if ( 0 == legend() || false == legend()->visible() ) {
    hideaction_->setDisabled(true);
  }
  else {
    hideaction_->setEnabled(true);
  }
}

}
}
