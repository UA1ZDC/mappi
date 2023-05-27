#ifndef METEO_COMMONS_UI_VERTICALCUT_CORE_CREATECUTWIDGET_H
#define METEO_COMMONS_UI_VERTICALCUT_CORE_CREATECUTWIDGET_H

#include <qstandarditemmodel.h>
#include <qdialog.h>

#include <meteo/commons/proto/meteomenu.pb.h>
#include <meteo/commons/ui/map/vprofiledoc.h>
#include <meteo/commons/ui/aero/table/aerotablewidget.h>

class StationWidget;
class QTreeWidgetItem;
namespace Ui {
class Form;
}

namespace meteo {
  namespace app {
  class MainWindow;
  }
  namespace map {
    class MapWindow;
    class MapScene;
    class VerticalCutAction;
    class MapView;
  } // map

class CreateCutWidget : public QDialog
{
  Q_OBJECT
public:
  enum Columns {
    kPlaceName,
    kNumber,
    kPlaceCoord,
    kDate,
    kFi,
    kLa,
    kAlt
  };

  enum ServiceType {
    kSrcData,
    kFieldData
  };

  CreateCutWidget( meteo::map::MapWindow* parent );
  virtual ~CreateCutWidget();


  meteo::map::MapWindow* cutWindow() const { return window_; }
  virtual void setVisible(bool visible);

  void setCutDoc( meteo::map::VProfileDoc* doc );
  void setCutWindow( meteo::map::MapWindow* w );
  void initMenu( meteo::map::MapWindow* w );
  bool addStWidget();
  int  currentCenter();

signals:

  void placeChanged(int num, const GeoPoint& coord, const QString& title );
  void placeRemoved(int);
  void placeAdded(int, const GeoPoint&, const QString&);
  void currentDataChanged();

public slots:
  void slotRun();

  void slotAddPlace();
  void slotRemovePlace();
  void slotTraceItemAdded(int num, const GeoPoint& coord);
  void slotTraceItemChanged(int num, const GeoPoint& coord);
  void slotTraceItemRemoved(int num);
  void slotClearAll();

  void slotMapBtnToggled(bool enable);

  void slotStartDtChanged();
  void slotEndDtChanged();

  void slotRequestFields();
  void slotEditName(QTreeWidgetItem* item, int column);

  void slotCenterChanged(int index);
  void slotTypeChanged(int index);
  void slotServiceChanged(int index);

  void slotWindowClosed();
  void slotActionDestroyed();

  void slotOpenAeroTable();
  void slotFillAeroTable();

private slots:
  void onItemMoved();

  void addSceneAct(meteo::map::MapView* view);
  void removeSceneAct();
  void updatePath();

  void fillItem(QTreeWidgetItem* item, const zond::PlaceData& adata);

  //! Добавляет пункт в список.
  int addPlace( const zond::PlaceData& data);

  //! Изменяет значение пункта с номером number на data. Если в списке нет элемента с заданным значением, будет добавлен
  //! новый элемент в конец списка.
  int setPlaceData(QTreeWidgetItem* item, const zond::PlaceData& data);

  //! Генерирует название для пункта.
  QString makePlaceName(const zond::PlaceData& data, int num) const;

  bool getProfiles(QList<zond::PlaceData>*);
  QString makeTitle(const QList<zond::PlaceData>& profiles) const;

  void fillCenterList();
  void updateDate();
  void updateFieldDate();
  void updateSrfDate();



  surf::DateReply* getZondDates(QString index);
 // field::DateReply* getFieldDates(QTreeWidgetItem* item);

  void loadSettings();
  void saveSettings() const;
  bool setIncut(meteo::map::Incut *incut, const meteo::map::proto::Map& );

private:
  // параметры
  ServiceType srvType_;
  meteo::map::Type cutType_;
  int center_ = -1;

  // данные
  meteo::map::VProfileDoc *vdoc_;
  meteo::field::ManyFieldsForDatesResponse* resp_;
  QDateTime sdt_;
  QDateTime edt_;

  // служебные
  Ui::Form* ui_ ;
  app::MainWindow* mainwindow_;
  meteo::map::MapWindow* window_;
  meteo::map::MapScene* scene_;
  StationWidget* station_widget_;
  meteo::map::VerticalCutAction* act_;
  meteo::aero::TabAeroTableWidget* aeroTable_; //!< Таблица данных зондирования
  QList<field::DataRequest> centerTail_;
  QList<zond::PlaceData> profilesData_;

};

}

#endif
