#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTPLUGIN_DOCUMENTPARAMS_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTPLUGIN_DOCUMENTPARAMS_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QKeyEvent>

#include <meteo/commons/settings/settings.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/ui/custom/selectstation.h>
#include <meteo/commons/ui/custom/forecastformwidget.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/global/formaldocsaver.h>

namespace Ui {
class DocumentParams;
}
namespace meteo {
class UserSelectForm;
}

namespace meteo {


enum DocumentType {
  kSimple,
  kForecasts,
  kBulletin,
  kStorm,
  kAv6,
  kFactUnderlying,
  kOnlyDate,
  kStormForecast,
  kGidroinfo
};

enum PythonScript {
  kMeteosummary,
  kForecast,
  kMeteo11,
  kMeteo44,
  kWindDoc,
  kAV6,
  kLayer,
  kStormAlert,
  kForecastWindRegion,
  kFactConditionUnderlying,
  kHSPrirodaVoiska,
  kFactyavleniavoiska,
  kStormvoiska,
  kSharopilot,
  kGidrometeotech,
  kGidrospravka,
  kFireHazardReport,
  kHydroMeteoReport,
  kFactCondition,
  kForecastCondition,
  kFloodSituation,
  kDokladOperBoi,
  kOprppsp,
  kGidrometeomerop,
  kZaprosgmiwar,
  kZaprosgmimir,
  kShortConditionForecast
};

class DocumentParams : public QWidget
{
  Q_OBJECT

public:
  explicit DocumentParams(QWidget* parent = 0);
  ~DocumentParams();

  void setup(int script, int type = kSimple) {script_ = script; type_ = type; }



private:
  void loadAvailableOceanStations();
  bool isInterAvailable();
  void setVisibleUserBtn( bool visible );
  void keyReleaseEvent(QKeyEvent* event);

private:
  int script_;         //!тип сценария
  int type_ = kSimple; //!тип документа
  Ui::DocumentParams* ui_ = nullptr;
  QString docName_; //!название вида документа (Например: Дневник погоды)
  meteo::UserSelectForm* usersWidget_ = nullptr;

  SelectStation* selectStation_;
  ForecastFormWidget* forecastForm_ = nullptr;
  QMap< QString, meteo::map::proto::Map > formals_;

  bool synMobSea_ = false;
  meteo::FormalDocSaver formaldoc_;

public slots:
  void show();

private slots:
  void slotOnCreateButtonClicked();
  void slotArbitraryEdited();
  void slotShowForecastForm();
  void slotClearStormText();
  void slotOnStormText(QString text);
  void slotOnComplete();
  void slotHydroChanged(int index);
  void slotDateTimeChanged(const QDateTime& datetime);
  void slotShowSelectUsers();
};

}

#endif
