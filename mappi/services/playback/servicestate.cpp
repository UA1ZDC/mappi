#include "servicestate.h"


namespace mappi {

namespace playback {

ServiceState::ServiceState(Context* ctx, StateMachine* fsm) :
  ReceptionState(ctx, fsm)
{
}

ServiceState::~ServiceState()
{
}

State::id_t ServiceState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::SERVICE_MODE_ON :
      notifyAll(true);  // client stub
      break ;

    case Primitive::SERVICE_MODE_OFF :
      notifyAll(false); // client stub
      input->append(Primitive::SERVICE_MODE_OFF);
      return State::FIND;

    default :
      break ;
  }

  return id();
}

void ServiceState::notifyAll(bool flag)
{
  ServiceRequest req;
  req.set_flag(flag);

  notifyReceiver(req);
  notifyAntenna(req);

  // info_log << QObject::tr("Сервисы приёма оповещены, service mode");
}

void ServiceState::notifyReceiver(const ServiceRequest& req)
{
  QScopedPointer<rpc::Channel> ch(channel(settings::proto::kReceiver));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kReceiver));
    return ;
  }

  QScopedPointer<conf::ReceiverResponse> resp(ch->remoteCall(&conf::ReceiverService::ServiceMode, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return ;
  }

  debug_log << "receiver state:" << STATE_NAME(resp->state());

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();

    return ;
  }
}

void ServiceState::notifyAntenna(const ServiceRequest& req)
{
  QScopedPointer<rpc::Channel> ch(channel(settings::proto::kAntenna));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kAntenna));
    return ;
  }

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::ServiceMode, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return ;
  }

  debug_log << "antenna state:" << STATE_NAME(resp->state());

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();
  }
}

}

}
