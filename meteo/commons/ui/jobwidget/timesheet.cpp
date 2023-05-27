#include "timesheet.h"
#include "ui_timesheet.h"
#include "cross-commons/debug/tlog.h"

TimeSheet::TimeSheet(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::TimeSheet)
{
  ui->setupUi(this);
  QObject::connect(ui->weekTable, SIGNAL(itemSelectionChanged()), SLOT(slotChangeWeekDay()));
  QObject::connect(ui->okBtn, SIGNAL(clicked()), SLOT(slotOk()));
  QObject::connect(ui->cancelBtn, SIGNAL(clicked()), SLOT(slotCancel()));
  QObject::connect(ui->periodBox, SIGNAL(currentIndexChanged(int)), SLOT(slotPeriod()));
  QObject::connect(ui->awaitBox, SIGNAL(valueChanged(int)), SLOT(slotAwait()));
  QObject::connect(ui->cronBox, SIGNAL(valueChanged(int)), SLOT(slotCron()));

  this->setWindowTitle("Расписание");

  ui->periodBox->addItem(QObject::tr("10 минут"), 10);
  ui->periodBox->addItem(QObject::tr("20 минут"), 20);
  ui->periodBox->addItem(QObject::tr("30 минут"), 30);
  ui->periodBox->addItem(QObject::tr("1 час"), 60);
  ui->periodBox->addItem(QObject::tr("2 часа"), 120);
  ui->periodBox->addItem(QObject::tr("3 часа"), 180);
  ui->periodBox->addItem(QObject::tr("6 часов"), 360);
  ui->periodBox->addItem(QObject::tr("12 часов"), 720);
  ui->periodBox->addItem(QObject::tr("24 часа"), 1440);

  ui->periodBox->setCurrentIndex(3);
}

TimeSheet::~TimeSheet()
{
  delete ui;
}

void TimeSheet::clear()
{
  ui->weekTable->clearSelection();
  ui->awaitBox->setValue(0);
  ui->periodBox->setCurrentIndex(3);
  days_ = "*";
  weekDays_ = "*";
  minutes_ = "0";
  hours_ = "*";
  months_ = "*";
  period_ = 60;
  await_ = 0;
  awaitMin_ = 0;
  awaitHour_ = 0;
}

void TimeSheet::setSheet(const QString &sheet)
{
  QStringList fields = sheet.split(QRegExp("[\t ]+"), QString::SkipEmptyParts);
  if ( fields.size() < 5 ) {
    return;
  }

  minutes_ = fields.at(0);
  hours_ = fields.at(1);
  days_ = fields.at(2);
  months_ = fields.at(3);
  weekDays_ = fields.at(4);
  parse();
}

void TimeSheet::setPeriod(int period)
{
  period_ = period;
  for ( int i = 0, sz = ui->periodBox->count(); i < sz; ++i ) {
    if ( ui->periodBox->itemData(i) == period_ ) {
      ui->periodBox->setCurrentIndex(i);
      break;
    }
  }
}

void TimeSheet::setAwait(int await)
{
  await_ = await;
  ui->awaitBox->setValue(await_);
}

void TimeSheet::setCron(int cron)
{
  cron_ = cron;
  ui->cronBox->setValue(cron_);
}

void TimeSheet::makeWeek()
{
  if ( true == ui->weekTable->selectedItems().isEmpty() || ui->weekTable->selectedItems().size() == ui->weekTable->columnCount() ) {
    weekDays_ = "*";
    return;
  }
  QStringList weekDays;
  for ( auto item : ui->weekTable->selectedItems() )
  {
    weekDays.append(QString::number(item->column() + 1));
  }
  weekDays.sort();
  if ( weekDays.last().toInt() - weekDays.first().toInt() + 1 == weekDays.size() ) {
    weekDays_ = weekDays.first() + "-" + weekDays.last();
  }
  else {
    weekDays_ = weekDays.join(",");
  }
}

void TimeSheet::makeMinHour()
{
  if ( period_ < 60 ) {
    QList<int> minutes;
    int m = 0;
    while ( m < 60 ) {
      minutes << m;
      m += period_;
    }
    QStringList str;
    for ( auto min : qAsConst(minutes) )
    {
      min = min + awaitMin_;
      min = min%60;
      str << QString::number(min);
    }
    minutes_ = str.join(",");
    hours_ = "*";
  }
  else if ( 60 == period_ ) {
    minutes_ = QString("%1").arg(awaitMin_);
    hours_ = "*";
  }
  else {
    minutes_ = QString("%1").arg(awaitMin_);
    QList<int> hours;
    int h = 0;
    while ( h < 24 ) {
      hours << h;
      h += period_/60;
    }
    QStringList str;
    for ( auto hour : qAsConst(hours) )
    {
      hour = hour + awaitHour_;
      if ( hour >= 24 ) {
        hour = hour - 24;
      }
      str << QString::number(hour);
    }
    hours_ = str.join(",");
  }
}

void TimeSheet::parse()
{
  if ( "*" == weekDays_ ) {
    ui->weekTable->selectAll();
  }
  else {
    if ( true == weekDays_.contains("-") )
    {
      for ( int i = weekDays_.midRef(0,1).toInt() - 1, sz = weekDays_.midRef(2,1).toInt(); i < sz; ++i )
      {
        ui->weekTable->item(0, i)->setSelected(true);
      }
    }
    else {
      QStringList weekDays = weekDays_.split(",");
      for (const auto &s : weekDays )
      {
        ui->weekTable->item(0, s.toInt() - 1)->setSelected(true);
      }
    }
  }
}

void TimeSheet::slotChangeWeekDay()
{
  makeWeek();
}

void TimeSheet::slotOk()
{
  makeMinHour();
  makeWeek();
  QStringList sheet = QStringList() << minutes_ << hours_ << days_ << months_ << weekDays_;
  sheet_ = sheet.join(" ");
  QDialog::accept();
}

void TimeSheet::slotCancel()
{
  QDialog::reject();
}

void TimeSheet::slotPeriod()
{
  period_ = ui->periodBox->currentData(Qt::UserRole).toInt();
}

void TimeSheet::slotAwait()
{
  await_ = ui->awaitBox->value();
}

void TimeSheet::slotCron()
{
  cron_ = ui->cronBox->value();
  if ( cron_ < 60 ) {
    awaitMin_ = cron_;
  }
  else {
    awaitHour_ = cron_/60;
    awaitMin_ = cron_%60;
  }
}
