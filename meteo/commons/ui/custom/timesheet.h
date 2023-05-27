#ifndef METEO_COMMONS_UI_CUSTOM_TIMESHEET_H
#define METEO_COMMONS_UI_CUSTOM_TIMESHEET_H

#include <QWidget>
#include <meteo/commons/ui/custom/multichoosebtn.h>
#include <qlabel.h>

const QString kDefaultTimesheet = "* * * * *";

class Timesheet : public QWidget
{
  Q_OBJECT
public:
  explicit Timesheet(QWidget *parent = 0);
  ~Timesheet();
  QString timesheet();
  bool setTimesheet(const QString& line) const;
  static bool parseTimesheet(const QString& line);

signals:

public slots:

private:
  MultiChooseBtn* hourBtn_;
  MultiChooseBtn* minuteBtn_;
};

#endif // TIMESHEET_H
