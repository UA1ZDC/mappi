#ifndef METEO_PRODUCT_UI_AV12PLUGIN_FACTWINDSTATIONS_H
#define METEO_PRODUCT_UI_AV12PLUGIN_FACTWINDSTATIONS_H

#include <qevent.h>
#include <QWidget>
#include <qdatetime.h>
//#include "availablestations.h"
#include <meteo/commons/ui/stationlist/stationlist.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface.pb.h>

namespace Ui {
  class FactWindStations;
}

namespace meteo {
namespace product {


class FactWindStations : public QWidget
{
  Q_OBJECT

public:
  enum ColumnRole {
    kCheckDataColumn,
    kIndexColumn,
    kNameColumn,
    kHourColumn,
    kColumnCount
  };
  enum DataRole {
    kUserRole = Qt::UserRole,
    kShadowIndex,
    kFi,
    kLa
  };

  explicit FactWindStations(QWidget *parent = nullptr);
  ~FactWindStations();

  void addStation( const sprinf::Station& station, const QString& shadowIndex = QString() );
  void addStation( const surf::Point& station );
  void delStation( const QString& index );
  QList<surf::Point> getStationsList();
  bool contains( const QString& index );
  void setDt( const QDateTime& dt );
  void setFindNear( bool findNear) { findNear_ = findNear; }
  void setFindRadius( int radius ) { radius_ = radius; }
  void checkAllItems();

private slots:
  void slotClose();
  void slotAvailableStation();
  void slotLoadStations();
  void slotAddStation();
  void slotAddStation(surf::Point station);
  void slotDeleteStation();

private:
  void keyReleaseEvent(QKeyEvent *event);
  QString checkData( const QString& index );
  bool addNearStation( const QString& station, const QString& shadowIndex = QString() );

protected:
  void hideEvent(QHideEvent* event);

private:
  Ui::FactWindStations* ui_ = nullptr;
  QDateTime dt_;
  StationList* availableStations_ = nullptr;
  bool findNear_ = false;
  int radius_ = 500;
  bool humanChange_ = false;
};


}
}
#endif // METEO_PRODUCT_UI_AV12PLUGIN_FACTWINDSTATIONS_H
