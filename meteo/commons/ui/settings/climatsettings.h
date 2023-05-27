#ifndef METEO_MASLO_UI_SETTINGS_CLIMATSETTINGSNEW_H
#define METEO_MASLO_UI_SETTINGS_CLIMATSETTINGSNEW_H

#include <qwidget.h>
#include <meteo/commons/ui/settings/stationselectform.h>

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/climat.pb.h>

namespace meteo {
  class ClimatSettings: public StationSelectForm {
    Q_OBJECT
  public:
    explicit ClimatSettings(const meteo::settings::Location& loc,
                               QWidget *parent = nullptr);

  protected:
    virtual QString stationListLabelText() override;
    virtual QList<sprinf::MeteostationType> stationTypes() override;
    virtual QList<sprinf::Station> loadStations() override;
    virtual bool saveStations(const QList<sprinf::Station> &stations) override;

    QStringList additionalStationParams() override;
    QWidget* getAdditionalColumnWidget(int index, const meteo::sprinf::Station& station) override;


  private slots:
    void onUtcComboChanged(int value);

  private:
    QHash<QString, int> stationsUtcs_;

  };
}

#endif
