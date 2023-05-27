#ifndef METEO_NOVOST_UI_PLUGINS_CITYPLUGIN_DISPLAYSETTINGSEDITDIALOG_H
#define METEO_NOVOST_UI_PLUGINS_CITYPLUGIN_DISPLAYSETTINGSEDITDIALOG_H

#include <qdialog.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/map_city.pb.h>

namespace Ui {
class DisplaySettingsEditDialog;
}

namespace meteo {

class DisplaySettingsEditDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DisplaySettingsEditDialog(QWidget *parent = 0);
  virtual ~DisplaySettingsEditDialog();

  void setCitySettings(meteo::map::proto::CitySetting citySetting);
  void setDisplaySetting(meteo::map::proto::DisplaySetting displaySetting);
  QString getCityName();
  meteo::map::proto::CitySetting getCitySetting();
  meteo::map::proto::DisplaySetting getDisplaySetting();

private slots:
  void slotToggleWidgetsVisibility();
  void slotRequestCoordChnged(bool b);
  void slotFocusWidgetChanged(QWidget* old, QWidget* now);
  void slotUpdateCoord(const GeoPoint& coord);

private:
  Ui::DisplaySettingsEditDialog* ui_;

  meteo::map::proto::CitySetting cs_;
  meteo::map::proto::DisplaySetting ds_;

  bool coordUpdateOn_;
};

} // meteo

#endif // METEO_NOVOST_UI_PLUGINS_CITYPLUGIN_DISPLAYSETTINGSEDITDIALOG_H
