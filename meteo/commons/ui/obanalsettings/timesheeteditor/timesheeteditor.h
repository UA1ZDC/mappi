#ifndef METEO_NOVOST_UI_TIMESHEETSETUP_H
#define METEO_NOVOST_UI_TIMESHEETSETUP_H

#include <QDialog>
#include <QScopedPointer>

class QTabWidget;

namespace Ui {
  class TimesheetSetupWidget;
}

namespace meteo {

class Timesheet;

class TimesheetEditor : public QDialog
{
  Q_OBJECT

  enum every_t {
    EVERY = 0,
    EVERY_N,
    CUSTOM
  };

public:
  explicit TimesheetEditor(QWidget* parent = 0);
  ~TimesheetEditor();

  void init(const Timesheet& ts);
  Timesheet timesheet() const;

private slots:
  void slotClearTimesheet();
  void slotEveryTypeChange(bool toggled);

  void slotSetMinute();
  void slotSetMinute(bool);
  void slotSetMinute(int minute);
  void slotSetHour();
  void slotSetHour(int hour);
  void slotSetHour(bool);
  void slotSetDay();
  void slotSetDay(bool);
  void slotSetMonth();
  void slotSetMonth(bool);
  void slotSetDayOfWeek();
  void slotSetDayOfWeek(bool);

private:
  void setTimesheet(const Timesheet& ts);
  void updatePages(const Timesheet& ts);
  void updatePageWith3Control(QWidget* targetPage, const QString& timeSheetPart);
  void updatePageWith2Control(QWidget* targetPage, const QString& timeSheetPart);
  void connectSignalsToSlots();

private:
  Ui::TimesheetSetupWidget* ui_;
  QScopedPointer<Timesheet> timesheet_;

};

} // meteo

#endif // METEO_NOVOST_UI_TIMESHEETSETUP_H
