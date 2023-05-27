#include "timeselectwidget.h"
#include "selectdatewidget.h"
#include "ui_selectdatewidget.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/dateformat.h>

namespace {
  const QString dtCondition() { return QString(" \"$and\" : "
                                               "[  { \"%3\" : { \"$gte\" : { \"$date\" : \"%1\" } } },"
                                               " { \"%3\" : { \"$lte\" : { \"$date\" : \"%2\" } } } ]"); }
}

SelectDateWidget::SelectDateWidget(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::SelectDateWidget)
{
  ui_->setupUi(this);
  ui_->start_date->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  ui_->end_date->setDisplayFormat(meteo::dtHumanFormatDateOnly);

  ui_->start_date->setDate(QDate::currentDate());
  ui_->end_date->setDate(QDate::currentDate());
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Принять"));
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Отмена"));
  connect(ui_->periodBox, SIGNAL(currentIndexChanged(int)), SLOT(slotSelectPeriod()));
  connect(ui_->start_time, SIGNAL(clicked()), SLOT(slotSelectStartTime()));
  connect(ui_->end_time, SIGNAL(clicked()), SLOT(slotSelectEndTime()));
}

SelectDateWidget::~SelectDateWidget()
{
  delete this->ui_;
}

QString SelectDateWidget::condition() const
{
  return ::dtCondition().arg(ui_->start_date->date().toString("yyyy-MM-ddT") + ui_->start_time->text().remove('&') + ":00Z")
      .arg(ui_->end_date->date().toString("yyyy-MM-ddT") + ui_->end_time->text().remove('&') + ":00Z");
}

QDateTime SelectDateWidget::dt_start() const
{
  return QDateTime(ui_->start_date->date(), QTime::fromString(ui_->start_time->text().remove('&')));
}

QDateTime SelectDateWidget::dt_end() const
{
  return QDateTime(ui_->end_date->date(), QTime::fromString(ui_->end_time->text().remove('&')));
}

void SelectDateWidget::slotSelectPeriod()
{
  switch( ui_->periodBox->currentIndex() ){
    case today : {
      ui_->start_date->setDate(QDate::currentDate());
      ui_->end_date->setDate(QDate::currentDate());
      break;
    }
    case yesterday : {
      ui_->start_date->setDate(QDate::currentDate().addDays(-1));
      ui_->end_date->setDate(QDate::currentDate().addDays(-1));
      break;
    }
    case days2 : {
      ui_->start_date->setDate(QDate::currentDate().addDays(-1));
      ui_->end_date->setDate(QDate::currentDate());
      break;
    }
    case days3 : {
      ui_->start_date->setDate(QDate::currentDate().addDays(-3));
      ui_->end_date->setDate(QDate::currentDate());
      break;
    }
    case week : {
      ui_->start_date->setDate(QDate::currentDate().addDays(-7));
      ui_->end_date->setDate(QDate::currentDate());
      break;
    }
  }
}

void SelectDateWidget::slotSelectStartTime()
{
  TimeSelectWidget* w = new TimeSelectWidget(this);
  w->setHour(ui_->start_time->text().split(":").at(0));
  w->setMinute(ui_->start_time->text().split(":").at(1));
  if( w->exec() == QDialog::Accepted ){
    ui_->start_time->setText(QString("%1:%2").arg(w->hour().remove("&")).arg(w->minute().remove("&")));
  }
}

void SelectDateWidget::slotSelectEndTime()
{
  TimeSelectWidget* w = new TimeSelectWidget(this);
  w->setHour(ui_->end_time->text().split(":").at(0));
  w->setMinute(ui_->end_time->text().split(":").at(1));
  if( w->exec() == QDialog::Accepted ){
    ui_->end_time->setText(QString("%1:%2").arg(w->hour().remove("&")).arg(w->minute().remove("&")));
  }
}
