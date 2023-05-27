#include "crontimer.h"
#include <cross-commons/debug/tlog.h>

CronTimer::CronTimer()
{
}

CronTimer::~CronTimer()
{

}

bool CronTimer::isValid()
{
  return is_valid_;
}

bool CronTimer::setInterval(const QString & interval)
{
  try
  {
    //cron_ = cron::make_cron("* 0/5 * * * ?");
    cron_ = cron::make_cron(interval.toStdString());
  }
  catch (cron::bad_cronexpr const & ex)
  {
     error_log << ex.what();
     is_valid_ = false;
     return false;
  }
  is_valid_ = true;
  return true;
}

qint64 CronTimer::msecsToNext(){
  QDateTime now = QDateTime::currentDateTimeUtc();
  QDateTime next = QDateTime::fromTime_t(cron::cron_next(cron_, now.toTime_t()));
  return now.msecsTo(next);
}

void CronTimer::start()
{
  QTimer::singleShot(msecsToNext(), this, SLOT(slotEmit()));
}

void CronTimer::slotEmit(){
  emit timeout();
  QTimer::singleShot(msecsToNext(), this, SLOT(slotEmit()));
}

