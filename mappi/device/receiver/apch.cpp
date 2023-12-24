#include "apch.h"
#include <QTimerEvent>


namespace mappi {

namespace receiver {

static const int APCH_INTERVAL = 5000; // в миллисекундах
static const uint64_t LIGHT_SPEED = 299792458;

ApchUnit::ApchUnit(const Coords::GeoCoord& site, QObject* parent /*=*/) :
    QObject(parent),
  site_(site),
  timerId_(-1)
{
}

ApchUnit::~ApchUnit()
{
  satellite_ = nullptr;
}

void ApchUnit::turnOn(Satellite* satellite, float freq)
{
  turnOff();

  satellite_ = satellite;
  freq_ = freq;

  timerId_ = startTimer(APCH_INTERVAL/*, Qt::PreciseTimer*/);
}

void ApchUnit::turnOff()
{
  if (timerId_ == -1) return;

  killTimer(timerId_);
  timerId_ = -1;
}

void ApchUnit::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != timerId_) return;
  if (satellite_ == nullptr) return;

  Coords::TopoCoord topo;
  if(satellite_->getPosition(QDateTime::currentDateTimeUtc(), site_, &topo)){
    emit update(freq_ * (1 - topo.rangeRate * 1000 / LIGHT_SPEED));
  }else{
    emit update(-1);
  }

}

}

}
