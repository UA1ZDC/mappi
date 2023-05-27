#include "factwindstations.h"
#include "ui_factwindstations.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace meteo {
namespace product {



FactWindStations::FactWindStations(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::FactWindStations)
{
  ui_->setupUi(this);
  setWindowTitle( QObject::tr("Список станций, со значениями ветра") );
  ui_->stationsTable->setColumnCount(kColumnCount);
  QStringList labels{QObject::tr(""),
                     QObject::tr("Индекс"),
                     QObject::tr("Имя"),
                     QObject::tr("Срок")};
  ui_->stationsTable->setHorizontalHeaderLabels(labels);
  ui_->stationsTable->horizontalHeader()->setStretchLastSection(true);
  ui_->stationsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  //ui_->stationsTable->setFocusPolicy(Qt::NoFocus);
  ui_->addBtn->setIcon(QIcon(":/meteo/icons/add.png"));
  ui_->delBtn->setIcon(QIcon(":/meteo/icons/delete.png"));
  QObject::connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(slotClose()));
  QObject::connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotAvailableStation()));
  QObject::connect(ui_->delBtn, SIGNAL(clicked()), SLOT(slotDeleteStation()));
}

FactWindStations::~FactWindStations()
{
  delete ui_;
  delete availableStations_;
}

void FactWindStations::keyReleaseEvent(QKeyEvent *event)
{
  if ( event->key() == Qt::Key_Escape ) {
    slotClose();
  }
}

void FactWindStations::addStation(const sprinf::Station& station, const QString& shadowIndex)
{
  QString ind;
  if (0 != station.index()) {
    ind = QString::number(station.index());
  }
  else if(station.has_cccc()) {
    ind = QString::fromStdString(station.cccc());
  }
  if ( contains(ind) ) {
    return;
  }
  if (checkData(ind).isEmpty() && findNear_) {
    if( addNearStation(ind, shadowIndex)) {
      return;
    }
  }
  QTableWidgetItem* indexItem = new QTableWidgetItem( ind );
  QTableWidgetItem* nameItem = new QTableWidgetItem (QString::fromUtf8(station.name().rus().data()) );
  QTableWidgetItem* checkDataItem = new QTableWidgetItem();
  QTableWidgetItem* hourItem = new QTableWidgetItem();
  QString hour = checkData(ind);
  if (!hour.isEmpty()) {
    checkDataItem->setIcon(QIcon(":/meteo/icons/ledgreen.png"));
    checkDataItem->setToolTip(QObject::tr("Данные есть"));
  }
  else {
    checkDataItem->setIcon(QIcon(":/meteo/icons/ledred.png"));
    checkDataItem->setToolTip(QObject::tr("Данных нет"));
  }
  indexItem->setData(Qt::UserRole, station.type());
  indexItem->setData(kShadowIndex, shadowIndex);
  indexItem->setData(kFi, station.position().lat_radian());
  indexItem->setData(kLa, station.position().lon_radian());
  indexItem->setFlags(indexItem->flags()^Qt::ItemIsEditable);
  nameItem->setFlags(nameItem->flags()^Qt::ItemIsEditable);
  checkDataItem->setFlags(checkDataItem->flags()^Qt::ItemIsEditable);
  hourItem->setText(hour);

  ui_->stationsTable->setRowCount(ui_->stationsTable->rowCount()+1);
  ui_->stationsTable->setItem(ui_->stationsTable->rowCount()-1,kIndexColumn,indexItem);
  ui_->stationsTable->setItem(ui_->stationsTable->rowCount()-1,kNameColumn,nameItem);
  ui_->stationsTable->setItem(ui_->stationsTable->rowCount()-1,kCheckDataColumn, checkDataItem);
  ui_->stationsTable->setItem(ui_->stationsTable->rowCount()-1,kHourColumn, hourItem);
  ui_->stationsTable->resizeColumnToContents(kCheckDataColumn);
  ui_->stationsTable->resizeColumnToContents(kIndexColumn);
}

void FactWindStations::addStation(const surf::Point &station)
{
  if ( contains(QString::fromStdString(station.index())) ) {
    return;
  }
  if (checkData(QString::fromStdString(station.index())).isEmpty() && findNear_) {
    if( addNearStation(QString::fromStdString(station.index()), QString::fromStdString(station.index())) ) {
      return;
    }
  }
  QTableWidgetItem* indexItem = new QTableWidgetItem( QString::fromStdString(station.index()) );
  QTableWidgetItem* nameItem = new QTableWidgetItem( QString::fromStdString(station.name()) );
  QTableWidgetItem* checkDataItem = new QTableWidgetItem();
  QTableWidgetItem* hourItem = new QTableWidgetItem();
  QString hour = checkData(QString::fromStdString(station.index()));
  if (!hour.isEmpty()) {
    checkDataItem->setIcon(QIcon(":/meteo/icons/ledgreen.png"));
    checkDataItem->setToolTip(QObject::tr("Данные есть"));
  }
  else {
    checkDataItem->setIcon(QIcon(":/meteo/icons/ledred.png"));
    checkDataItem->setToolTip(QObject::tr("Данных нет"));
  }
  indexItem->setData(Qt::UserRole, station.type());
  indexItem->setFlags(indexItem->flags()^Qt::ItemIsEditable);
  indexItem->setData(kShadowIndex,QString::fromStdString(station.index()));
  indexItem->setData(kFi, station.fi());
  indexItem->setData(kLa, station.la());
  nameItem->setFlags(nameItem->flags()^Qt::ItemIsEditable);
  checkDataItem->setFlags(checkDataItem->flags()^Qt::ItemIsEditable);
  hourItem->setText(hour);

  ui_->stationsTable->setRowCount( ui_->stationsTable->rowCount()+1 );
  ui_->stationsTable->setItem( ui_->stationsTable->rowCount()-1, kIndexColumn, indexItem );
  ui_->stationsTable->setItem( ui_->stationsTable->rowCount()-1, kNameColumn, nameItem );
  ui_->stationsTable->setItem(ui_->stationsTable->rowCount()-1,kCheckDataColumn, checkDataItem);
  ui_->stationsTable->setItem(ui_->stationsTable->rowCount()-1,kHourColumn, hourItem);
  ui_->stationsTable->resizeColumnToContents(kCheckDataColumn);
  ui_->stationsTable->resizeColumnToContents(kIndexColumn);
}

void FactWindStations::delStation(const QString& index)
{
  for ( int i = 0, sz = ui_->stationsTable->rowCount(); i < sz; ++i ) {
    if ( ui_->stationsTable->item(i, kIndexColumn)->text() == index
         || ui_->stationsTable->item(i,kIndexColumn)->data(kShadowIndex).toString() == index ) {
      ui_->stationsTable->removeRow(i);
      ui_->stationsTable->resizeColumnsToContents();
      return;
    }
  }
}

bool FactWindStations::contains(const QString &index)
{
  for ( int i = 0, sz = ui_->stationsTable->rowCount(); i < sz; ++i ) {
    if ( ui_->stationsTable->item(i, kIndexColumn)->text() == index ) {
      return true;
    }
  }
  return false;
}

void FactWindStations::setDt(const QDateTime &dt)
{
  if ( dt_ == dt ) {
    return;
  }
  dt_ = dt;
  if ( nullptr != availableStations_ ) {
    if ( availableStations_->isHidden() ) {
      delete availableStations_;
      availableStations_ = nullptr;
    }
    else {
      QDateTime dtCopy{dt};
      QTime tm = dtCopy.time();
      tm.setHMS( int(tm.hour()/12) * 12, tm.minute(),  tm.second());
      dtCopy.setTime(tm);
      availableStations_->setDateTime(dtCopy);
      availableStations_->fill();
    }
  }
  checkAllItems();
}

QList<surf::Point> FactWindStations::getStationsList()
{
  QList<surf::Point> listStations;
  for(int i = 0, sz = ui_->stationsTable->rowCount(); i < sz ; ++i) {
    surf::Point st;
    st.set_index(ui_->stationsTable->item(i,kIndexColumn)->text().toStdString());
    st.set_name(ui_->stationsTable->item(i,kNameColumn)->text().toStdString());
    st.set_type(ui_->stationsTable->item(i,kIndexColumn)->data(Qt::UserRole).toInt());
    listStations.append(st);
  }
  return listStations;
}

void FactWindStations::slotClose()
{
  close();
}

void FactWindStations::slotAvailableStation()
{
  if ( nullptr == availableStations_ ) {
    auto stList = QList<meteo::sprinf::MeteostationType>{ meteo::sprinf::MeteostationType::kStationSynop, meteo::sprinf::MeteostationType::kStationSynmob,
        meteo::sprinf::MeteostationType::kStationAirport, meteo::sprinf::MeteostationType::kStationAero , meteo::sprinf::MeteostationType::kStationAeromob,
        meteo::sprinf::MeteostationType::kStationAirplane, meteo::sprinf::MeteostationType::kStationAerodrome };
    availableStations_ = new StationList(this, objectName(), stList);
    QObject::connect(availableStations_, SIGNAL(currentStationChanged()), SLOT(slotAddStation()));
    QObject::connect(availableStations_, SIGNAL(dtChanged(const QDateTime& )), SLOT(slotLoadStations()) );
  }
  QDateTime dtCopy{dt_};
  QTime tm = dtCopy.time();
  tm.setHMS( int(tm.hour()/12) * 12, tm.minute(),  tm.second());
  dtCopy.setTime(tm);
  availableStations_->setDateTime(dtCopy);
  if (ui_->stationsTable->rowCount() > 0) {
    auto index = ui_->stationsTable->item(0,kIndexColumn)->text();
    GeoPoint gp;
    gp.setFi(ui_->stationsTable->item(0,kIndexColumn)->data(kFi).toDouble());
    gp.setLa(ui_->stationsTable->item(0,kIndexColumn)->data(kLa).toDouble());
    auto name = ui_->stationsTable->item(0,kNameColumn)->text();
    availableStations_->setPoint(index, gp, name);
  }
  availableStations_->move( mapToGlobal( ui_->addBtn->pos() ) );
  availableStations_->show();
  availableStations_->fill();
}

void FactWindStations::slotLoadStations()
{
  if ( nullptr != availableStations_ ) {
    availableStations_->fill();
  }
}
void FactWindStations::slotAddStation()
{
  if ( nullptr != availableStations_ ) {
    availableStations_->fill();
    humanChange_ = true;
    surf::Point station;
    station.set_name( availableStations_->name().toStdString() );
    auto index = availableStations_->station();
    if ( 0 != index.size() && index.at(0) == '0' ) {
      index = index.remove(0,1);
    }
    station.set_index( index.toStdString() );
    station.set_fi(availableStations_->coord().fi());
    station.set_la(availableStations_->coord().la());
    addStation(station);
  }
}
void FactWindStations::slotAddStation(surf::Point station)
{
  humanChange_ = true;
  addStation(station);
}

void FactWindStations::slotDeleteStation()
{
  humanChange_ = true;
  int currentRow = ui_->stationsTable->currentRow();
  if ( -1 == currentRow ) {
    error_log.msgBox() << QObject::tr("Станция для удаления не выбрана");
    return;
  }
  ui_->stationsTable->removeRow(currentRow);
}

QString FactWindStations::checkData(const QString &index)
{
  auto ch = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == ch ) {
    error_log  << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kSrcData));
    return QString();
  }
  meteo::surf::DataRequest req;
  req.set_only_last(true);
  req.add_station(index.toStdString());
  req.add_type( meteo::surf::kAeroType );
  req.add_type( meteo::surf::kAeroFix );
  req.add_type( meteo::surf::kAeroArtillery );
  req.add_type( meteo::surf::kAeroWind );
  req.add_type( meteo::surf::kAeroFixLayer );
  req.add_type( meteo::surf::kAeroMob );
  req.add_type( meteo::surf::kAeroBufr );
  req.add_type( meteo::surf::kAeroMobSea );
  req.add_type( meteo::surf::kAeroMobAir );
  req.add_type( meteo::surf::kEndAeroType );
  req.set_date_start( dt_.addSecs(-12*60*60).toString(Qt::ISODate).toStdString() );
  req.set_date_end( dt_.toString(Qt::ISODate).toStdString() );
  auto respond = ch->remoteCall( &meteo::surf::SurfaceService::GetTZondOnStation, req, 30000 );
  delete ch;
  if ( respond->data_size() > 0 ) {
    auto date = QString::fromStdString(respond->data(0).date());
    delete respond;
    return date;
  }
  delete respond;
  return QString();
}

void FactWindStations::checkAllItems()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  for ( int i = 0, sz = ui_->stationsTable->rowCount(); i < sz; ++i ) {
    auto index = ui_->stationsTable->item(i, kIndexColumn)->text();
    QString hour = checkData(index);
    if ( !hour.isEmpty() ) {
      ui_->stationsTable->item(i, kCheckDataColumn)->setIcon(QIcon(":/meteo/icons/ledgreen.png"));
      ui_->stationsTable->item(i, kCheckDataColumn)->setData(Qt::UserRole, true);
    }
    else {
      ui_->stationsTable->item(i, kCheckDataColumn)->setIcon(QIcon(":/meteo/icons/ledred.png"));
      ui_->stationsTable->item(i, kCheckDataColumn)->setData(Qt::UserRole, false);
    }
    ui_->stationsTable->item(i,kHourColumn)->setText(hour);
  }
  if ( !humanChange_ && findNear_) {
    for( int i = 0, sz = ui_->stationsTable->rowCount(); i < sz; ++i ) {
      if ( !ui_->stationsTable->item(i,kCheckDataColumn)->data(Qt::UserRole).toBool() ) {
        if (addNearStation(ui_->stationsTable->item(i,kIndexColumn)->data(kShadowIndex).toString(), ui_->stationsTable->item(i,kIndexColumn)->data(kShadowIndex).toString())) {
          ui_->stationsTable->removeRow(i);
          i--;
          break;
        }
      }
    }
  }
  qApp->restoreOverrideCursor();
}

bool FactWindStations::addNearStation(const QString& station, const QString& shadowIndex)
{
 auto ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
 if (nullptr == ctrl) {
   return false;
 }
 meteo::sprinf::MultiStatementRequest stReq;
 bool ok = false;
 station.toInt(&ok);
 if (ok) {
   stReq.add_index(station.toInt());
 }
 else {
   stReq.add_cccc(station.toStdString());
 }
 auto stResp = ctrl->remoteCall( &meteo::sprinf::SprinfService::GetStations, stReq, 30000 );
 meteo::sprinf::Station protoSt;
 if ( stResp->station_size() > 0) {
   protoSt.CopyFrom( stResp->station(0) );
 }
 else {
   delete ctrl;
   delete stResp;
   return false;
 }
 delete stResp;
 meteo::sprinf::CircleRegionRequest request;
 request.set_station_type(3);
 request.mutable_circle()->set_radius_meter(radius_*1000);
 request.mutable_circle()->mutable_center()->set_lon_radian(protoSt.position().lon_radian());
 request.mutable_circle()->mutable_center()->set_lat_radian(protoSt.position().lat_radian());
 auto response = ctrl->remoteCall( &meteo::sprinf::SprinfService::GetStationsByPosition, request, 30000 );
 delete ctrl;
 if ( nullptr == response ) {
   return false;
 }
 for ( int i = 0, sz = response->station_size(); i < sz; ++i  ) {
  auto station = response->station(i);
  if ( !checkData(QString::number(station.main().index()) ).isEmpty() ) {
    addStation( station.main(), shadowIndex );
    delete response;
    return true;
  }
 }
 delete response;
 return false;
}

void FactWindStations::hideEvent(QHideEvent *event)
{
  if (nullptr != availableStations_) {
    availableStations_->hide();
  }
  QWidget::hideEvent(event);
}


}
}
