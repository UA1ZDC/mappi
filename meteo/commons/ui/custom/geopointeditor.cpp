#include "geopointeditor.h"
#include "ui_geopointeditor.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>
#include <meteo/commons/ui/custom/stationwidget.h>

#include <qevent.h>

QList<GeoPointEditor*> GeoPointEditor::allCreatedGeoPointEditors_ = QList<GeoPointEditor*>();

const QList<GeoPointEditor*>& GeoPointEditor::getAllEditors()
{
  return allCreatedGeoPointEditors_;
}

GeoPointEditor::GeoPointEditor(QWidget* p)
  : QWidget( p ),
    ui_( new Ui::GeoPointEditor ),
    oneLine_( false ),
    labelVisible_( true ),
    format_( kDegMinSec ),
    readOnly_(false),
    mapAction_(nullptr)
{
  allCreatedGeoPointEditors_ << this;
  ui_->setupUi(this);

  setAltitudeVisible(false);
  setMapButtonVisible(false);
  ui_->searchCheck->setEnabled(false);

  setCoordFormat(kDegMinSec);

  connect( ui_->latSpin, SIGNAL(editingFinished()), SIGNAL(changed()) );
  connect( ui_->lonSpin, SIGNAL(editingFinished()), SIGNAL(changed()) );
  connect( ui_->latEdit, SIGNAL(editingFinished()), SIGNAL(changed()) );
  connect( ui_->lonEdit, SIGNAL(editingFinished()), SIGNAL(changed()) );
  connect( ui_->latCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
  connect( ui_->lonCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
  connect( ui_->altSpin, SIGNAL(valueChanged(double)), SIGNAL(changed()) );

  connect( ui_->latEdit, SIGNAL(textChanged(QString)), SLOT(slotUpdateValue()) );
  connect( ui_->lonEdit, SIGNAL(textChanged(QString)), SLOT(slotUpdateValue()) );

  connect( ui_->latSpin, SIGNAL(valueChanged(double)), SIGNAL(changed()) );
  connect( ui_->lonSpin, SIGNAL(valueChanged(double)), SIGNAL(changed()) );
  connect( ui_->latEdit, SIGNAL(textEdited(QString)), SIGNAL(changed()) );
  connect( ui_->lonEdit, SIGNAL(textEdited(QString)), SIGNAL(changed()) );

  connect( ui_->mapBtn, &QToolButton::toggled, this, &GeoPointEditor::slotOnMapRequest);


  connect(ui_->latSpin,  SIGNAL(valueChanged(double)),     SIGNAL(tryChanged()));
  connect(ui_->lonSpin,  SIGNAL(valueChanged(double)),     SIGNAL(tryChanged()));
  connect(ui_->latEdit,  SIGNAL(textChanged(QString)),     SIGNAL(tryChanged()));
  connect(ui_->lonEdit,  SIGNAL(textChanged(QString)),     SIGNAL(tryChanged()));
  connect(ui_->latCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(tryChanged()));
  connect(ui_->lonCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(tryChanged()));
  connect(ui_->altSpin,  SIGNAL(valueChanged(double)),     SIGNAL(tryChanged()));
  connect(this, &GeoPointEditor::signalOnMap, this, &GeoPointEditor::setMapBtnPressed);

}

GeoPointEditor::~GeoPointEditor()
{
  allCreatedGeoPointEditors_.removeOne(this);
  if ( nullptr != this->ui_ ){
    delete this->ui_;
    this->ui_ = nullptr;
  }
  if ( nullptr != this->mapAction_ ){
    this->mapAction_->deleteLater(); //Нужно использовать deleteLater, т.к. disconnect еще не произошел и будет передан сигнал
    this->mapAction_ = nullptr;
  }
}

/**
 * блокируем все сигналы, испускаемые от этой фигни
 * @param bl [description]
 */
void GeoPointEditor::blockAllSignals(bool bl){

  ui_->latSpin->blockSignals(  bl );
  ui_->lonSpin->blockSignals(  bl );
  ui_->latEdit->blockSignals(  bl );
  ui_->lonEdit->blockSignals(  bl );
  ui_->latCombo->blockSignals( bl );
  ui_->lonCombo->blockSignals( bl );
  ui_->altSpin->blockSignals(  bl );
  ui_->searchCheck->blockSignals(  bl );
  this->blockSignals(          bl );
  return;
}

void GeoPointEditor::setCoord( const meteo::GeoPoint& pnt )
{
  meteo::GeoPoint old = coord();

  bool blocked = this->signalsBlocked();
  this->blockSignals(true);  

  setLatRadian(pnt.lat());
  setLonRadian(pnt.lon());

  this->blockSignals(blocked);
  setAltitudeMeter(pnt.alt());

  meteo::GeoPoint now = coord();
  //!blocked
  if ( now != old ) { 
    this->onCoordChanged();
  }
}

void GeoPointEditor::setLatRadian( double lat )
{
  setLatDegree( lat*180.0 / M_PI );
}

void GeoPointEditor::setLonRadian( double lon )
{
  setLonDegree( lon*180.0 / M_PI );
}

void GeoPointEditor::setPropertyError( const QString st, const bool bl ){
  QVariant v= QVariant(bl);

  ui_->latEdit->setProperty(st.toStdString().c_str(), v);
  ui_->lonEdit->setProperty(st.toStdString().c_str(), v);
  return;
}

void GeoPointEditor::setLatDegree( double lat )
{
  int idx = 0;
  if ( lat < 0 ) {
    idx = 1;
    lat = -lat;
  }

  if ( MnMath::isEqual(ui_->latSpin->value(), lat) && idx == ui_->latCombo->currentIndex() ) { return; }

  ui_->latSpin->setValue(lat);
  ui_->latEdit->setDecDegree(lat);
  ui_->latCombo->setCurrentIndex(idx);

  this->onCoordChanged();
}

void GeoPointEditor::setLonDegree( double lon )
{
  int idx = 0;
  if ( 0 > lon ) {
    idx = 1;
    lon = -lon;
  }

  if ( MnMath::isEqual(ui_->lonSpin->value(), lon) && idx == ui_->lonCombo->currentIndex() ) { return; }

  ui_->lonSpin->setValue(lon);
  ui_->lonEdit->setDecDegree(lon);
  ui_->lonCombo->setCurrentIndex(idx);

  this->onCoordChanged();
}

void GeoPointEditor::setAltitudeMeter(double alt)
{
  if ( MnMath::isEqual(ui_->altSpin->value(), alt) ) { return; }

  ui_->altSpin->setValue(alt);
}

meteo::GeoPoint GeoPointEditor::coord() const
{
  meteo::GeoPoint pnt( latRadian(), lonRadian(), altitudeMeter() );
  return pnt;
}

double GeoPointEditor::latRadian() const
{
  return latDegree()*M_PI/ 180.0;
}

double GeoPointEditor::lonRadian() const
{
  return lonDegree()*M_PI/ 180.0;
}

double GeoPointEditor::latDegree() const
{
  double lat = ui_->latSpin->value();
  if ( 1 == ui_->latCombo->currentIndex() ) {
    lat = -lat;
  }
  return lat;
}

double GeoPointEditor::lonDegree() const
{
  double lon = ui_->lonSpin->value();
  if ( 1 == ui_->lonCombo->currentIndex() ) {
    lon = -lon;
  }
  return lon;
}

double GeoPointEditor::altitudeMeter() const
{
  return ui_->altSpin->value();
}

void GeoPointEditor::setLatDegMinSec(const QString& dms)
{
  ui_->latEdit->setDms(CoordData::fromString(dms));
}

QString GeoPointEditor::latDegMinSec() const
{
  return ui_->latEdit->dms();
}

void GeoPointEditor::setLonDegMinSec(const QString& dms)
{
  ui_->lonEdit->setDms(CoordData::fromString(dms));
}

QString GeoPointEditor::lonDegMinSec() const
{
  return ui_->lonEdit->dms();
}

void GeoPointEditor::setOneLine(bool flag)
{
  oneLine_ = flag;

  if ( true == flag ) {
    delete layout();

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setMargin(0);
    l->addWidget(ui_->latLabel);
    l->addWidget(ui_->latSpin);
    l->addWidget(ui_->latEdit);
    l->addWidget(ui_->latCombo);

    l->addWidget(ui_->lonLabel);
    l->addWidget(ui_->lonSpin);
    l->addWidget(ui_->lonEdit);
    l->addWidget(ui_->lonCombo);

    l->addWidget(ui_->altLabel);
    l->addWidget(ui_->altSpin);

    l->addWidget(ui_->mapLabel);
    l->addWidget(ui_->mapBtn);
    l->addWidget(ui_->searchCheck);
    l->addStretch();
  }
  else {
    delete layout();

    QGridLayout* l = new QGridLayout(this);
    l->setMargin(0);
    l->addWidget(ui_->latLabel, 0, 0);
    l->addWidget(ui_->latSpin, 0, 1);
    l->addWidget(ui_->latEdit, 0, 2);
    l->addWidget(ui_->latCombo, 0, 3);

    l->addWidget(ui_->lonLabel, 1, 0);
    l->addWidget(ui_->lonSpin, 1, 1);
    l->addWidget(ui_->lonEdit, 1, 2);
    l->addWidget(ui_->lonCombo, 1, 3);

    l->addWidget(ui_->altLabel, 2, 0);
    l->addWidget(ui_->altSpin, 2, (format_ == kDecDegree) ? 1 : 2);

    l->addWidget(ui_->mapLabel, 3, 1, 1, 2);
    l->addWidget(ui_->mapBtn, 3, 0);
    //FIXME не увидел где используется этот кейс. Пока убрал чтобы не сломать
//    ui_->searchCheck->hide();
    l->addWidget(ui_->searchCheck, 3, 3);

  }
}

void GeoPointEditor::setLabelVisible(bool visible)
{
  labelVisible_ = visible;
  ui_->latLabel->setVisible(labelVisible_);
  ui_->lonLabel->setVisible(labelVisible_);
  ui_->altLabel->setVisible(labelVisible_ && isAltitudeVisible());
  ui_->mapLabel->setVisible(labelVisible_ && isMapButtonVisible());
}

void GeoPointEditor::setCoordFormat(GeoPointEditor::CoordFormat format)
{
  format_ = format;

  switch ( format_ ) {
    case kDecDegree: {
      ui_->latEdit->setVisible(false);
      ui_->lonEdit->setVisible(false);
      ui_->latSpin->setVisible(true);
      ui_->lonSpin->setVisible(true);
    } break;
    case kDegMinSec: {
      ui_->latEdit->setVisible(true);
      ui_->lonEdit->setVisible(true);
      ui_->latSpin->setVisible(false);
      ui_->lonSpin->setVisible(false);
    }
  }

  QGridLayout* l = qobject_cast<QGridLayout*>(layout());
  if ( nullptr != l ) {
    int col = (format_ == kDecDegree) ? 1 : 2;
    l->removeWidget(ui_->altSpin);
    l->addWidget(ui_->altSpin, 2, col);
  }
}

void GeoPointEditor::setReadOnly(bool readOnly)
{
   readOnly_ = readOnly;

   ui_->latEdit->setReadOnly(readOnly_);
   ui_->lonEdit->setReadOnly(readOnly_);
   ui_->latSpin->setReadOnly(readOnly_);
   ui_->lonSpin->setReadOnly(readOnly_);
   ui_->latEdit->setReadOnly(readOnly_);
   ui_->lonEdit->setReadOnly(readOnly_);
   ui_->latSpin->setReadOnly(readOnly_);
   ui_->lonSpin->setReadOnly(readOnly_);
   ui_->altSpin->setReadOnly(readOnly_);
   ui_->latCombo->setEnabled(!readOnly_);
   ui_->lonCombo->setEnabled(!readOnly_);
   ui_->mapBtn->setEnabled(!readOnly_);
   ui_->mapLabel->setEnabled(!readOnly_);

}


bool GeoPointEditor::isMapButtonPressed() const
{
  return ui_->mapBtn->isChecked();
}

bool GeoPointEditor::isMapButtonVisible() const
{
  return ui_->mapBtn->isVisible();
}

void GeoPointEditor::setSearchVisible(bool visible)
{
  ui_->searchCheck->setVisible(visible);
}

bool GeoPointEditor::isSearchChecked() const
{
  return ui_->searchCheck->isChecked();
}

bool GeoPointEditor::isSearchEnabled() const
{
  return ui_->searchCheck->isEnabled();
}

bool GeoPointEditor::isSearchVisible() const
{
  return ui_->searchCheck->isVisible();
}

void GeoPointEditor::setMapButtonVisible(bool visible)
{
  ui_->mapBtn->setVisible(visible);
  ui_->mapLabel->setVisible(isLabelVisible() && visible);
  ui_->searchCheck->setVisible(visible);

}

bool GeoPointEditor::isAltitudeVisible() const
{
  return ui_->altSpin->isVisible();
}

void GeoPointEditor::setAltitudeVisible(bool visible)
{
  ui_->altSpin->setVisible(visible);
  ui_->altLabel->setVisible(isLabelVisible() && visible);
}

void GeoPointEditor::setMapBtnPressed(bool state)
{
  if ( ui_->mapBtn->isChecked() != state ) {
    ui_->mapBtn->toggle();
  }
  ui_->searchCheck->setEnabled(ui_->mapBtn->isChecked());
}

static meteo::map::MapWindow* tryGetMapWindow(QMdiSubWindow* mdi)
{
  if ( nullptr == mdi ){
    return nullptr;
  }

  for (int i = 0; i < mdi->children().count(); ++i)
  {
    meteo::map::MapWindow* mapCast = qobject_cast<meteo::map::MapWindow*>(mdi->children()[i]);
    if ( nullptr == mapCast || nullptr == mapCast->mapscene() || nullptr == mapCast->document() ){
      continue;
    }

    auto doctype = mapCast->document()->property().doctype();
    if (meteo::map::proto::kGeoMap == doctype) {
      return mapCast;
    }
  }
  return nullptr;
}


void GeoPointEditor::unpressMapRequestBtn()
{
  if ( true == this->ui_->mapBtn->isChecked() )  {
    this->ui_->mapBtn->setChecked(false);
  }
}

void GeoPointEditor::openMapWindow()
{
  auto mainWindow = WidgetHandler::instance()->mainwindow();
  auto mdi = mainWindow->mdi();

  this->mapSwitchCurrentActiveMdiSubwindow_ = nullptr;
  this->mapSwitchCurrentActiveMdiSubwindow_ = nullptr;

  auto mapWindows = WidgetHandler::instance()->geoMapSearch();
  if ( 0 == mapWindows.size() ) {
  this->mapSwitchLastActiveMdiSubwindow_ = mdi->activeSubWindow();
    auto mapBuilderAction = mainWindow->findAction("emptymap");
    if (nullptr == mapBuilderAction){
      error_log << QObject::tr("Невозможно создать карту, не найдено действие в меню");
      return;
    }
    mapBuilderAction->trigger();

    this->mapSwitchCurrentActiveMdiSubwindow_ = mdi->activeSubWindow();
    mdi->setActiveSubWindow(this->mapSwitchCurrentActiveMdiSubwindow_);
  }
  else {
    this->mapSwitchLastActiveMdiSubwindow_ = mdi->activeSubWindow();
    this->mapSwitchCurrentActiveMdiSubwindow_ = mapWindows.first();
    mdi->setActiveSubWindow(this->mapSwitchCurrentActiveMdiSubwindow_);
  }

  mdi->tileSubWindows();


  auto map = tryGetMapWindow(mapSwitchCurrentActiveMdiSubwindow_);

  if ( nullptr != map && nullptr != map->mapscene()  ){
    static const auto actionName = QObject::tr("gpeditoraction");
    if ( true == map->mapscene()->hasAction(actionName) ){
      this->mapAction_ = qobject_cast<meteo::map::TraceAction*>(map->mapscene()->getAction(actionName));
    }
    else {
      this->mapAction_ = new meteo::map::TraceAction(map->mapscene(), "gpeditaction");
      this->mapAction_->setWorkMode(meteo::map::TraceAction::WorkMode::DotMode);
    }
    QObject::connect(this->mapAction_, &QObject::destroyed, this, &GeoPointEditor::slotMapActionDestroyed);
  }

  for ( auto editor: allCreatedGeoPointEditors_ ){
    if ( this == editor ){
      continue;
    }
    editor->unpressMapRequestBtn();
  }
  emit signalOnMap(true);
}

void GeoPointEditor::restoreFromMapWindow()
{
  auto mainWindow = WidgetHandler::instance()->mainwindow();
  auto mdi = mainWindow->mdi();
  if (this->mapSwitchCurrentActiveMdiSubwindow_ == mdi->activeSubWindow())
  {
    mdi->setActiveSubWindow(this->mapSwitchLastActiveMdiSubwindow_);
  }

  if ( nullptr != this->mapAction_ ){
    delete this->mapAction_;
  }

  this->mapSwitchCurrentActiveMdiSubwindow_ = nullptr;
  this->mapSwitchCurrentActiveMdiSubwindow_ = nullptr;  
}


void GeoPointEditor::slotOnMapRequest(bool state)
{
  this->setMapBtnPressed(state);
  auto mainWindow = WidgetHandler::instance()->mainwindow();

  auto mdi = mainWindow->mdi();
  if ( nullptr == mdi->currentSubWindow() ) {
    return;
  }
  if ( true == state)
  {
      if ( true == WidgetHandler::instance()->isCurrentTabGeoMap() ){
        return;
      }
      auto mapWindows = WidgetHandler::instance()->geoMapSearch();

      if ( mapWindows.size() == 0 && false == mainWindow->askUser(QObject::tr("Открыть новую карту?")) ){
        this->unpressMapRequestBtn();
        return;
      }
      this->openMapWindow();
    }
  else
  {
    this->restoreFromMapWindow();
  }
}

void GeoPointEditor::slotUpdateValue()
{
  CoordEdit* e = qobject_cast<CoordEdit*>(sender());
  if ( nullptr == e ) {
    return;
  }

  if ( ui_->latEdit == e ) { setLatDegree(e->decDegree()); }
  if ( ui_->lonEdit == e ) { setLonDegree(e->decDegree()); }
}

void GeoPointEditor::slotMapActionDestroyed(QObject* obj)
{
  if ( this->mapAction_ == obj ){
    this->mapAction_ = nullptr;
    emit signalOnMap(false);
  }
}

void GeoPointEditor::onCoordChanged()
{
  auto coord = this->coord();
  if ( nullptr != this->mapAction_ ){
    this->mapAction_->setSearchNear(isSearchChecked());
    this->mapAction_->appendPlace(coord);
  }
  if ( false == this->signalsBlocked() ){
    emit coordChanged(coord);
  }
}

meteo::map::MapScene* GeoPointEditor::getAssociatedScene()
{
  if ( nullptr == this->mapAction_ ){
    return nullptr;
  }
  return mapAction_->scene();
}

meteo::map::TraceAction* GeoPointEditor::getAssociatedAction()
{
  return this->mapAction_;
}
