#include "receiver.h"
#include "subprocessengine.h"
#include "udpengine.h"
#include "dumpengine.h"


namespace mappi {

namespace receiver {

Receiver::Receiver(QObject* parent /*=*/) :
    QObject(parent),
  apch_(nullptr),
  engine_(nullptr)
{
}

Receiver::~Receiver()
{
}

bool Receiver::tuned(const Configuration& conf)
{
  if (conf.apch) {
    apch_ = new ApchUnit(conf.coord, this);
    QObject::connect(apch_, &ApchUnit::update, this, &Receiver::adjustFreq);
  }

  if (conf.demoMode)
    engine_ = new DumpEngine(this);
  else {
    switch (conf.engine) {
      case conf::kSubEngine :
        engine_ = new SubProcessEngine(this);
        break;

      case conf::kUdpEngine :
        engine_ = new UdpEngine(this);
        break;

      default :
        break;
    }
  }

  info_log << engine_->type();

  return (engine_ && engine_->configure(conf));
}

bool Receiver::start(Satellite* satellite, const Parameters& param)
{
  if (apch_)
    apch_->turnOn(satellite, param.freq);

  return engine_->start(satellite, param);
}

bool Receiver::stop()
{
  if (apch_)
    apch_->turnOff();

  return engine_->stop();
}

void Receiver::adjustFreq(float freq) {
  if (freq == -1) {
    warning_log << QObject::tr("Ошибка получения радиальной скорости спутника, АПЧ не работает");
    return ;
  }

  debug_log << "update freq:" << freq;
  engine_->updateFreq(freq);
}

}

}
