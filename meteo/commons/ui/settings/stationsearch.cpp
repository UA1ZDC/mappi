#include "settingsfuncs.h"
#include "stationsearch.h"
#include "ui_stationsearch.h"
#include "datasortitem.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/common.cpp>

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
  kColumnDistSearch
};
static auto kNoSprinfConnection =
    QObject::tr("Не удалось установить соединение с сервисом "
                    "справочной информации");

StationSearch::StationSearch(double latRad,
                             double lonRad,
                             double range,
                             const GeoPoint own,
                             QList<meteo::sprinf::MeteostationType> stationTypes,
                             QWidget *parent):
  QDialog(parent),
  ui(new Ui::StationSearch),
  searchCenter_(latRad, lonRad),
  range_(range),
  own_(own),
  stationTypes_(stationTypes)
{
  ui->setupUi(this);
  ui->stationsList->setColumnHidden(kColumnIsoNumber, true);  
  QObject::connect(ui->pbAccept, SIGNAL(clicked()), this, SLOT(accept()));
  auto title = QObject::tr("Результаты поиска %1, радиус %2 км.")
      .arg(searchCenter_.toString())
      .arg(range);
  setWindowTitle(title);

  connect(ui->stationsList,
          SIGNAL(itemSelectionChanged()),
          SLOT(slotStationSelectionChagned()));
  connect(ui->stationsList,
          SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
          SLOT(slotStationListItemDoubleClicked(QTreeWidgetItem*, int)));

  QTimer::singleShot(0, this, SLOT(updateItemsTable()));
}

StationSearch::~StationSearch()
{
  delete ui;
}

bool StationSearch::search(){
  sprinf::MultiStatementRequest request;
  for ( auto type: stationTypes_ ){
    request.add_type(type);
  }
  auto lat = searchCenter_.lat();
  auto lon = searchCenter_.lon();
  request.mutable_circle()->mutable_center()->set_lat_radian(lat);
  request.mutable_circle()->mutable_center()->set_lon_radian(lon);
  request.mutable_circle()->set_radius_meter(range_ * 1000);
  sprinf::Stations resp;

  if ( false == global::loadStations(request, &resp) ) {
    error_log.msgBox() << QObject::tr("Не удается загрузить станции "
                                          "в указанном радиусе");
    return false;
  }
  for ( auto stationProto: resp.station()){
    auto station = QString::fromStdString(stationProto.station());
    auto key = qMakePair(stationProto.type(), station);
    stations_[key] = stationProto;
  }
  return true;
}


void StationSearch::updateItemsTable()
{
  ui->stationsList->clear();
  stations_.clear();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  bool ok = search();
  QApplication::restoreOverrideCursor();
  if ( false == ok ){
    reject();
    return;
  }


  for ( auto st: stations_ ){
    auto stType = static_cast<meteo::sprinf::MeteostationType>(st.type());

    auto index = QString::fromStdString(st.station());
    auto srtStationType = stationType(stType);
    auto latDegree = MnMath::rad2deg(st.position().lat_radian());
    auto lonDegree = MnMath::rad2deg(st.position().lon_radian());
    auto alt = st.position().height_meters();
    GeoPoint station(st.position().lat_radian(), st.position().lon_radian());
    auto distance = own_.calcDistance(station);
    auto distanceFromCenter = searchCenter_.calcDistance(station);
    auto ruName = st.name().rus();
    auto enName = st.name().international();

    QTreeWidgetItem* item = new DataSortItem(ui->stationsList);
    item->setSizeHint(1, QSize(-1, 32));
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    item->setText(kColumnIdx,  index);
    item->setData(kColumnIdx, Qt::UserRole, index );

    item->setText(kColumnType, srtStationType);
    item->setData(kColumnType, Qt::UserRole, stType);

    item->setText(kColumnLat,  QString::number(latDegree));
    item->setData(kColumnLat, Qt::UserRole, latDegree);

    item->setText(kColumnLon,  QString::number(lonDegree));
    item->setData(kColumnLon, Qt::UserRole, lonDegree);

    item->setText(kColumnAlt,  QString::number(alt));
    item->setData(kColumnAlt, Qt::UserRole, alt);

    item->setText(kColumnDist,  QString::number(distance));
    item->setData(kColumnDist, Qt::UserRole, distance);

    item->setText(kColumnDistSearch, QString::number(distanceFromCenter));
    item->setData(kColumnDistSearch, Qt::UserRole, distanceFromCenter);

    item->setText(kColumnRuName, QString::fromUtf8(ruName.c_str(),
                                                   ruName.length()));    
    item->setData(kColumnRuName, Qt::UserRole,
                  QString::fromUtf8(ruName.c_str(), ruName.length()));
  }
}

QList<meteo::sprinf::Station> StationSearch::selectedStations() const
{  
  QList<meteo::sprinf::Station> list;
  for ( auto key: selectedStations_ ){
    list << stations_[key];
  }
  return list;
}

void StationSearch::slotStationSelectionChagned()
{
  auto selectedItems = ui->stationsList->selectedItems();
  selectedStations_.clear();
  for ( auto selectedItem: selectedItems ){    
    bool typeOk = false;
    const QString& index = selectedItem->data(kColumnIdx, Qt::UserRole).toString();
    const int type = selectedItem->data(kColumnType, Qt::UserRole).toInt(&typeOk);
    if ( false == typeOk || true == index.isEmpty() ) {
      error_log.msgBox() << QObject::tr("Не удалось извлечь данные о станции из таблицы. Проверьте корректность заполнения данных");
    }
    QPair<int, QString> data(type, index);
    selectedStations_ << qMakePair(type, index);
  }
}

void StationSearch::slotStationListItemDoubleClicked(QTreeWidgetItem *, int)
{
  accept();
}

}
