#ifndef SELECTDATEWIDGET_H
#define SELECTDATEWIDGET_H

#include <QtWidgets>

namespace Ui{
  class SelectDateWidget;
}

enum Period{
  today     = 0,
  yesterday = 1,
  days2     = 2,
  days3     = 3,
  week      = 4
};

class SelectDateWidget : public QDialog
{
  Q_OBJECT
public:
  explicit SelectDateWidget(QWidget *parent = 0);
  ~SelectDateWidget();
  QString condition() const;
  QDateTime dt_start() const;
  QDateTime dt_end() const;
private:
  Ui::SelectDateWidget* ui_;

signals:

private slots:
  void slotSelectPeriod();
  void slotSelectStartTime();
  void slotSelectEndTime();
};

#endif // SELECTDATEWIDGET_H
