#ifndef METEO_MASLO_UI_SETTINGS_STATIONSEARCH_H
#define METEO_MASLO_UI_SETTINGS_STATIONSEARCH_H

#include <qdialog.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <commons/geobasis/geopoint.h>
#include <qtreewidget.h>

namespace Ui {
class StationSearch;
}

namespace meteo {
class StationSearch : public QDialog
{
  Q_OBJECT

public:
  explicit StationSearch(double latRad,
                         double lonRad,
                         double range,
                         const GeoPoint own,
                         QList<meteo::sprinf::MeteostationType> stationTypes,
                         QWidget *parent = nullptr);
  ~StationSearch();

  QList<meteo::sprinf::Station> selectedStations() const;

private slots:
  bool search();
  void updateItemsTable();
  void slotStationSelectionChagned();
  void slotStationListItemDoubleClicked(QTreeWidgetItem *, int );

private:  
  Ui::StationSearch *ui;
  const GeoPoint searchCenter_;
  const double range_;  
  const GeoPoint own_;
  QList<meteo::sprinf::MeteostationType> stationTypes_;

  QHash<QPair<int,QString>,meteo::sprinf::Station> stations_;
  QList<QPair<int,QString>> selectedStations_;
};

}
#endif
