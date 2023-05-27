#include "aeroaction.h"
#include "createaerodiagwidget.h"

#include <qfile.h>
#include <qcursor.h>
#include <qevent.h>
#include <qaction.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/stationlist/stationlist.h>
#include <meteo/commons/ui/graphitems/puansonitem.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/graph/layergraph.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace meteo {
namespace map {

static const QString kAxisP = QObject::tr("гПа");
static const QString kAxisH = QObject::tr("км");
static const QString kAxisW = QObject::tr("км/ч");

const QString AeroAction::kName = "aero_action";

AeroAction::AeroAction(::meteo::adiag::CreateAeroDiagWidget* ad, MapScene* scene)
  : Action(scene, AeroAction::kName),
    btn_( new ActionButton ),
   stationsList_(nullptr)

{

  btn_->setToolTip( QObject::tr("Список станций") );
  btn_->setIcon( QIcon(":/meteo/icons/map/vane.png") );
  btnitem_ = scene_->addWidget(btn_);
  btnitem_->setZValue(15000);
  item_ = btnitem_;

  QList<meteo::sprinf::MeteostationType> stations_to_load;
  stations_to_load << meteo::sprinf::MeteostationType::kStationAero
  		   << meteo::sprinf::MeteostationType::kStationSynop
		   << meteo::sprinf::MeteostationType::kStationAirport
		   << meteo::sprinf::MeteostationType::kStationAerodrome
		   << meteo::sprinf::MeteostationType::kStationRadarmap
		   << meteo::sprinf::MeteostationType::kStationHydro
		   << meteo::sprinf::MeteostationType::kStationOcean
		   << meteo::sprinf::MeteostationType::kStationGeophysics;
  
  stationsList_ = new meteo::StationList(ad, "aerolist", stations_to_load);
  QObject::connect( btn_, SIGNAL( toggled(bool) ), this, SLOT( slotShow(bool) ) );
  QObject::connect( stationsList_, SIGNAL( finished(int) ), btn_, SLOT( toggle() ) );
  QObject::connect( ad, SIGNAL(setStationMode(bool)), this, SLOT(slotStationMode(bool)));
  stationsList_->hide();
  ad->setStationListWidget(stationsList_);

  pLAct_  = new QAction(tr("Слева"),nullptr);
  pRAct_  = new QAction(tr("Справа"),nullptr);
  pLRAct_ = new QAction(tr("Слева и справа"),nullptr);
  pHideAct_ = new QAction(tr("Скрыть"),nullptr);

  hLAct_  = new QAction(tr("Слева"),nullptr);
  hRAct_  = new QAction(tr("Справа"),nullptr);
  hLRAct_ = new QAction(tr("Слева и справа"),nullptr);
  hHideAct_ = new QAction(tr("Скрыть"),nullptr);

  wLAct_  = new QAction(tr("Слева"),nullptr);
  wRAct_  = new QAction(tr("Справа"),nullptr);
  wLRAct_ = new QAction(tr("Слева и справа"),nullptr);
  wHideAct_ = new QAction(tr("Скрыть"),nullptr);

  axisMenu_ = new QMenu(tr("Управление осями"));
  hMenu_ = new QMenu(tr("Высота,км"),nullptr);
  pMenu_ = new QMenu(tr("Высота,гПа"),nullptr);
  wMenu_ = new QMenu(tr("Ветер,км/ч"),nullptr);

  axisMenu_->addMenu(hMenu_);
  axisMenu_->addMenu(pMenu_);
  axisMenu_->addMenu(wMenu_);

  pMenu_->addActions({ pLAct_, pRAct_, pLRAct_, pHideAct_ });
  hMenu_->addActions({ hLAct_, hRAct_, hLRAct_, hHideAct_ });
  wMenu_->addActions({ wLAct_, wRAct_, wLRAct_, wHideAct_ });

  for ( QAction* a : { pLAct_, pRAct_, pLRAct_, pHideAct_ } ) { a->setData(kAxisP); }
  for ( QAction* a : { hLAct_, hRAct_, hLRAct_, hHideAct_ } ) { a->setData(kAxisH); }
  for ( QAction* a : { wLAct_, wRAct_, wLRAct_, wHideAct_ } ) { a->setData(kAxisW); }

  for ( QAction* a : actions() ) {
    connect( a,  SIGNAL(triggered(bool)), SLOT(slotActionClicked()) );
  }

  if ( scene_->document()->eventHandler() != nullptr ) {
    scene_->document()->eventHandler()->installEventFilter(this);
  }

  loadSettings();
}

AeroAction::~AeroAction()
{
  delete stationsList_;
  stationsList_ = nullptr;
}

void AeroAction::slotShow( bool fl )
{
  if (nullptr == stationsList_) return;
  if( true == fl ) {
    stationsList_->showNormal();
    stationsList_->fill();
  }
  else {
    stationsList_->hide();
  }
}

void AeroAction::slotStationMode(bool fl)
{
  if (nullptr == stationsList_ || nullptr == btn_) {
    return;
  }

  if (!fl) {
    stationsList_->clearAndClose();
  }
  
  btn_->setEnabled(fl);   
}
  
void AeroAction::slotLayerWidgetClosed()
{
  btn_->toggle();
}

void AeroAction::addActionsToMenu(Menu* menu) const
{
  if ( nullptr == menu ) { return; }

  bool hasP = !findAxis(AxisSimple::kLeft, kAxisP).isEmpty();
  bool hasH = !findAxis(AxisSimple::kLeft, kAxisH).isEmpty();
  bool hasW = !findAxis(AxisSimple::kLeft, kAxisW).isEmpty();

  if ( !hasP && !hasH && !hasW ) { return; }

  menu->addLayerAction(axisMenu_->menuAction());

  for ( QAction* a : { pLAct_, pRAct_, pLRAct_ } ) {
    a->setVisible(hasP);
  }
  for ( QAction* a : { hLAct_, hRAct_, hLRAct_ } ) {
    a->setVisible(hasH);
  }
  for ( QAction* a : { wLAct_, wRAct_, wLRAct_ } ) {
    a->setVisible(hasW);
  }
}

bool AeroAction::eventFilter(QObject* obj, QEvent* e)
{
  if ( scene_->document()->eventHandler() == obj ) {
    if ( e->type() == map::DocumentEvent::DocumentChanged ) {
      DocumentEvent* event = static_cast<DocumentEvent*>(e);
      if ( nullptr != event ) {
        setupUi();
      }
    }
  }

  return Action::eventFilter(obj, e);
}

void AeroAction::slotActionClicked()
{
  QAction* a = qobject_cast<QAction*>(sender());

  for ( QAction* act : { pLAct_, hLAct_, wLAct_ } ) {
    if ( a != act ) {
      continue;
    }
    for ( AxisSimple* ax : findAxis(AxisSimple::kLeft,  act->data().toString()) ) { ax->setVisible(true); }
    for ( AxisSimple* ax : findAxis(AxisSimple::kRight, act->data().toString()) ) { ax->setVisible(false); }
  }

  for ( QAction* act : { pRAct_, hRAct_, wRAct_ } ) {
    if ( a != act ) {
      continue;
    }
    for ( AxisSimple* ax : findAxis(AxisSimple::kLeft,  act->data().toString()) ) { ax->setVisible(false); }
    for ( AxisSimple* ax : findAxis(AxisSimple::kRight, act->data().toString()) ) { ax->setVisible(true); }
  }

  for ( QAction* act : { pLRAct_, hLRAct_, wLRAct_ } ) {
    if ( a != act ) {
      continue;
    }
    for ( AxisSimple* ax : findAxis(AxisSimple::kLeft,  act->data().toString()) ) { ax->setVisible(true); }
    for ( AxisSimple* ax : findAxis(AxisSimple::kRight, act->data().toString()) ) { ax->setVisible(true); }
  }

  for ( QAction* act : { pHideAct_, hHideAct_, wHideAct_ } ) {
    if ( a != act ) {
      continue;
    }
    for ( AxisSimple* ax : findAxis(AxisSimple::kLeft,  act->data().toString()) ) { ax->setVisible(false); }
    for ( AxisSimple* ax : findAxis(AxisSimple::kRight, act->data().toString()) ) { ax->setVisible(false); }
  }

  saveSettings();

  if ( nullptr != scene_->document()->eventHandler() ) {
    scene_->document()->eventHandler()->notifyDocumentChanges(DocumentEvent::Changed);
  }

}

bool AeroAction::loadSettings()
{
  if ( loaded_ ) { return true; }

  if ( !TProtoText::fromFile(MnCommon::userSettingsPath() + "/map/aerodiag_axis.conf", &settings_) ) {
    return false;
  }

  loaded_ = true;
  return true;
}

void AeroAction::saveSettings()
{
  for ( AxisSimple* a : findAxis(AxisSimple::kLeft,kAxisH) ) {
    settings_.set_show_left_h_axis(a->isVisible());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kRight,kAxisH) ) {
    settings_.set_show_right_h_axis(a->isVisible());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kLeft,kAxisP) ) {
    settings_.set_show_left_p_axis(a->isVisible());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kRight,kAxisP) ) {
    settings_.set_show_right_p_axis(a->isVisible());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kLeft,kAxisW) ) {
    settings_.set_show_left_w_axis(a->isVisible());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kRight,kAxisW) ) {
    settings_.set_show_right_w_axis(a->isVisible());
  }

  TProtoText::toFile(settings_,MnCommon::userSettingsPath() + "/map/aerodiag_axis.conf");
}


QList<QAction*> AeroAction::actions() const
{
  return QList<QAction*>()  << pLAct_
                            << pRAct_
                            << pLRAct_
                            << hLAct_
                            << hRAct_
                            << hLRAct_
                            << wLAct_
                            << wRAct_
                            << wLRAct_
                            << pHideAct_
                            << hHideAct_
                            << wHideAct_
                               ;
}

QList<AxisSimple*> AeroAction::findAxis(AxisSimple::AxisType type, const QString& label) const
{
  QList<AxisSimple*> list;

  for ( Layer* l : scene_->document()->layers() ) {
    LayerAxis* layer = maplayer_cast<LayerAxis*>(l);
    if ( nullptr == layer ) { continue; }

    for ( AxisSimple* a : layer->axes() ) {
      if ( a->axisType() != type ) {
        continue;
      }

      if ( label == a->label() ) {
        list << a;
      }
    }
  }
  return list;
}

void AeroAction::setupUi()
{
  loadSettings();

  for ( AxisSimple* a : findAxis(AxisSimple::kLeft,kAxisH) ) {
    a->setVisible(settings_.show_left_h_axis());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kRight,kAxisH) ) {
    a->setVisible(settings_.show_right_h_axis());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kLeft,kAxisP) ) {
    a->setVisible(settings_.show_left_p_axis());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kRight,kAxisP) ) {
    a->setVisible(settings_.show_right_p_axis());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kLeft,kAxisW) ) {
    a->setVisible(settings_.show_left_w_axis());
  }
  for ( AxisSimple* a : findAxis(AxisSimple::kRight,kAxisW) ) {
    a->setVisible(settings_.show_right_w_axis());
  }
}

} // map
} // meteo
