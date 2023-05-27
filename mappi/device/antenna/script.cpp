#include "script.h"
#include "print.h"
#include <QTimerEvent>


namespace mappi {

namespace antenna {

Script::Script(QObject* parent /*=*/) :
    QObject(parent),
  timerId_(-1),
  timeSlot_(0),
  antenna_(nullptr),
  calls_(0),
  max_(0),
  average_(0)
{
}

Script::~Script()
{
  antenna_ = nullptr;
}

bool Script::run(Antenna* antenna)
{
  if (antenna == nullptr)
    return false;

  // если уже запущен, повторно не запускается
  if (timerId_ != -1)
    return true;

  antenna_ = antenna;
  antenna_->snapshot(&azimut, &elevat);
  info_log << azimut.toString() << elevat.toString();

  timeSlot_ = antenna->profile().timeSlot * 1000;       // msec

  calls_ = 0;
  max_ = 0;
  average_ = 0;

  timerId_ = startTimer(timeSlot_, Qt::PreciseTimer);

  return true;
}

void Script::terminate()
{
  // если уже остановлен, повторно не останавливается
  if (timerId_ == -1)
    return ;

  killTimer(timerId_);
  timerId_ = -1;

  antenna_->stop();
  info_log << trace();
}

bool Script::isRunning() const
{
  return (timerId_ != -1);
}

QString Script::trace()
{
  return QString("script trace(calls: %1, time diff(average/max): %2/%3)")
    .arg(calls_)
    .arg(average_/calls_)
    .arg(max_);
}

void Script::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != timerId_)
    return ;

  // сбор статистики
  calls_ += 1;
  int msec = QTime::currentTime().msec() % timeSlot_;
  average_ += msec;
  if (max_ < msec)
    max_ = msec;

  antenna_->snapshot(&azimut, &elevat);
  if (!exec(antenna_)) {
    terminate();
    emit finished();    // emit signal
  }

  // debug_log << azimut.toString() << elevat.toString();
}

QDateTime Script::now() const
{
  return QDateTime::currentDateTimeUtc();
}

QString Script::toString(const QDateTime& timeStamp)
{
  return timeStamp.toString(TIME_FORMAT);
}

QString Script::toString(const QDateTime& timeStamp, float azimut, float elevat) const
{
  return QObject::tr("%1  %2")
    .arg(timeStamp.toString(TIME_FORMAT))
    .arg(toString(azimut, elevat));
}

QString Script::toString(float azimut, float elevat) const
{
  return QObject::tr("%1;%2")
    .arg(azimut, FW_AZIMUT, 'f', 2)
    .arg(elevat, FW_ELEVAT, 'f', 2);
}

QString Script::toString(const QDateTime& timeStamp, const Drive& azimut, const Drive& elevat) const
{
  return QObject::tr("%1  %2")
    .arg(timeStamp.toString(TIME_FORMAT))
    .arg(toString(azimut, elevat));
}

QString Script::toString(const Drive& azimut, const Drive& elevat) const
{
  return QObject::tr("azimut(%1/%2) elevat(%3/%4)")
    .arg(azimut.self, FW_AZIMUT, 'f', 2)
    .arg(azimut.dst, -FW_AZIMUT, 'f', 2)
    .arg(elevat.self, FW_ELEVAT, 'f', 2)
    .arg(elevat.dst, -FW_ELEVAT, 'f', 2);
}

}

}
