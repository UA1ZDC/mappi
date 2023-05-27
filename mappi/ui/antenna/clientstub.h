#pragma once

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <mappi/proto/reception.pb.h>
#include <qobject.h>


namespace mappi {

namespace antenna {

using namespace meteo;

class ClientStub :
  public QObject
{
  Q_OBJECT
public :
  explicit ClientStub(QObject* parent = nullptr);
  virtual ~ClientStub();

signals :
  void snapshot(conf::AntennaResponse* resp);

public :
  bool serviceMode(bool isEnabled);
  bool setPosition(float azimut, float elevat, float azimut_speed = 0, float elevat_speed = 0);
  bool monkeyTest(float dsa, float dse);
  bool stop();

  bool feedhornPower(bool flag);
  bool feedhornOutput(int n, bool flag);

  bool healthCheck();
  bool subscribe();
  void unSubscribe();

private slots :
  void pipeline(conf::AntennaResponse* resp);   // rpc subscribe callback

private :
  rpc::Channel* channel(settings::proto::ServiceCode code);

private :
  rpc::Channel* channel_;       // subscribe channel
};

}

}
