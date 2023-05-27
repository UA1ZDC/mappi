#include "stationselectform.h"
#include "ui_stationselectform.h"
#include "stationsearch.h"
#include "settingsfuncs.h"
#include "datasortitem.h"

#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/common.h>


namespace meteo {

enum Column
{
  kColumnIsoNumber = 0,
  kColumnIdx,
  kColumnRuName,
  kColumnLat,
  kColumnLon,
  kColumnAlt,
  kColumnType,
  kColumnDist,
  kColumnAdditional
};

StationSelectForm::StationSelectForm( const meteo::settings::Location& loc,
                                      QWidget *parent) :
  SettingsWidget(parent),
  ui_(new Ui::StationSelectForm()),
  own_(loc)
{  
  ui_->setupUi(this);  
}

void StationSelectForm::init()
{
  ui_->stationsList->setColumnHidden(kColumnIsoNumber, true);

  connect(ui_->btnAdd, SIGNAL(clicked()), SLOT(bntAddClicked()));
  connect(ui_->btnSearch, SIGNAL(clicked()), SLOT(bntSearchClicked()));
  connect(ui_->pbRemove, SIGNAL(clicked()), SLOT(slotRemoveSelected()));

  QObject::connect(ui_->selectStationWidget, SIGNAL(changed()),
                   this, SLOT(slotStationWidgetChanged()));
  QObject::connect(ui_->stationsList, SIGNAL(itemSelectionChanged()),
                   this, SLOT(slotStationListSelectionChanged()));
  this->ownStationChanged(this->own_);
  this->slotStationWidgetChanged();

  this->ui_->lbStationList->setText( this->stationListLabelText());
}

StationSelectForm::~StationSelectForm()
{
  delete ui_;
}

void StationSelectForm::save()
{
  if (changed_ == false) {
    return;
  }
  if ( true == this->saveStations(this->savedStations_.values())){
    changed_ = false;
  }
}
void StationSelectForm::load()
{
  QList<sprinf::MeteostationType> stationTypes = this->stationTypes();

  ui_->selectStationWidget->loadStation(stationTypes);
  ui_->stationsList->clear();

  this->savedStations_.clear();

  for (auto station: this->loadStations() ){
    this->addStation(station);
  }

  changed_ = false;
}


void StationSelectForm::addStation(const meteo::sprinf::Station& st)
{
  auto stType = static_cast<meteo::sprinf::MeteostationType>(st.type());
  auto station = QString::fromUtf8(st.station().c_str(),
                                   st.station().size());
  auto key = qMakePair(stationType(stType), station);
  savedStations_[key] = st;
  this->updateItemsTable();
}

void StationSelectForm::updateItemsTable()
{
  ui_->stationsList->clear();
  auto additionalColumns = this->additionalStationParams();  
  this->ui_->stationsList->setColumnCount(kColumnAdditional + additionalColumns.size());
  for ( int i = 0; i < additionalColumns.size(); ++i ) {
    this->ui_->stationsList->headerItem()->setText( kColumnAdditional + i, additionalColumns[i] );
  }



  for ( auto st: this->savedStations_ ){
    auto stType = static_cast<meteo::sprinf::MeteostationType>(st.type());
    auto stIndex =  QString::fromUtf8(st.station().c_str());
    auto latDegree = MnMath::rad2deg(st.position().lat_radian());
    auto lonDegree = MnMath::rad2deg(st.position().lon_radian());
    auto alt = st.position().height_meters();
    GeoPoint loc = pbgeopoint2geopoint(own_.coord());
    GeoPoint station(st.position().lat_radian(), st.position().lon_radian());
    auto distance = loc.calcDistance(station);    
    auto columnNameRu = QString::fromUtf8(st.name().rus().c_str());

    QTreeWidgetItem* item = new DataSortItem(ui_->stationsList);
    item->setSizeHint(1, QSize(-1, 32));
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    item->setText(kColumnIdx,  stIndex);
    item->setData(kColumnIdx, Qt::UserRole, stIndex);
    item->setText(kColumnType, stationType(stType));    
    item->setData(kColumnType, Qt::UserRole, stType);    
    item->setText(kColumnLat,  QString::number(latDegree));
    item->setData(kColumnLat, Qt::UserRole, latDegree);
    item->setText(kColumnLon,  QString::number(lonDegree));
    item->setData(kColumnLon, Qt::UserRole, lonDegree);
    item->setText(kColumnAlt,  QString::number(alt));
    item->setData(kColumnAlt, Qt::UserRole, alt);
    item->setText(kColumnDist,  QString::number(distance));
    item->setData(kColumnDist, Qt::UserRole, distance);
    item->setText(kColumnRuName, columnNameRu);
    item->setData(kColumnRuName, Qt::UserRole, columnNameRu);

    for ( int i = 0; i < additionalColumns.size(); ++i ){
      auto widget = this->getAdditionalColumnWidget(i, st);
      this->ui_->stationsList->setItemWidget(item, kColumnAdditional + i,widget);
    }
  }
}

void StationSelectForm::bntAddClicked()
{  
  auto stWgt = ui_->selectStationWidget;

  // auto cType = static_cast<meteo::sprinf::MeteostationType>(stWgt->stationType());
  // auto stationName = stWgt->stationIndex();
  // auto cIdx = stWgt->stationIndex().toInt();
  // auto cLat = static_cast<double>(stWgt->coord().lat());
  // auto cLon = static_cast<double>(stWgt->coord().lon());
  // auto cAlt = static_cast<double>(stWgt->coord().alt());
  // GeoPoint loc = map::pbgeopoint2geopoint(own_.coord());
  // double dDist = static_cast<double>(loc.calcDistance(stWgt->coord()));
  // auto cDist = QString::number(dDist);
  // // auto datatype = stWgt->dataType();
  // auto cRu = stWgt->ruName();
  // auto cEn = stWgt->enName();

  meteo::sprinf::Station proto = stWgt->toStation();
  // proto.set_index(cIdx);
  // proto.set_type(cType);
  // proto.mutable_name()->set_rus(cRu.toStdString());
  // proto.mutable_name()->set_international(cEn.toStdString());
  // proto.mutable_position()->set_lat_radian(cLat);
  // proto.mutable_position()->set_lon_radian(cLon);
  // proto.mutable_position()->set_height_meters(cAlt);
  // // proto.set_data_type(datatype);
  // proto.set_station(stationName.toStdString());
  this->addStation(proto);
  this->stateChanged();
}

void StationSelectForm::stateChanged()
{
  changed_ = true;
  emit changed();
}

void StationSelectForm::bntSearchClicked()
{
  auto stationWidget = ui_->selectStationWidget;
  auto coord = stationWidget->coord();
  double lat = static_cast<double>(coord.lat());
  double lon = static_cast<double>(coord.lon());
  double range = ui_->sbSearchRange->value();
  auto ownCoord = pbgeopoint2geopoint(own_.coord());

  StationSearch search(lat, lon, range, ownCoord, this->stationTypes());
  auto exitCode = search.exec();
  if ( exitCode == QDialog::Accepted ){    
    auto selected =search.selectedStations();
    for (auto station: search.selectedStations() ){
      this->addStation(station);
      this->stateChanged();
    }
  }
}

void StationSelectForm::slotStationListSelectionChanged()
{
  auto selectedCnt = ui_->stationsList->selectedItems().count();
  ui_->pbRemove->setEnabled( selectedCnt != 0 );
}

void StationSelectForm::slotStationWidgetChanged()
{
  bool isValid = ui_->selectStationWidget->isStationValid();
  ui_->btnAdd->setEnabled(isValid);
  
  // auto index = ui_->selectStationWidget->stationIndex();
  // auto dataType = ui_->selectStationWidget->dataType();
  // auto stationType = ui_->selectStationWidget->stationType();
  // auto modelIndex = ui_->selectStationWidget->findItem(index, stationType, dataType);  
  // ui_->btnAdd->setEnabled(true == modelIndex.isValid()&&
  //                         false == index.isEmpty()
  //                         );
}

void StationSelectForm::slotRemoveSelected()
{
  bool isChanged = false;
  for ( auto item: ui_->stationsList->selectedItems()){
    auto index = item->data(kColumnIdx, Qt::UserRole).toString();
    auto type = static_cast<sprinf::MeteostationType>(
          item->data(kColumnType, Qt::UserRole).toInt()
          );
    auto key = qMakePair(stationType(type), index);
    if ( true == this->savedStations_.contains(key) ){
      isChanged = true;
      this->savedStations_.remove(key);
    }
  }
  if ( true == isChanged ){
    this->updateItemsTable();
    this->stateChanged();
  }
}

void StationSelectForm::ownStationChanged(const meteo::settings::Location& loc)
{
  this->own_ = loc;
  this->updateItemsTable();
}

const meteo::settings::Location& StationSelectForm::own()
{
  return this->own_;
}

QStringList StationSelectForm::additionalStationParams()
{
  return QStringList();
}

QWidget* StationSelectForm::getAdditionalColumnWidget(int index, const meteo::sprinf::Station& station)
{
  Q_UNUSED(index);
  Q_UNUSED(station);
  return nullptr;
}

}
