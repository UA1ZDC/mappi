#include "parameters.h"
#include <limits>

#define MIN_VALUE std::numeric_limits<float>::min()


namespace mappi {

namespace receiver {

void Parameters::toProto(const Parameters& other, conf::ReceiverParam* param)
{
  // param->mode()
  param->set_freq(other.freq);
  param->set_gain(other.gain);
  param->set_rate(other.rate);
  param->set_script(other.script.toStdString());

  // if (other.dump.isUsed) {
  conf::DumpParam* dumpParam = param->mutable_dump();
  dumpParam->set_file(other.dump.file.toStdString());
  dumpParam->set_bitrate(other.dump.bitrate);
  dumpParam->set_balancer(other.dump.balancer);
  // }
}

void Parameters::fromProto(const conf::ReceiverParam& other, Parameters* param)
{
  param->freq = other.freq();
  param->gain = other.gain();
  param->rate = other.rate();
  param->script = other.script().c_str();

  if (other.has_dump()) {
    conf::DumpParam dumpParam = other.dump();

    param->dump.isUsed = true;
    param->dump.file = dumpParam.file().c_str();
    param->dump.bitrate = dumpParam.bitrate();
    param->dump.balancer = dumpParam.balancer();

    if (param->dump.balancer == 0)
      param->dump.balancer = 1;
  }
}

Parameters::Parameters()
{
  setDefault();
}

Parameters::~Parameters()
{
}

bool Parameters::isDefault() const
{
  return ((freq == MIN_VALUE) && (gain == MIN_VALUE) && (rate == MIN_VALUE) && script == "sampleRecorder");
}

void Parameters::setDefault()
{
  freq = MIN_VALUE;
  gain = MIN_VALUE;
  rate = MIN_VALUE;
  script = "sampleRecorder";

  dump.isUsed = false;
}

QString Parameters::toString() const
{
  return QString("receiver param(freq:%1, gain:%2, rate:%3, script:%4)")
    .arg(freq)
    .arg(gain)
    .arg(rate)
    .arg(script);
}

}

}
