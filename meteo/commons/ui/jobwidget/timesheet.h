#ifndef TIMESHEET_H
#define TIMESHEET_H

#include <QDialog>
#include "meteo/commons/proto/weather.pb.h"

namespace Ui {
class TimeSheet;
}

class TimeSheet : public QDialog
{
  Q_OBJECT

public:
  explicit TimeSheet(QWidget *parent = nullptr);
  ~TimeSheet();
  QString sheet(){return sheet_;}
  void clear();
  void setSheet(const QString& sheet);
  void setPeriod(int period);
  void setAwait(int await);
  void setCron(int cron);
  int await() {return await_;};
  int cron() {return cron_;}
  int period() {return period_;};

private:
  Ui::TimeSheet *ui;
  QString sheet_;
  QString days_ = "*";
  QString weekDays_ = "*";
  QString minutes_ = "0";
  QString hours_ = "*";
  QString months_ = "*";
  int period_ = 60;
  int await_ = 0;
  int cron_ = 0;
  int awaitMin_ = 0;
  int awaitHour_ = 0;
  void makeWeek();
  void makeMinHour();
  void parse();

private slots:
  void slotChangeWeekDay();
  void slotOk();
  void slotCancel();
  void slotPeriod();
  void slotAwait();
  void slotCron();
};

#endif // TIMESHEET_H

