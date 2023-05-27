#ifndef METEO_MASLO_UI_SETTINGS_STATIONSELECTFORM_H
#define METEO_MASLO_UI_SETTINGS_STATIONSELECTFORM_H

#include <qwidget.h>
#include <meteo/commons/ui/settings/settingswidget.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/storm.pb.h>

namespace Ui {
class StationSelectForm;
}

namespace meteo {

class StationSelectForm : public SettingsWidget
{
  Q_OBJECT

public:
  explicit StationSelectForm(const meteo::settings::Location& loc,
                             QWidget *parent);
  ~StationSelectForm();

  virtual void save();
  virtual void load();

  void init();

protected:  
  virtual QString stationListLabelText() = 0;
  virtual QList<sprinf::MeteostationType> stationTypes() = 0;
  virtual QList<sprinf::Station> loadStations() = 0;
  virtual bool saveStations(const QList<sprinf::Station>& stations) = 0;
  const meteo::settings::Location& own();
  virtual QStringList additionalStationParams();
  virtual QWidget* getAdditionalColumnWidget(int index, const meteo::sprinf::Station& station);

  void stateChanged();  

public slots:
  void ownStationChanged(const meteo::settings::Location&);

private slots:
  void bntAddClicked();
  void bntSearchClicked();
  void slotStationWidgetChanged();
  void slotStationListSelectionChanged();
  void slotRemoveSelected();

private:

  void addStation(const meteo::sprinf::Station& st);
  void updateItemsTable();
  bool getSaveSettings(meteo::StationSelectForm* sett);

private:
  Ui::StationSelectForm *ui_;  
  meteo::settings::Location own_;
  QHash<QPair<QString, QString>, meteo::sprinf::Station> savedStations_;
};

}
#endif
