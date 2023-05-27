#ifndef METEO_COMMONS_UI_aerowidget_H
#define METEO_COMMONS_UI_aerowidget_H

#include <qdialog.h>

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/meteomenu.pb.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/aerodocument.h>
#include <meteo/commons/ui/aero/diagram/aerodiagdata.h>
#include <meteo/commons/zond/placedata.h>
#include <meteo/commons/ui/aero/verticalcut/verticalcutaction.h>

#include <qstandarditemmodel.h>
class StationWidget;

namespace Ui {
  class CreateAeroDiagForm;
}

namespace meteo {
  namespace aero {
    class AeroTableWidget;
  }
}

namespace meteo {
  class StationList;

  namespace adiag {
  class AeroDiagDoc;
  class AeroDiagData;
  class AeroIndexes;
  class InputForm;

  class CreateAeroDiagWidget : public QWidget
{
  Q_OBJECT
  public:

    enum Columns { /*kTypeName,*/ kPlaceName, kPlaceCoord, kDataDate, /*kNumber, */kData };


   CreateAeroDiagWidget( meteo::map::MapWindow* parent );
  ~CreateAeroDiagWidget();

  void createDocument();
  void setStationListWidget(meteo::StationList* stl);

  meteo::map::MapWindow* adiagwindow() const {return window_;}
  zond::PlaceData currentPlaceData() const;
  const zond::PlaceData& currentDocData() const { return document_->data(); }

  //  ServiceType sourceDataMode() { return stype_; }
  StationWidget *stationWidget();

  protected:
  bool eventFilter(QObject* watched, QEvent* event);
  bool addStWidget();

  void fillCenters();


  public slots:
    void slotRun();
    void slotRunByCheck(bool);
    void slotRunFromList();

    void slotSdtCh();
    void slotCenterCh(int cc);
  //    void slotTypeCh(int cc);
    void slotServiceCh(int cc);
    void stationChanged();

    void slotOpenAeroTable();
    void slotOpenIndexes();
    void slotFillAeroTable();

  void setDateTime(const QDateTime& dt);

  private slots:
    void slotOnMap(bool isOnMap);
    void onMapActionDestroyed(QObject* obj);

  private:
    QString makePlaceName() const ;
    bool setIncut(meteo::map::Incut*);

    meteo::map::MapWindow* window_ = nullptr;
    meteo::map::AeroDocument* document_ = nullptr;
    meteo::adiag::AeroDiagData* adiagdata_ = nullptr;
    meteo::StationList* stationList_ = nullptr;

    Ui::CreateAeroDiagForm* ui_ = nullptr;


    app::MainWindow* mainwindow_ = nullptr;
    meteo::aero::AeroTableWidget* aeroTable_ = nullptr; //!< Таблица данных зондирования
    meteo::adiag::AeroIndexes* aeroIndexes_  = nullptr; //!< Виджет с индексами неустройчивости
    ServiceType stype_;
  //   Type type_;

    zond::PlaceData current_data_; //!< Текущие данные зондирования

    QObject* watchedEventHandler_ = nullptr;
    meteo::map::VerticalCutAction* mapAction_ = nullptr;

signals:
  void currentDataChanged(const QString& station);
  void setStationMode(bool);
};

}
}
Q_DECLARE_METATYPE(zond::PlaceData)

#endif
