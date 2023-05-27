#ifndef METEO_COMMONS_UI_OBANALSETTINGS_OBANALOPTIONS_H
#define METEO_COMMONS_UI_OBANALSETTINGS_OBANALOPTIONS_H

#include <QDialog>
#include <QString>
#include <QScopedPointer>

class QLineEdit;
template<typename T> class QList;

namespace Ui{
   class ObanalOptions;
}

namespace meteo {
namespace obanalsettings {
  class Task;
} // obanalsettings
} // novost

namespace meteo {

class Timesheet;

class ObanalOptions : public QDialog
{
  Q_OBJECT

public:
  explicit ObanalOptions(QWidget* parent = 0);
  ~ObanalOptions();

  void init(const meteo::obanalsettings::Task* const config, const QString* const timesheet = 0);
  meteo::obanalsettings::Task config() const;
  QString timesheet() const;

private slots:
  void slotSwitchStationsAndCenters(bool enabled);
  void slotChangeTimesheet();
  void slotChangeTimesheet(const QString& str);
  void slotSwitchSheduleMode(bool isAdvanced);

  void slotClearLevels();
  void slotClearDescriptors();
  void slotClearCenters();
  void slotClearStations();

  void slotSetupLevels();
  void slotSetupCenters();
  void slotSetupStations();
  void slotSetupDescriptors();
  void slotSetupTimesheet();

private:
  void setDefaultValue(QLineEdit* target);
  QString levelsFromTask(const meteo::obanalsettings::Task& config) const;
  QString descriptorsFromTask(const meteo::obanalsettings::Task& config) const;
  QString centersFromTask(const meteo::obanalsettings::Task& config) const;
  QString stationsFromTask(const meteo::obanalsettings::Task& config) const;
  Timesheet timesheetFromString(const QString& str) const;
  int gribHourFromTask(const meteo::obanalsettings::Task& config) const;

  void updateTimesheetBasic(int hour);
  void updateTimesheetAdvanced();
  int basicTimesheetHour(const Timesheet& ts) const;

  int typeToTask() const;
  QList<int> levelsToTask() const;
  QList<int> descriptorsToTask() const;
  QList<int> centersToTask() const;
  QList<int> stationsToTask() const;
  QString timesheetToString() const;
  int gribHourToTask() const;

  QList<int> valuesToTask(const QString& values) const;

private:
  Ui::ObanalOptions* ui_;
  QScopedPointer<Timesheet> currentTimesheet_;

};

} // meteo

#endif // METEO_COMMONS_UI_OBANALSETTINGS_OBANALOPTIONS_H
