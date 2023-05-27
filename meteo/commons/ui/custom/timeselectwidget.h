#ifndef TIMESELECTWIDGET_H
#define TIMESELECTWIDGET_H

#include <QtWidgets>

namespace Ui{
  class TimeSelectWidget;
}

class TimeSelectWidget : public QDialog
{
  Q_OBJECT
public:
  TimeSelectWidget(QWidget *parent = 0);
  QString hour() const;
  QString minute() const;
  void setHour(const QString& hour);
  void setMinute(const QString& minute);
private:
  Ui::TimeSelectWidget* ui_;
  QMenu* hourMenu_;
  QMenu* minuteMenu_;

private slots:
  void slotHourClicked();
  void slotMinuteClicked();
  void slotHourMenuActivated(QAction*);
  void slotMinuteMenuActivated(QAction*);

signals:
};

#endif
