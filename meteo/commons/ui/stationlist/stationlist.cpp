#include "stationlist.h"
#include "ui_stationlist.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/locsettings.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/filterheader.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/global/dateformat.h>

using namespace meteo;

enum Columns {
  kIndexCol = 0,
  kNameCol = 1,
  kLatCol  = 2,
  kLonCol  = 3,
  kDistCol  = 4
};

StationList::StationList(QWidget* parent,
			 const QString& name,
			 const QList<meteo::sprinf::MeteostationType>& stations_to_load):
  QDialog(parent,Qt::WindowStaysOnTopHint),
  ui_(nullptr),
  current_item_(nullptr)
{
  ui_ = new Ui::StationList;
  ui_->setupUi(this);
  ui_->date->setDisplayFormat(meteo::dtHumanFormatDateOnly);

  FilterHeader* fheader = new FilterHeader(ui_->stations);
  ui_->stations->setHeader(fheader);
  fheader->setSectionsClickable(true);

  ui_->stations->sortByColumn(kDistCol, Qt::AscendingOrder);

  settings::Location loc = global::Settings::instance()->location();
  ui_->point->setStation(QString::fromStdString(loc.index()),
			 pbgeopoint2geopoint(loc.coord()),
			 QString::fromStdString(loc.name()));

  connect(ui_->stations, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(changeCurrent(QTreeWidgetItem *)));
  
  connect(ui_->time, SIGNAL(timeChanged(const QTime &)), SLOT(dtChanged()));
  connect(ui_->date, SIGNAL(dateChanged(const QDate &)), SLOT(dtChanged()));
  connect(ui_->point, SIGNAL(changed()), SLOT(recalcDistance()));

  ui_->point->setObjectName(name);
  ui_->point->loadStation(stations_to_load);
}

StationList::~StationList()
{
  if (nullptr != ui_) {
    delete ui_;
    ui_ = nullptr;
  }
}

void StationList::setPoint(const QString &station, const GeoPoint& gp, const QString& name)
{
  ui_->point->setStation(station, gp, name);
}

//! Очистить и закрыть окно
void StationList::clearAndClose()
{
  ui_->stations->clear();
  ui_->stationCount->setText(QObject::tr(" Нет станций "));
  current_item_ = nullptr;
  station_ = QString();
  name_ = QString();
  coord_ = GeoPoint();
  
  close();
}

//! Установка даты/времени
void StationList::setDateTime(const QDateTime& dt)
{
  if (dt.date() != ui_->date->date()) {
    ui_->date->setDate(dt.date());
  }
  if (dt.time() != ui_->time->time()) {
    ui_->time->setTime(dt.time());
  }
}

//! Установка текущей станции в списке
void StationList::setStation(const QString& station)
{
  curDt_ = QDateTime(ui_->date->date(), ui_->time->time());
  station_ = station;

  if (!isVisible()) {
    return;
  }  
  
  if (nullptr != current_item_) {
    for( int i = 0; i <= ui_->stations->columnCount(); i++ ) {
      current_item_->setBackground(i, QBrush(QColor(0,0,0,255), Qt::NoBrush));
    }
  }

  for( int i = 0, sz = ui_->stations->topLevelItemCount(); i < sz; ++i ) {
    QTreeWidgetItem* item = ui_->stations->topLevelItem(i);
    if (nullptr == item) continue;
    if (station == item->text(kIndexCol)) {
      current_item_ = item;
      for( int i = 0; i <= ui_->stations->columnCount(); i++ ){
	current_item_->setBackgroundColor(i, Qt::yellow);
      }
      break;
    }
  }

  if (nullptr != current_item_) {
    ui_->stations->setCurrentItem(current_item_);
    name_ = current_item_->text(kNameCol);
    coord_ = GeoPoint(current_item_->data(kLatCol, Qt::UserRole).toFloat(),
		      current_item_->data(kLonCol, Qt::UserRole).toFloat(),
		      current_item_->data(kLonCol, Qt::UserRole + 1).toFloat());
  }
}


void StationList::fill()
{
  if (dtypes_.isEmpty()) {
    dtypes_ << meteo::surf::kAeroFix
	    << meteo::surf::kAeroMobSea
	    << meteo::surf::kAeroMobAir
	    << meteo::surf::kAeroMob
	    << meteo::surf::kAeroBufr;
  }
  
  fill(dtypes_);
}

// Заполнение списка станций
void StationList::fill(const QList<meteo::surf::DataType>& data_types)
{
  ui_->stations->clear();
  ui_->stationCount->setText(QString(" Нет станций "));

  current_item_ = nullptr;

  if (!isVisible()) {
    return;
  }
  
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  surf::StationsDataReply* response = requestStations(data_types);

  if ( nullptr == response ) {
    // warning_log.msgBox()
    //    << QObject::tr("Данные от сервиса данных не получены. Попробуйте еще раз или перезапустите сервис данных.");
    qApp->restoreOverrideCursor();
    return;
  }

  if ( false == response->result() ) {
    delete response;
    //warning_log.msgBox() << response->comment();
  }
  else {
    GeoPoint point = ui_->point->coord();
    
    if (response->coords_size() > 0) {
      ui_->stationCount->setText(QObject::tr("Всего станций: ").append(QString::number(response->coords_size())));
    }
    
    for( int i = 0, sz = response->coords_size(); i < sz; ++i ) {
      QTreeWidgetItem* item = new QTreeWidgetItem(ui_->stations);
      const surf::Point& pnt = response->coords(i);
      bool isIndex;
      QString station = QString::fromStdString(pnt.index());
      station.toInt(&isIndex);
      if (isIndex) {
	item->setText(kIndexCol, station.rightJustified(5, '0')); //индекс
      }
      else {
	item->setText(kIndexCol, station); //позывной, название и пр.
      }
      if (!pnt.name().empty()) {
	item->setText(kNameCol, QString::fromStdString(pnt.name()));
      }
      else {
	item->setText(kNameCol, QString::fromStdString(pnt.eng_name()));
      }
      GeoPoint gpnt = GeoPoint(pnt.fi(), pnt.la());
      item->setText(kLatCol, gpnt.strLat());
      item->setText(kLonCol, gpnt.strLon());
      item->setData(kLatCol, Qt::UserRole, pnt.fi());
      item->setData(kLonCol, Qt::UserRole, pnt.la());
      item->setData(kLonCol, Qt::UserRole + 1, pnt.height());
      
      item->setData(kDistCol, Qt::DisplayRole, round(gpnt.calcDistance(point)));
      
      if (station_ == QString::fromStdString(pnt.index()) &&
	  curDt_ == QDateTime(ui_->date->date(), ui_->time->time())) {
	current_item_ = item;
	for( int i = 0; i <= ui_->stations->columnCount(); i++ ) {
	  current_item_->setBackgroundColor(i, Qt::yellow);
	}	
      }
    }
    
    
    for (int idx = 0; idx < ui_->stations->columnCount(); idx++) {
      ui_->stations->resizeColumnToContents(idx);
    }
    
    ui_->stations->sortByColumn(ui_->stations->sortColumn(),
				ui_->stations->header()->sortIndicatorOrder());
    
    if (nullptr != current_item_) {
      ui_->stations->setCurrentItem(current_item_);
    }
    
    FilterHeader* fheader = static_cast<FilterHeader*>(ui_->stations->header());
    fheader->reloadFilters();
  }
  
  delete response;
  qApp->restoreOverrideCursor();
}


/*! Запрос станций из БД
  \param dt - срок для запроса станций с доступными данными
*/
surf::StationsDataReply* StationList::requestStations(const QList<meteo::surf::DataType>& data_types)
{
  QDateTime dt =  QDateTime(ui_->date->date(), ui_->time->time());
  
  rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if (nullptr == ctrl) {
    // error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом данных");
    return nullptr;
  }
  
  surf::DataRequest request;
  request.set_date_start(QString("%1").arg(dt.toString("yyyy-MM-dd hh:mm")).toStdString());
  for (auto dtype : data_types) {
    request.add_type(dtype);
  }
  // request.add_type(meteo::surf::kAeroFix);
  // request.add_type(meteo::surf::kAeroMobSea);
  // request.add_type(meteo::surf::kAeroMobAir);
  // request.add_type(meteo::surf::kAeroMob);
  // request.add_type(meteo::surf::kAeroBufr);
  request.set_type_level(meteodescr::kIsobarLevel);
  //var(request.DebugString());
  surf::StationsDataReply* response = ctrl->remoteCall( &meteo::surf::SurfaceService::GetAvailableZond, request, 100000 );
  delete ctrl;

  return response;
}


void StationList::changeCurrent(QTreeWidgetItem * item)
{
  if (nullptr == item) return;

  if (nullptr != current_item_) {
    for( int i = 0; i <= ui_->stations->columnCount(); i++ ) {
      current_item_->setBackground(i, QBrush(QColor(0,0,0,255), Qt::NoBrush));
    }
  }

  current_item_ = item;
  ui_->stations->setCurrentItem(item);

  station_ = current_item_->text(kIndexCol);
  name_ = current_item_->text(kNameCol);
  coord_ = GeoPoint(current_item_->data(kLatCol, Qt::UserRole).toFloat(),
		    current_item_->data(kLonCol, Qt::UserRole).toFloat(),
		    current_item_->data(kLonCol, Qt::UserRole + 1).toFloat());

  emit currentStationChanged();
  
  // StationWidget* sbar = ad_->stationWidget();
  // if (nullptr != sbar) {
  //   GeoPoint gp = GeoPoint(item->data(kLatCol, Qt::UserRole).toFloat(),
  // 			   item->data(kLonCol, Qt::UserRole).toFloat(),
  // 			   item->data(kLonCol, Qt::UserRole + 1).toFloat());
  //   sbar->setStation(item->text(kIndexCol), gp, item->text(kNameCol));
  //   ad_->slotRun();
  // }


}


void StationList::recalcDistance()
{
  GeoPoint station = ui_->point->coord();
  ui_->stations->setSortingEnabled(false);
  for( int i = 0, sz = ui_->stations->topLevelItemCount(); i < sz; ++i ) {
      QTreeWidgetItem* item = ui_->stations->topLevelItem(i);
      if (nullptr == item) continue;
      GeoPoint gpnt = GeoPoint(item->data(kLatCol, Qt::UserRole).toDouble(), item->data(kLonCol, Qt::UserRole).toDouble());
      float d = gpnt.calcDistance(station);
      item->setData(kDistCol, Qt::DisplayRole, round(d));
    }

  ui_->stations->sortByColumn(ui_->stations->sortColumn(),
                              ui_->stations->header()->sortIndicatorOrder());
  ui_->stations->setSortingEnabled(true);
}


void StationList::dtChanged()
{
  emit dtChanged(QDateTime(ui_->date->date(), ui_->time->time()));
}
