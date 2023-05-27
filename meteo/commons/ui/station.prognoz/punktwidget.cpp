#include "ui_punktwidget.h"
#include "punktwidget.h"
#include <meteo/commons/global/global.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/proto/forecast.pb.h>

namespace meteo {
namespace map {

enum {
  kStationIndex = 0,
  kStationName  = 1,
  kStationType  = 2,
  kCoordinate   = 3,
  kHeight       = 4,
  kId           = 5,
  kActive       = 6
};

const QString& forecastServiceTitle()
{
  static auto title = global::serviceTitle(settings::proto::kForecastData);
  return title;
}

PunktWidget::PunktWidget(meteo::app::MainWindow* parent) :
  QDialog(parent),
  ui_(new Ui::PunktWidget),
  station_widget_(0),
  mainWindow_(parent)
{

  ui_->setupUi(this);
  ui_->addButton->setIcon(QIcon(":/meteo/icons/plus.png"));
  ui_->delBtn->setIcon(QIcon(":/meteo/icons/minus.png"));
  ui_->addButton->setDisabled(true);
  ui_->treeWidget->setColumnHidden(kId, true);
  ui_->treeWidget->setColumnHidden(kStationType, true);

  connect(this, SIGNAL(finished(int)), SLOT(slotHideStationBar()));
  connect(ui_->addButton, SIGNAL(clicked()), SLOT(slotAddPunkt()));
  connect(ui_->delBtn, SIGNAL(clicked()), SLOT(slotRemovePunkt()));
  connect(ui_->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotIndexChange()));
  connect(ui_->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotItemChanged(QTreeWidgetItem*,int)));

  station_widget_ = new StationWidget(this);
  ui_->stationLayout->addWidget(station_widget_);

  if( nullptr != station_widget_ ){
    connect(station_widget_, SIGNAL(changed()), SLOT(slotAnyChange()));
  }
  slotAnyChange();
}

bool PunktWidget::init()
{
  this->setEnabled(false);
  auto ok = loadPunkts();
  
  QList<sprinf::MeteostationType> stations_to_load;
  stations_to_load << meteo::sprinf::MeteostationType::kStationAero
                   << meteo::sprinf::MeteostationType::kStationSynop
                   << meteo::sprinf::MeteostationType::kStationAirport
                   << meteo::sprinf::MeteostationType::kStationAerodrome
                   << meteo::sprinf::MeteostationType::kStationHydro
                   << meteo::sprinf::MeteostationType::kStationOcean
                   << meteo::sprinf::MeteostationType::kStationRadarmap
                   << meteo::sprinf::MeteostationType::kStationGeophysics;
  station_widget_->loadStation(stations_to_load);
  
  this->setEnabled(true);
  return ok;
}

PunktWidget::~PunktWidget()
{

}

bool PunktWidget::loadPunkts()
{
  ui_->treeWidget->clear();
  meteo::forecast::PunktRequest req;
  req.set_requestcode(meteo::forecast::kPunktGetRequest);
  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if ( nullptr == ch ) {
    error_log.msgBox() << msglog::kNoConnect.arg(forecastServiceTitle());
    return false;
  }
  auto resp = std::unique_ptr<meteo::forecast::PunktResponce>(ch->remoteCall(&meteo::forecast::ForecastData::GetForecastPunkts, req, 10000));

  if (nullptr == resp){
    error_log.msgBox() << msglog::kNoConnect.arg(forecastServiceTitle());
    return false;
  }

  for (int i = 0; i < resp->punkts_size(); ++i){
    const meteo::forecast::PunktValue &value = resp->punkts(i);

    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->treeWidget);
    item->setText(kStationName, QString::fromStdString(value.name()) );
    item->setText(kStationIndex, QString::fromStdString(value.stationid()) );

    double degFi = MnMath::rad2deg( value.fi() );
    double degLa = MnMath::rad2deg( value.la() );

    item->setText(kCoordinate, QString("(%1, %2)").arg(QString::number(degFi, 'd', 2)).arg(QString::number(degLa, 'd', 2)) );
    item->setText(kHeight, QString::number(value.height()) );
    item->setText(kId, QString::fromStdString(value.id()));
    bool is_active = value.isactive();
    item->setCheckState(kActive, is_active? Qt::Checked : Qt::Unchecked );

    item->setText(kStationType, QString::number(value.station_type()) );
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setSizeHint(0, QSize(0, 20));
  }


  for( int i = 0; i < ui_->treeWidget->columnCount(); i++ ){
    ui_->treeWidget->resizeColumnToContents(i);
  }
  ui_->treeWidget->setCurrentItem(ui_->treeWidget->topLevelItem(0));
  return true;
}


void PunktWidget::addPunktForecast()
{
  if( nullptr == station_widget_ ){
    return;
  }

  meteo::forecast::PunktRequest req;

  QString name = station_widget_->ruName();
  QString stationId = station_widget_->stationIndex();
  int height = station_widget_->coord().alt();
  double lat = station_widget_->coord().latDeg();
  double lon = station_widget_->coord().lonDeg();
  int stationType = station_widget_->stationType();

  req.set_requestcode(meteo::forecast::kPunktAddRequest);
  auto punkt = req.add_punkts();
  punkt->set_name(name.toStdString());
  punkt->set_stationid(stationId.toStdString());
  punkt->set_height(height);
  punkt->set_la(lat);
  punkt->set_fi(lon);
  punkt->set_station_type(stationType);
  punkt->set_isactive(true);


  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if ( nullptr == ch ) {
    error_log.msgBox() << msglog::kNoConnect.arg(forecastServiceTitle());
    return;
  }
  meteo::forecast::PunktResponce* resp = ch->remoteCall(&meteo::forecast::ForecastData::GetForecastPunkts, req, 10000);

  if (nullptr == resp){
    error_log.msgBox() << msglog::kNoConnect.arg(forecastServiceTitle());
    return;
  }

  if (false == resp->result()){
    error_log.msgBox() << meteo::msglog::kServiceAnsverFailed.arg(forecastServiceTitle());
    return;
  }
  return;

}

void PunktWidget::rmPunktForecast()
{
  auto selectedItemsList = ui_->treeWidget->selectedItems();
  meteo::forecast::PunktRequest request;
  request.set_requestcode(meteo::forecast::kPunktRemoveRequest);
  for (auto item : selectedItemsList){
    QString askMessage = QString("Удалить пункт %1").arg(item->text(kStationName));
    if (mainWindow_->askUser(askMessage)){
      auto punktToRemove = request.add_punkts();
      punktToRemove->set_id(item->text(kId).toStdString());
    }
  }
  if (0 == request.punkts_size()) return;

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if ( nullptr == ch ) {
    error_log.msgBox() << msglog::kNoConnect.arg(forecastServiceTitle());
    return;
  }
  meteo::forecast::PunktResponce* resp = ch->remoteCall(&meteo::forecast::ForecastData::GetForecastPunkts, request, 10000);

  if (nullptr == resp){
    error_log.msgBox() << msglog::kServiceRequestFailed.arg(forecastServiceTitle());
    return;
  }

  if (!resp->result()){
    error_log.msgBox() << QObject::tr("Не удалось удалить пункт из системы");
    return;
  }
  return;
}


void PunktWidget::slotHideStationBar()
{  
  if( nullptr != parentWidget() ){
    parentWidget()->close();
  }
}

void PunktWidget::slotAddPunkt()
{
  this->setEnabled(false);
  addPunktForecast();
  loadPunkts();
  this->setEnabled(true);
}

void PunktWidget::slotRemovePunkt()
{
  this->setEnabled(false);
  rmPunktForecast();
  loadPunkts();
  ui_->treeWidget->setCurrentItem(ui_->treeWidget->topLevelItem(ui_->treeWidget->topLevelItemCount()-1));
  this->setEnabled(true);
}

void PunktWidget::slotIndexChange()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( nullptr == item ){
    ui_->delBtn->setDisabled(true);
  }
  else{
    ui_->delBtn->setDisabled(false);
  }
}

void PunktWidget::slotAnyChange()
{
  if( nullptr == station_widget_ ){
    return;
  }
  if( nullptr == station_widget_->stationIndex() ){
    ui_->addButton->setDisabled(true);
  }
  else{
    ui_->addButton->setDisabled(false);
  }
}

bool PunktWidget::changePunktActive(const QString &oid, bool isActive){
  meteo::forecast::PunktRequest req;
  auto punkt = req.add_punkts();
  req.set_requestcode(meteo::forecast::kPunktEnabledChangeRequest);
  punkt->set_stationid(oid.toStdString());
  punkt->set_isactive(isActive);

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if ( nullptr == ch ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::settings::proto::kForecastData);
    return false;
  }
  meteo::forecast::PunktResponce* resp = ch->remoteCall(&meteo::forecast::ForecastData::GetForecastPunkts, req, 10000);

  if (nullptr == resp){
    error_log.msgBox() << msglog::kServerAnswerFailed.arg(forecastServiceTitle());
    return false;
  }

  return resp->result();
}

void PunktWidget::slotItemChanged(QTreeWidgetItem* item,int){

  if ( false == this->isEnabled() ) return;
  this->setEnabled(false);

  if ( false == this->changePunktActive(item->text(kId), item->checkState(kActive)) ){
    error_log.msgBox() << msglog::kServerAnswerFailed.arg(forecastServiceTitle());
  }
  //this->loadPunkts();
  this->setEnabled(true);

}

}
}

