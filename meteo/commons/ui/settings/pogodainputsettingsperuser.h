#ifndef POGODAINPUTSETTINGSPERUSER_H
#define POGODAINPUTSETTINGSPERUSER_H

#include <qwidget.h>
#include <meteo/commons/ui/settings/settingswidget.h>
#include <meteo/commons/ui/settings/pogodainputsettings.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/pogodainput.pb.h>

namespace Ui {
class PogodaInputSettingsPerUser;
}

namespace meteo {

  class PogodaInputSettingsPerUser : public SettingsWidget
  {
    Q_OBJECT

  public:
    explicit PogodaInputSettingsPerUser( const meteo::settings::Location& loc,
                                         QWidget *parent);
    virtual ~PogodaInputSettingsPerUser() override;

    virtual void init() override;
    virtual void save() override;
    virtual void load() override;

  public slots:
    void ownStationChanged(const meteo::settings::Location& loc);


  private slots:
    void onUserChanged(int index);
    void slotChanged();

    bool saveRpc();

  private:
    Ui::PogodaInputSettingsPerUser *ui_;
    meteo::settings::Location loc_;
    QHash<QString, WgtPogodaInputSettings*> settings_;
  };

}
#endif // POGODAINPUTSETTINGSPERUSER_H
