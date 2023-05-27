#ifndef METEO_MASLO_UI_SETTINGS_STORMSETTINGS_H
#define METEO_MASLO_UI_SETTINGS_STORMSETTINGS_H

#include <qwidget.h>
#include <meteo/commons/ui/settings/stationselectform.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/storm.pb.h>

namespace meteo {

class WgtStormSettings : public StationSelectForm
{
  Q_OBJECT

public:
  explicit WgtStormSettings(const meteo::settings::Location& loc,
                             QWidget *parent = nullptr);

protected:
  virtual QString stationListLabelText() override;
  virtual QList<sprinf::MeteostationType> stationTypes() override;
  virtual QList<sprinf::Station> loadStations() override;
  virtual bool saveStations(const QList<sprinf::Station> &stations) override;
};

}
#endif
