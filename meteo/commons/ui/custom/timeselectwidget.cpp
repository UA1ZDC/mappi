#include "timeselectwidget.h"
#include "ui_timeselectwidget.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

TimeSelectWidget::TimeSelectWidget(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::TimeSelectWidget)
{
  ui_->setupUi(this);
  connect(ui_->hourBtn, SIGNAL(clicked()), SLOT(slotHourClicked()));
  connect(ui_->minBtn, SIGNAL(clicked()), SLOT(slotMinuteClicked()));
  hourMenu_ = new QMenu(this);
  connect(hourMenu_, SIGNAL(triggered(QAction*)), SLOT(slotHourMenuActivated(QAction*)));
  for( int i = 0; i < 24; i++ ){
    hourMenu_->addAction(QString("%1").arg(i, 2, 10, QLatin1Char('0')));
  }

  minuteMenu_ = new QMenu(this);
  connect(minuteMenu_, SIGNAL(triggered(QAction*)), SLOT(slotMinuteMenuActivated(QAction*)));
  for( int i = 0; i < 60; i += 5 ){
    minuteMenu_->addAction(QString("%1").arg(i, 2, 10, QLatin1Char('0')));
  }
  minuteMenu_->addAction(QString("%1").arg(59, 2, 10, QLatin1Char('0')));

}

QString TimeSelectWidget::hour() const
{
  return ui_->hourBtn->text();
}

QString TimeSelectWidget::minute() const
{
  return ui_->minBtn->text();
}

void TimeSelectWidget::setHour(const QString& hour)
{
  ui_->hourBtn->setText(hour);
}

void TimeSelectWidget::setMinute(const QString& minute)
{
 ui_->minBtn->setText(minute);
}

void TimeSelectWidget::slotHourClicked()
{
  hourMenu_->exec(QCursor::pos());
}

void TimeSelectWidget::slotMinuteClicked()
{
  minuteMenu_->exec(QCursor::pos());
}

void TimeSelectWidget::slotHourMenuActivated(QAction* act)
{
  ui_->hourBtn->setText(act->text());
}

void TimeSelectWidget::slotMinuteMenuActivated(QAction* act)
{
  ui_->minBtn->setText(act->text());
}
