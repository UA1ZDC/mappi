#ifndef METEO_COMMONS_UI_CUSTOM_TIMESHEETWGT_H
#define METEO_COMMONS_UI_CUSTOM_TIMESHEETWGT_H

#include <QWidget>
#include <meteo/commons/ui/custom/multichoosewgt.h>
#include <qlabel.h>

const QString kDefaultTimesheetWgt = "* * * * *";

class TimesheetWgt : public QWidget
{
  Q_OBJECT
public:
  explicit TimesheetWgt(QWidget *parent = 0);
  ~TimesheetWgt();
  QString timesheet();
  bool setTimesheet(const QString& line) const;
  static bool parseTimesheet(const QString& line);

signals:
  void changed();
public slots:

private:
  MultiChooseWgt* hour_;
  MultiChooseWgt* minute_;
};

#endif // TIMESHEETWGT_H
