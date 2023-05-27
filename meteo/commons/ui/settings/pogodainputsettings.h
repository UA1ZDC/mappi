#ifndef METEO_MASLO_UI_SETTINGS_POGODAINPUTSETTINGS_H
#define METEO_MASLO_UI_SETTINGS_POGODAINPUTSETTINGS_H

#include <qwidget.h>
#include <meteo/commons/ui/settings/stationselectform.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/pogodainput.pb.h>

namespace meteo {

class WgtPogodaInputSettings : public StationSelectForm
{
  Q_OBJECT

public:
  explicit WgtPogodaInputSettings(const meteo::settings::Location& loc,
                                  const QString& username,
                                  QWidget *parent = nullptr);

  const meteo::PogodaInputSts& getData();
  void setData(const meteo::PogodaInputSts& data);
protected:
  virtual QString stationListLabelText() override;
  virtual QList<sprinf::MeteostationType> stationTypes() override;
  virtual QList<sprinf::Station> loadStations() override;
  virtual bool saveStations(const QList<sprinf::Station>& stations) override;

private:
  const QString username_;
  meteo::PogodaInputSts data_;
};

}
#endif
