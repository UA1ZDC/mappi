#pragma once

#include <mappi/proto/reception.pb.h>
#include <qmap.h>


namespace mappi {

namespace receiver {

/* Параметры приёма. */
class Parameters
{
public :
  typedef QMap<QString, Parameters> pool_t;

  struct dump_t {
    bool isUsed;
    QString file;
    float bitrate;
    int balancer;
  };

public :
  static void toProto(const Parameters& other, conf::ReceiverParam* param);
  static void fromProto(const conf::ReceiverParam& other, Parameters* param);

public :
  Parameters();
  ~Parameters();

  bool isDefault() const;
  void setDefault();

  QString toString() const;

public :
  float freq;
  float gain;
  float rate;

  dump_t dump;  // опционально
};

}

}
