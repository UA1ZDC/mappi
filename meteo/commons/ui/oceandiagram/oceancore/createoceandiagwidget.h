#ifndef METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_CREATEOCEANDIAGWIDGET_H
#define METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_CREATEOCEANDIAGWIDGET_H

#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <qdialog.h>

#include <meteo/commons/ocean/placedata.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/meteomenu.pb.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/oceandiagram/oceancore/oceandiagdata.h>
#include <meteo/commons/ui/oceandiagram/oceancore/oceandiag_doc.h>
#include <meteo/commons/ui/oceandiagram/oceantable/oceantablewidget.h>
#include <meteo/commons/ui/custom/geopointeditor.h>

class StationWidget;

class TreeWidgetNumberItem : public QTreeWidgetItem
{
public:
  TreeWidgetNumberItem(QTreeWidget* parent):QTreeWidgetItem(parent){}

private:
  bool operator<(const QTreeWidgetItem &other)const {
    int column = treeWidget()->sortColumn();
    return text(column).toInt() < other.text(column).toInt();
  }
};

namespace Ui {
class CreateOceanDiagForm;
}

namespace meteo {
namespace map {
  class Incut;
} // map
} // meteo

namespace meteo {
namespace odiag {
class OceanDiagDoc;
class OceanDiagData;
class InputForm;
} // odiag
} // meteo

namespace meteo {
namespace odiag {

enum StationRole {
  kStationNameRole  = Qt::DisplayRole,
  kStationCoordRole = Qt::UserRole + 1,
  kStationLatRole,
  kStationLonRole,
  kStationIndexRole,
};

class CreateOceanDiagWidget : public QDialog
{
  Q_OBJECT
public:

  enum Columns {
    kPlaceName = 0,
    kLat,
    kLon,
    kCount,
    kLevel,
    kDistance,
    kId
  };

  CreateOceanDiagWidget(  meteo::map::MapWindow* window );
  ~CreateOceanDiagWidget();

  void createDocument();

protected:
  bool eventFilter(QObject* watched, QEvent* event);

public slots:
  void slotRun();
  void slotCoordChanged();
  void slotUpdateAvailableData();
  void slotClearAll();
  void slotSdtCh();

  void setAngleSpin(float);
  void repaintChannel(double);
  void sortSrcDataTree();
  void slotPlaceChanged(int num, const GeoPoint& point, const QString& text);

  void slotOpenOceanTable();
  void slotFillOceanTable();

private:

  ocean::PlaceData* getProfile();
  QString makePlaceName() ;
  bool setIncut(meteo::map::Incut*);
  bool addStWidget();

  signals:
  void stationChanged(const GeoPoint& , const QString& ,bool);
  void currentDataChanged();

private:
  meteo::map::MapWindow* window_ = nullptr;
  meteo::odiag::OceanDiagDoc *odiagdoc_ = nullptr;
  meteo::odiag::OceanDiagData *odiagdata_ = nullptr;
  meteo::odiag::OceanTableWidget* oceanTable_ = nullptr;

  Ui::CreateOceanDiagForm* ui_;

  //    app::MainWindow* mainwindow_;

  ServiceType stype_;
  Type type_;

  StationWidget* station_widget_;
  QTreeWidgetItem* current_item_;
  QMap<QString, ocean::PlaceData> data_;

  QObject* watchedEventHandler_;
};

}
}
Q_DECLARE_METATYPE(ocean::PlaceData)

#endif // METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_CREATEOCEANDIAGWIDGET_H
