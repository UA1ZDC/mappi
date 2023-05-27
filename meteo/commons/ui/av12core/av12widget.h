#ifndef METEO_COMMONS_UI_AV12CORE_AV12WIDGET_H
#define METEO_COMMONS_UI_AV12CORE_AV12WIDGET_H

#include <qevent.h>
#include <qdatetimeedit.h>
#include <qfilesystemwatcher.h>
#include <qprocess.h>
#include <qwidget.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qwidgetaction.h>

//#include <meteo/product/proto/documentservice.pb.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/ui/custom/selectstation.h>
#include <meteo/commons/ui/custom/forecastformwidget.h>
#include <meteo/commons/global/formaldocsaver.h>


class QTableWidgetItem;

namespace Ui {
  class Av12Widget;
}

namespace meteo {

class UserSelectForm;

namespace product {

class FactWindStations;


class Av12Widget: public QWidget
{
  Q_OBJECT
public:
  enum BillType {
    kVRP,
    kPolet,
    kPerelet,
    kBattle
  };
  enum DataRole {
    kUserRole = Qt::UserRole,
    kFi,
    kLa
  };
  enum column {
    kRole,
    kIndex,
    kName,
    kTime,
    kEchelon
  };

  enum WidgetType {
    kAv12,
    kAv12d
  };

public:
  Av12Widget(QWidget* parent = nullptr, WidgetType type = kAv12);
  ~Av12Widget();
  //void setUsersVisible(bool visible);

private:
  void appendStations(meteo::map::proto::av12script* av12);
  void keyReleaseEvent(QKeyEvent* event);
  void addStation(QTableWidgetItem* roleItem, QString time);
  surf::Point* getDeparture();
  surf::Point* getArrival();
  surf::Point* getStationByRole( QString role );
  void loadStations();
  bool isInterAvailable();
  void setVisibleUserBtn( bool visible );

private slots:
  void slotCreateAv12();
  void slotChangeType(int type);
  void slotOfficeClosed();
  void slotRemoveStation();
  void slotEditTime();
  void slotCheckTime(int row, int column);
  void slotAddOnRouteStation();
  void slotAddDepartureStation();
  void slotAddArrivalStation();
  void slotAddReservStation();
  void slotShowMenu(QPoint point);
  void slotSaveState();
  void slotLoadState();
  void slotTimeDepartureChanged(QTime time);
  void slotTimeLandingChanged(QTime time);
  void slotDateChanged();
  void slotShowForecastDlg();
  void slotChangeEnableDopSign();

  void slotSetTextFWA(QString newValue);
  void slotSetTextFWR(QString newValue);
  void slotSetTextOA(QString newValue);
  void slotSetTextOR(QString newValue);
  void slotSetTextFWAAv12d(QString newValue);
  void slotSetTextTFWAv12d(QString newValue);
  void slotShowSelectUsers();
  void slotChangeFindStation();
  void slotChangeRadius();
  void slotShowFactWindForm();

protected:
  void hideEvent(QHideEvent *event);

private:
  Ui::Av12Widget* ui_ = nullptr;
  BillType type_;
  WidgetType widgetType_;
  QStringList* roleList_ = nullptr;
  QStringList* indexList_ = nullptr;
  QStringList* nameList_ = nullptr;
  QMenu* menu_ = nullptr;
  QMenu* radiusMenu_ = nullptr;
  QSettings* settings_ = nullptr;
  QAction* removeAct_ = nullptr;
  QAction* editAct_ = nullptr;
  QSpinBox* radiusBox_ = nullptr;
  QWidgetAction* radiusAct_ = nullptr;
  meteo::map::proto::Map map_;
  ForecastFormWidget* forecastForm_ = nullptr;
  meteo::UserSelectForm* usersWidget_ = nullptr;
  meteo::FormalDocSaver formaldoc_;
  FactWindStations* factWind_ = nullptr;
  QString stDeparture_;
  QString stArrival_;


};

}
}

#endif // METEO_PRODUCT_UI_AV12PLUGIN_AV12WIDGET_H
