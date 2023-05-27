#include <clientstub.h>
#include <cross-commons/debug/tlog.h>

#define ANTENNA_SERVICE  settings::proto::kAntenna
#define PLAYBACK_SERVICE settings::proto::kPlayback


namespace mappi {

namespace antenna {

ClientStub::ClientStub(QObject* parent /*=*/) :
    QObject(parent),
  channel_(nullptr)
{
}

ClientStub::~ClientStub()
{
}

bool ClientStub::serviceMode(bool isEnabled)
{
  debug_log << QString("Sending serviceMode(%1) to antenna").arg(isEnabled);
  QScopedPointer<rpc::Channel> ch(channel(PLAYBACK_SERVICE));
  if (ch == nullptr)
    return false;

  ServiceRequest req;
  req.set_flag(isEnabled);

  QScopedPointer<conf::ReceptionResponse> resp(ch->remoteCall(&conf::ReceptionService::ServiceMode, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();

    return false;
  }

  debug_log << "bool ClientStub::serviceMode(bool isEnabled) - true";
  return true;
}

bool ClientStub::setPosition(float azimut, float elevat, float azimut_speed, float elevat_speed)
{
  debug_log << QString("Sending position(%1[%2], %3[%4]) to antenna")
                        .arg(azimut).arg(elevat).arg(azimut_speed).arg(elevat_speed);
  QScopedPointer<rpc::Channel> ch(channel(ANTENNA_SERVICE));
  if (ch == nullptr)
    return false;

  conf::Position req;
  req.set_azimut(azimut);
  req.set_elevat(elevat);
  if(azimut_speed != 0) req.set_azimut_speed(azimut_speed);
  if(elevat_speed != 0) req.set_elevat_speed(elevat_speed);

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::SetPosition, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();

    return false;
  }

  return true;
}

bool ClientStub::monkeyTest(float dsa, float dse)
{
  debug_log << QObject::tr("Sending monkeyTest to antenna");
  QScopedPointer<rpc::Channel> ch(channel(ANTENNA_SERVICE));
  if (ch == nullptr)
    return false;

  conf::SpinParam req;
  req.set_dsa(dsa);
  req.set_dse(dse);

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::MonkeyTest, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();

    return false;
  }

  return true;
}

bool ClientStub::stop()
{
  debug_log << QObject::tr("Stopping antenna");
  QScopedPointer<rpc::Channel> ch(channel(ANTENNA_SERVICE));
  if (ch == nullptr)
    return false;

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::Stop, Dummy(), 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();

    return false;
  }

  return true;
}

bool ClientStub::feedhornPower(bool flag)
{
  QScopedPointer<rpc::Channel> ch(channel(ANTENNA_SERVICE));
  if (ch == nullptr)
    return false;

  conf::Feedhorn req;
  req.set_power_on(flag);

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::FeedhornPower, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  return resp->result();
}

bool ClientStub::feedhornOutput(int n, bool flag)
{
  QScopedPointer<rpc::Channel> ch(channel(ANTENNA_SERVICE));
  if (ch == nullptr)
    return false;

  conf::Feedhorn req;
  switch (n) {
    case 1 : req.set_output_1(flag); break ;
    case 2 : req.set_output_2(flag); break ;
    case 3 : req.set_output_3(flag); break ;
    case 4 : req.set_output_4(flag); break ;

    default :
      break ;
  }

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::FeedhornOutput, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  return resp->result();
}

bool ClientStub::healthCheck()
{
  QScopedPointer<rpc::Channel> ch(channel(ANTENNA_SERVICE));
  if (ch == nullptr)
    return false;

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::HealthCheck, Dummy(), 5000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  return resp->result();
}

bool ClientStub::subscribe()
{
  // WARNING проверка нужна, mainwindow вызывает 2 раза подряд
  if (channel_ != nullptr)
    return false;

  channel_ = channel(ANTENNA_SERVICE);
  if (channel_ == nullptr)
    return false;

  if (channel_->subscribe(&conf::AntennaService::Subscribe, Dummy(), this, &ClientStub::pipeline) == false) {
    error_log << QObject::tr("%1 не удалось подписаться")
      .arg(global::serviceTitle(ANTENNA_SERVICE));
    return false;
  }

  QObject::connect(channel_, &rpc::Channel::disconnected, this, &ClientStub::unSubscribe);
  debug_log << QObject::tr("antenna service: subscribe enable");

  return true;
}

void ClientStub::unSubscribe()
{
  if (channel_ == nullptr)
    return ;

  QObject::disconnect(channel_, nullptr, this, nullptr);

  delete channel_;
  channel_ = nullptr;

  debug_log << QObject::tr("antenna service: subscribe disable");
}

void ClientStub::pipeline(conf::AntennaResponse* resp)
{
  emit snapshot(resp);
}

rpc::Channel* ClientStub::channel(settings::proto::ServiceCode code)
{
  rpc::Channel* res = global::serviceChannel(code);
  if (res == nullptr)
    warning_log << QObject::tr("%1 недоступен").arg(global::serviceTitle(code));

  return res;
}

}

}
