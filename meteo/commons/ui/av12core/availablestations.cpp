#include "availablestations.h"
#include "ui_availablestations.h"
#include <meteo/commons/ui/custom/filterheader.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>



namespace meteo {
namespace product {

AvailableStations::AvailableStations(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::AvailableStations)
{
  ui_->setupUi(this);
  setWindowTitle(QObject::tr("Доступные станции"));
  FilterHeader* fheader = new FilterHeader(ui_->stationsTable);
  ui_->stationsTable->setHeader(fheader);
  ui_->stationsTable->setColumnCount(kColumnCount);
  ui_->stationsTable->setHeaderLabels(QStringList{"Индекс","Широта","Долгота","Имя"});
  fheader->setSectionsClickable(true);
  QObject::connect(ui_->stationsTable, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotEmitStations()));
}

AvailableStations::~AvailableStations()
{
  delete ui_;
}

void AvailableStations::setDt(const QDateTime &dt)
{
  dt_ = dt;
  fill();
}

void AvailableStations::fill()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  ui_->stationsTable->clear();


  auto ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );

  if (nullptr == ctrl) {
    QApplication::restoreOverrideCursor();
    return;
  }

  surf::DataRequest request;
  request.set_date_start(dt_.addSecs(-12*60*60).toString(Qt::ISODate).toStdString());
  request.set_date_end(dt_.toString(Qt::ISODate).toStdString());
  request.add_type(meteo::surf::kAeroType);
  request.add_type(meteo::surf::kAeroFix);
  request.add_type(meteo::surf::kAeroArtillery );
  request.add_type(meteo::surf::kAeroWind);
  request.add_type(meteo::surf::kAeroFixLayer );
  request.add_type(meteo::surf::kAeroMob);
  request.add_type(meteo::surf::kAeroBufr);
  request.add_type(meteo::surf::kAeroMobSea);
  request.add_type(meteo::surf::kAeroMobAir);
  request.add_type(meteo::surf::kEndAeroType);
  debug_log << request.DebugString();
  surf::StationsDataReply* response = ctrl->remoteCall( &meteo::surf::SurfaceService::GetAvailableStations, request, 100000 );
  debug_log << response->DebugString();
  delete ctrl;
  if ( nullptr == response ) {
    qApp->restoreOverrideCursor();
    return;
  }
  for( int i = 0, sz = response->coords_size(); i < sz; ++i ) {
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->stationsTable);
    const surf::Point& pnt = response->coords(i);
    item->setText(kIndexColumn, QString::fromStdString(pnt.index()).rightJustified(5, '0'));
    item->setData(kIndexColumn, Qt::UserRole, pnt.type());
    if (!pnt.name().empty()) {
      item->setText(kNameColumn, QString::fromStdString(pnt.name()));
    } else {
      item->setText(kNameColumn, QString::fromStdString(pnt.eng_name()));
    }
        GeoPoint gpnt = GeoPoint(pnt.fi(), pnt.la());
        item->setText(kLatColumn, gpnt.strLat());
        item->setText(kLonColumn, gpnt.strLon());
        item->setData(kLatColumn, Qt::UserRole, pnt.fi());
        item->setData(kLonColumn, Qt::UserRole, pnt.la());
  }
  delete response;
  for (int idx = 0; idx < ui_->stationsTable->columnCount(); idx++) {
    ui_->stationsTable->resizeColumnToContents(idx);
  }

  ui_->stationsTable->sortByColumn( ui_->stationsTable->sortColumn(), ui_->stationsTable->header()->sortIndicatorOrder() );

  static_cast<FilterHeader*>( ui_->stationsTable->header() )->reloadFilters();

  qApp->restoreOverrideCursor();
}

void AvailableStations::slotEmitStations()
{
  surf::Point station;
  auto item = ui_->stationsTable->currentItem();
  station.set_index(item->text(kIndexColumn).toStdString());
  station.set_name(item->text(kNameColumn).toStdString());
  station.set_type(item->data(kIndexColumn, Qt::UserRole).toInt());
  station.set_fi(item->data(kLatColumn, Qt::UserRole).toDouble());
  station.set_la(item->data(kLonColumn, Qt::UserRole).toDouble());
  emit addStation(station);
}


}
}
