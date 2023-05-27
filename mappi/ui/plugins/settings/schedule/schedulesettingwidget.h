#ifndef MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SCHEDULESETTINGWIDGET_H
#define MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SCHEDULESETTINGWIDGET_H

#include <qmap.h>

#include <mappi/ui/plugins/settings/base/settingwidget.h>
#include <mappi/proto/reception.pb.h>

class QTableWidgetItem;

namespace Ui {
  class ScheduleSettingWidget;
}

namespace mappi {

class SatelliteEditor;

class ScheduleSettingWidget : public meteo::app::SettingWidget
{
  Q_OBJECT
  enum Column { kEnable, kName, kType, kUseles };

public:
  explicit ScheduleSettingWidget(QWidget *parent = nullptr);
  ~ScheduleSettingWidget();

public slots:
  void slotLoadConf();
  void slotSaveConf() const;

private slots:
  void slotToggleSat(int row, int col);
  void slotEditSat(int row, int col);
  void slotAddSat();
  void slotRemoveSat();
  void slotSaveSatellite(const conf::ReceptionParam& sat);

private:
  void loadSettings(const QString& filename);
  void saveSettings(const QString& filename) const;

private:
  Ui::ScheduleSettingWidget*    ui_;
  SatelliteEditor*              satelliteEditor_;
  QString                       editName_;
  int                           editRow_;
  QMap<QString, conf::ReceptionParam>  satellites_;
  bool                          loadedSettings_ = false;

private:
  static bool isUsesSatellite(const conf::ReceptionParam& sat, const conf::ScheduleConf& sched);
  static QTableWidgetItem* createCell(Column col, const conf::ReceptionParam& sat, bool isUses = false);
  static void fillCell(QTableWidgetItem* item, Column col, const conf::ReceptionParam& sat, bool isUses = false);
};

} //mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_SCHEDULE_SCHEDULESETTINGWIDGET_H
