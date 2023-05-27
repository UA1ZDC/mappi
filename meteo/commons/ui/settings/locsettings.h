#ifndef METEO_UI_SETTINGS_LOCSETTINGS_H
#define METEO_UI_SETTINGS_LOCSETTINGS_H

#include "settingswidget.h"
#include <meteo/commons/proto/locsettings.pb.h>
#include <commons/geobasis/geopoint.h>

namespace Ui {
   class LocSettings;
}
namespace meteo {

class LocSettings : public SettingsWidget
{
  Q_OBJECT

public:
  explicit LocSettings(QWidget* parent = 0);
  ~LocSettings();
  meteo::settings::Location loc() const;

signals:
  void locationChanged(const meteo::settings::Location& loc);

private slots:
  void slotChangeState();
  void slotCorrectInput(const QString& text);

private:
  static bool loadLocationSettigns(meteo::settings::Location* loc);
  static bool writeLocationSettings(const meteo::settings::Location& loc);
  void save();
  void load();  
  bool setLoc(const meteo::settings::Location& loc);

  QString fioCmdr() const;
  int rankCmdr() const;
  QString locName() const;
  meteo::GeoPoint coord() const;
  QString locIndex() const;
  QString unitNumber() const;
  QString ipAddr() const;
  meteo::settings::NodeType stationType() const;


  void setFioCmdr(const QString& fio);
  void setRankCmdr(int rank);
  void setLocName(const QString& name);
  void setCoord(const meteo::GeoPoint coord);
  void setLocIndex(const QString& index);
  void setUnitNumber(const QString& number);
  void setIpAddr(const QString& ipAddr);
  void setStatonType(meteo::settings::NodeType stationtype);


private:
  Ui::LocSettings* ui_;
};

} // meteo

#endif // METEO_UI_SETTINGS_LOCSETTINGS_H
