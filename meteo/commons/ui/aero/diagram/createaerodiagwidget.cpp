#include "createaerodiagwidget.h"

#include "ui_createaerodiagwidget.h"
#include "aeroindexes.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/aero/diagram/aerodiagdata.h>
#include <meteo/commons/ui/aero/table/aerotablewidget.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/zond/placedata.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actions/mousecoordaction.h>
#include <meteo/commons/ui/map/view/actions/layersaction.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/aero/table/aerotablewidget.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>
#include <meteo/commons/ui/stationlist/stationlist.h>


#define start_time QElapsedTimer _timer__name; _timer__name.start()
#define reset_time(text) debug_log << text << _timer__name.restart() << "msec"


namespace meteo
{
namespace adiag
{
  static const QString kTitle = QObject::tr("Профиль температуры и влажности");
  static const QString kName = QObject::tr("aero_diag_action");
  static const QString kNameStationWgt = QObject::tr("aero");

CreateAeroDiagWidget::CreateAeroDiagWidget(meteo::map::MapWindow *parent)
  : QWidget(parent),
    window_(parent),
    document_(nullptr),
    adiagdata_(nullptr),
    ui_(new Ui::CreateAeroDiagForm),
    mainwindow_(window_->mainwindow()),
    aeroTable_(nullptr),
    aeroIndexes_(nullptr),
    watchedEventHandler_(nullptr),
    mapAction_(nullptr)
{
  ui_->setupUi(this);
  ui_->departureDate->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  ui_->spaceCenterCombo->addItem(tr("<нет доступных центров>"), -1);

  QDateTime dt = QDateTime::currentDateTimeUtc();
  if(dt.time().hour()>12){
      dt.setTime(QTime(12, 0));
    } else {
      dt.setTime(QTime(0, 0));
    }

  document_ = static_cast<meteo::map::AeroDocument*>(window_->document());
  // space type
  ui_->departureDate->setDate(dt.date());
  ui_->departureTime->setTime(dt.time());
  adiagdata_ = new meteo::adiag::AeroDiagData();

  //type_ = kSensType;
  stype_ = kSrcData;

  adiagdata_->setTime(dt);
  adiagdata_->setServiceType(stype_);

  ui_->forecTimeSpin->setSuffix(tr(" ч"));
  ui_->forecTimeSpin->setSingleStep(12);
  ui_->forecTimeSpin->setRange(0,180);
  ui_->forecTimeSpin->setVisible(false);
  ui_->labelForecTime->setVisible(false);
  ui_->restoreDataCheck->setChecked(true);
  ui_->restoreDataCheck->setHidden(true);

  QObject::connect(ui_->runBtn, SIGNAL(clicked(bool)), SLOT(slotRun()));
  QObject::connect(ui_->restoreDataCheck, SIGNAL(toggled(bool)), SLOT(slotRunByCheck(bool)));

  QObject::connect(ui_->spaceCenterCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotCenterCh(int)));
  QObject::connect(ui_->typeCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotServiceCh(int)));

  QObject::connect(ui_->departureTime, SIGNAL(timeChanged(const QTime &)), SLOT(slotSdtCh()));
  QObject::connect(ui_->departureDate, SIGNAL(dateChanged(const QDate &)), SLOT(slotSdtCh()));

  QObject::connect(ui_->forecTimeSpin, SIGNAL(valueChanged(int)), SLOT(slotSdtCh()));
  addStWidget();

  createDocument();
  slotServiceCh(0);
}


CreateAeroDiagWidget::~CreateAeroDiagWidget()
{
  if (nullptr != ui_) {
    delete ui_;
    ui_ = nullptr;
  }

  if (nullptr != aeroTable_) {
    delete aeroTable_;
    aeroTable_ = nullptr;
  }

  if (nullptr != aeroIndexes_) {
    delete aeroIndexes_;
    aeroIndexes_ = nullptr;
  }

  if ( nullptr != watchedEventHandler_ ) {
    this->removeEventFilter(watchedEventHandler_);
    watchedEventHandler_ = nullptr;
  }
  if (nullptr != this->mapAction_ ){
    delete this->mapAction_;
  }
}

void CreateAeroDiagWidget::setStationListWidget(meteo::StationList* stl)
{
  stationList_ = stl;
  if (nullptr != stationList_) {
    stationList_->setDateTime(QDateTime(ui_->departureDate->date(), ui_->departureTime->time()));
    connect(stationList_, SIGNAL(dtChanged(QDateTime)), this, SLOT(setDateTime(QDateTime)));
    connect(this, SIGNAL(currentDataChanged(QString)), stationList_, SLOT(setStation(QString)));
    connect(stationList_, SIGNAL(currentStationChanged()), this, SLOT(slotRunFromList()));
  }
}

bool CreateAeroDiagWidget::addStWidget(){

  stationChanged();
  QObject::connect(ui_->stWidget, SIGNAL(changed()), SLOT(stationChanged()));
  QObject::connect(ui_->stWidget, &StationWidget::signalOnMap, this, &CreateAeroDiagWidget::slotOnMap);
  return true;
}

void CreateAeroDiagWidget::setDateTime(const QDateTime& dt)
{
  ui_->departureDate->setDate(dt.date());
  ui_->departureTime->setTime(dt.time());
}

void CreateAeroDiagWidget::stationChanged()
{
  if(nullptr == ui_->stWidget) return;
  current_data_.setIndex(ui_->stWidget->stationIndex());
  current_data_.setRuName(ui_->stWidget->ruName());
  current_data_.setEnName(ui_->stWidget->enName());
  current_data_.setCoord(ui_->stWidget->coord());
  current_data_.setDt(QDateTime(ui_->departureDate->date(), ui_->departureTime->time()));

  current_data_.setName(makePlaceName());
}



void CreateAeroDiagWidget::slotServiceCh(int indx)
{
  if (nullptr == adiagdata_) {
      return;
  }

  QList<meteo::sprinf::MeteostationType> stations_to_load;
  QList<meteo::sprinf::MeteostationType> stations_to_show;

  stations_to_load << meteo::sprinf::MeteostationType::kStationAero
                   << meteo::sprinf::MeteostationType::kStationSynop
                   << meteo::sprinf::MeteostationType::kStationAirport
                   << meteo::sprinf::MeteostationType::kStationAerodrome
                   << meteo::sprinf::MeteostationType::kStationRadarmap
                   << meteo::sprinf::MeteostationType::kStationHydro
                   << meteo::sprinf::MeteostationType::kStationOcean
                   << meteo::sprinf::MeteostationType::kStationGeophysics;

  stype_ = ServiceType(indx);

  switch (stype_) {
  case kSrcData:
      ui_->spaceCenterCombo->setVisible(false);
      ui_->labelCenter->setVisible(false);
      ui_->forecTimeSpin->setVisible(false);
      ui_->labelForecTime->setVisible(false);
      stations_to_show << meteo::sprinf::MeteostationType::kStationAero;
      if (nullptr != stationList_) {
        stationList_->setDataTypes(QList<meteo::surf::DataType>());
        stationList_->fill();
      }
      break;
  case kFieldData:
      ui_->spaceCenterCombo->setVisible(true);
      ui_->labelCenter->setVisible(true);
      ui_->forecTimeSpin->setValue(0);
      ui_->forecTimeSpin->setVisible(false);
      ui_->labelForecTime->setVisible(false);

      fillCenters();
      break;
  case kSatData:
      ui_->spaceCenterCombo->setVisible(false);
      ui_->labelCenter->setVisible(false);
      ui_->forecTimeSpin->setVisible(false);
      ui_->labelForecTime->setVisible(false);
      if (nullptr != stationList_) {
        stationList_->setDataTypes(QList<meteo::surf::DataType>() <<  meteo::surf::kSatAero);
        stationList_->fill();
      }
      break;
  }
  ui_->departureTime->setTime(QTime(0, 0));
  adiagdata_->setServiceType(stype_);

  if ( false == ui_->stWidget->loadStation( stations_to_load, true, stations_to_show) ) {
    error_msg.msgBox() << QObject::tr("Ошибка при загрузке метеостанций %1").arg(ui_->stWidget->lastError());
  }

  emit setStationMode(stype_ != kFieldData);
}

void CreateAeroDiagWidget::slotCenterCh(int cc)
{
  if (nullptr != adiagdata_) {
      adiagdata_->setCenter(ui_->spaceCenterCombo->itemData(cc).value<int>());
    }
}

void CreateAeroDiagWidget::slotSdtCh()
{
  QDateTime dt = QDateTime(ui_->departureDate->date(), ui_->departureTime->time());

  current_data_.setDt(dt);
  if (nullptr != adiagdata_) {
      adiagdata_->setTime(dt);
      adiagdata_->setForecHour(ui_->forecTimeSpin->value());
    }

  if (nullptr != stationList_) {
    stationList_->setDateTime(dt);
    stationList_->fill();
  }
    if(kFieldData == stype_) fillCenters();

}


void CreateAeroDiagWidget::fillCenters()
{
  QMap< int , QString > centers;
  QComboBox *cb = ui_->spaceCenterCombo;
  if (nullptr == cb) {
      return ;
    }
  cb->clear();
  if (!adiagdata_->loadCenters(&centers) || 0 == centers.size()) {
      cb->addItem(tr("<нет доступных центров>"), -1);
      return ;
    }
  else {
      cb->addItem(tr("<автоматический выбор>"), -1);
    }
  auto i = centers.constBegin();
  while (i != centers.constEnd()) {
      cb->addItem(i.value(), i.key());
      ++i;
    }

}

void CreateAeroDiagWidget::createDocument()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  if (nullptr != window_->mapscene()) {
    window_->mapscene()->addPluginGroup("meteo.map.common");
    map::LayersAction* act = static_cast<map::LayersAction*>(window_->mapscene()->getAction("layersaction"));
    if (nullptr != act) {
      act->setDeleteVisible(false);
    }
  }
  window_->setWindowTitle(kTitle);

  auto path = QList<QPair<QString, QString>> ({QPair<QString, QString>("table", "Задачи")});

  auto title = QPair<QString, QString> ("createaerodiagtable", "Данные зондирования");
  QAction* a = window_->addActionToMenu(title, path);
  QObject::connect(a,SIGNAL(triggered()),this,SLOT(slotOpenAeroTable()));

  title = QPair<QString, QString> ("creatediagindexes", "Расчет индексов неусточивости");
  QAction* a2 = window_->addActionToMenu(title, path);
  QObject::connect(a2,SIGNAL(triggered()),this,SLOT(slotOpenIndexes()));


  QList< meteo::map::Action*> al =  window_->mapscene()->actions();
  for(int i=0;i< al.count();++i){
    if(al.at(i)->name() == "mousecoordaction") {
      meteo::map::MouseCoordAction* act = qobject_cast< meteo::map::MouseCoordAction* >(al.at(i));
      act->setTextFormat("температура %1\xC2\xB0""С   давление %2 гПа");
    }
  }

  watchedEventHandler_ = window_->document()->eventHandler();
  watchedEventHandler_->installEventFilter(this);
  QApplication::restoreOverrideCursor();
}

bool CreateAeroDiagWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == watchedEventHandler_) {
      if (event->type() == map::DocumentEvent::DocumentChanged) {
          auto de = static_cast<meteo::map::DocumentEvent*>(event);
          if (nullptr != de && de->changeType() == ::meteo::map::DocumentEvent::DocumentLoaded) {

              map::proto::Map info = window_->document()->info();

              if (info.document().has_raw_data()) {
                  QByteArray zd(info.document().raw_data().data(), info.document().raw_data().size());
                  current_data_ << zd;
                  document_->setData(current_data_);
                  emit currentDataChanged(current_data_.index());
                }
            }
        }
    }

  return QWidget::eventFilter(watched, event);
}

bool CreateAeroDiagWidget::setIncut(meteo::map::Incut *incut)
{
  if(!incut) return false;
  incut->setMapName(kTitle);

  if ( !current_data_.zond().isEmpty() ) {
    if ( kSrcData == stype_ ) {
      incut->setLabel("center", tr("По данным зондирования") );
    }
    else if ( kFieldData == stype_ ) {
      incut->setLabel("station", QString());
      QString centers;
      if(false == current_data_.zond().centersName().empty()){
        centers = current_data_.zond().centersName().join(";");
        incut->setLabel("center", tr("Центр: %1").arg(centers));
      } else {
        incut->setLabel("center", tr("Центр: %1").arg(ui_->spaceCenterCombo->currentText()));
      }

    } else {
      incut->setLabel("center", tr("По данным зондирования") );
    }

    if(!current_data_.ruName().isEmpty()) {
      incut->setLabel("station", tr("%1 (%2)")
                      .arg(current_data_.ruName()).
                      arg(current_data_.index()));
    }
    if(current_data_.coord().isValid()) {
      incut->setLabel("coords", tr("%1")
                      .arg(current_data_.coord().toString(false, "%1%2")));
    }

  } else {
    incut->setLabel("center", false );
    incut->setLabel("station", false );
    incut->setLabel("coords", false );
  }

  incut->setWMOHeader(false);
  incut->setScale(false);
  return true;
}


void CreateAeroDiagWidget::slotRun()
{
  if (nullptr == document_) {
    error_log << QObject::tr("Отсутствует документ");
    return;
  }
  if (false == current_data_.coord().isValid() || false == current_data_.dt().isValid() ||
      (stype_ == kSrcData && current_data_.index().isEmpty()) ){
    error_log << QObject::tr("Некорректные параметры");
    return;
  }

  setEnabled(false);
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  map::Weather w;
  meteo::map::proto::Map map = window_->document()->info();
  map.clear_station();
  meteo::sprinf::Station * st= map.add_station();

  st->set_station(current_data_.index().toStdString());
  st->set_index(current_data_.index().toInt());
  st->mutable_name()->set_international( current_data_.enName().toStdString());
  st->mutable_name()->set_rus(current_data_.ruName().toStdString());
  st->mutable_name()->set_short_(current_data_.name().toStdString());
  st->mutable_position()->set_lat_radian(current_data_.coord().lat());
  st->mutable_position()->set_lon_radian(current_data_.coord().lon());
  st->mutable_position()->set_height_meters(current_data_.coord().alt());
  map.set_datetime(current_data_.dt().toString(Qt::ISODate).toStdString());

  switch (stype_) {
  case kSrcData:
    map.set_source(meteo::map::proto::DataSource::kSurface);
    break;
  case kFieldData: {
    map.set_source(meteo::map::proto::DataSource::kField);
    map.set_hour(ui_->forecTimeSpin->value());
    map.mutable_document()->mutable_map_center()->set_lat_radian( ui_->stWidget->coord().fi());
    map.mutable_document()->mutable_map_center()->set_lon_radian( ui_->stWidget->coord().la());
    map.mutable_document()->mutable_map_center()->set_height_meters(ui_->stWidget->coord().alt());
    map.set_datetime(QDateTime( ui_->departureDate->date(), ui_->departureTime->time()).toString(Qt::ISODate).toStdString());

    int index = ui_->spaceCenterCombo->currentIndex();
    int center = ui_->spaceCenterCombo->itemData(index).value<int>();
    if (-1 != center) {
      map.set_center(center);
    }
    else {
      map.clear_center();
    }
  }
    break;
  case kSatData:
    map.set_source(meteo::map::proto::DataSource::kSatSource);
    break;
  }
  QString errstr;
  if(w.buildAD(document_,map,&errstr)){
    current_data_ = document_->data();
    if (nullptr != window_ && nullptr != window_->document()) {
      QByteArray zd;
      current_data_ >> zd;
      ::meteo::map::proto::Map info = window_->document()->info();
      info.mutable_document()->set_raw_data(zd.data(), zd.size());
      window_->document()->setProperty(info.document());
    }
    current_data_.setName(makePlaceName());
    document_->setName(kTitle +" - "+ current_data_.name());
  }
  else {
    document_->setName(kTitle);
    info_log.msgBox()<<errstr;
    //QMessageBox::information( this, QObject::tr("Внимание"), errstr);
    map.Clear();
    current_data_.zond().clear();
    document_->setMap(map);
    // document_->clearDocument();
  }
  QList<map::Incut*> list = document_->incuts();
  if( 0 != list.size() ) {
    setIncut(list[0]);
  }

  emit currentDataChanged(current_data_.index());

  QApplication::restoreOverrideCursor();
  setEnabled(true);
  window_->loadBaseLayersVisibilitySettings();
}

void CreateAeroDiagWidget::slotRunByCheck(bool checked)
{
  adiagdata_->setDataRestore(checked);
  slotRun();
}

void CreateAeroDiagWidget::slotRunFromList()
{
  if (nullptr == stationList_) {
    return;
  }

  QString st = stationList_->station();
  QString name = stationList_->name();
  GeoPoint gp = stationList_->coord();
  ui_->stWidget->setStation(st, gp, name);

  slotRun();
}


QString CreateAeroDiagWidget::makePlaceName() const
{
  QString name = "";
  QString stName, stCoord, stIndex;
  if (!current_data_.ruName().isEmpty()) {
      stName = current_data_.ruName();
  } else if (!current_data_.enName().isEmpty()) {
      stName = current_data_.enName();
  }
  if (!current_data_.index().isEmpty()) {
      stIndex =  current_data_.index();
  }
  if(current_data_.coord().isValid()){
      stCoord = current_data_.coord().toString(true, "%1 %2 %3");
  }
  name = tr(" %1 (%2, %3)").arg(stName)
                                     .arg(stIndex)
                                     .arg(stCoord);
  //  name += ( !current_data_.dt().isValid() ) ? "" : QObject::tr(" за ") + current_data_.dt().toString("hh:mm MM-dd-yyyy");
  return name;
}

zond::PlaceData CreateAeroDiagWidget::currentPlaceData() const
{
  return current_data_;
}

//! Таблица с данными зондирования
void CreateAeroDiagWidget::slotOpenAeroTable()
{
  if(!aeroTable_) {
      aeroTable_ = new aero::AeroTableWidget(this);
      connect(this, SIGNAL(currentDataChanged(QString)), this, SLOT(slotFillAeroTable()));
    }
  aeroTable_->fillAeroTable(currentPlaceData());
  aeroTable_->show();
}

void CreateAeroDiagWidget::slotFillAeroTable(){
  if(aeroTable_){
      aeroTable_->fillAeroTable(currentPlaceData());
    }
}

//! Виджет с индексами неустойчивости
void CreateAeroDiagWidget::slotOpenIndexes()
{
  if (nullptr == aeroIndexes_) {
      aeroIndexes_ = new AeroIndexes(this);
    }

  aeroIndexes_->show();
}

//! true - режим получения данных станция (для списка доступных данных)
// bool CreateAeroDiagWidget::isStationDataMode()
// {
//   if (/*type_ == kSensType && */stype_ != kField) {
//       return true;
//     }

//   return false;
// }


StationWidget *CreateAeroDiagWidget::stationWidget() {
  return this->ui_->stWidget;
}

void CreateAeroDiagWidget::slotOnMap(bool isOnMap)
{
  if ( true == isOnMap ){
    auto scene = this->stationWidget()->getAssociatedScene();
    this->mapAction_ = new meteo::map::VerticalCutAction(scene);
    QObject::connect(this->mapAction_, &QObject::destroyed, this, &CreateAeroDiagWidget::onMapActionDestroyed);
    scene->addAction(this->mapAction_);
  }
  else {
    if (nullptr != this->mapAction_){
      delete this->mapAction_;
      this->mapAction_ = nullptr;
    }
  }
}

void CreateAeroDiagWidget::onMapActionDestroyed(QObject* obj)
{
  if ( obj == this->mapAction_) {
    this->mapAction_ = nullptr;
  }
}

}
}
