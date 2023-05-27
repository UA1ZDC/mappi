#pragma once

#include "fsm.h"
#include "context.h"
#include <qobject.h>


namespace mappi {

namespace antenna {

/**
 * Основная задача сервиса - работа с антенной (относится к сервисам приёма):
 * - формирование траектории спутника;
 * - пересчет координат, если он необходим;
 * - сопрождение спутника по сформированной траектории с учетом времени сеанса;
 * - тестирование антенны.
 *
 * Поддерживает режимы работы:
 * - автоматический - работа по расписанию;
 * - демострационный - в этом режиме работает не происходит физического подключения к антенне;
 * - сервисный - переход сервиса в этот режим, означает прекращение работы по расписанию, доступно ручное управление.
 */
class ServiceHandler :
  public QObject
{
  Q_OBJECT
public :
  explicit ServiceHandler(QObject* parent = nullptr);
  virtual ~ServiceHandler();

  bool init();
  bool start();

  void acceptSession(const conf::SessionBundle* req, conf::AntennaResponse* resp);
  void serviceMode(const ServiceRequest* req, conf::AntennaResponse* resp);
  void healthCheck(const Dummy* req, conf::AntennaResponse* resp);

  void feedhornPower(const conf::Feedhorn* req, conf::AntennaResponse* resp);
  void feedhornOutput(const conf::Feedhorn* req, conf::AntennaResponse* resp);

  void setPosition(const conf::Position* req, conf::AntennaResponse* resp);
  void stop(const Dummy* req, conf::AntennaResponse* resp);
  void monkeyTest(const conf::SpinParam* req, conf::AntennaResponse* resp);
  void travelMode(const Dummy* req, conf::AntennaResponse* resp);

  void subscribe(meteo::rpc::Controller* ctrl, conf::AntennaResponse* resp);

public slots :
  void unsubscribe(meteo::rpc::Controller* ctrl);

protected :
  virtual void timerEvent(QTimerEvent* event);

private :
  void setResponse(conf::AntennaResponse* resp);

private :
  int timerId_;
  Context* ctx_;
  FSM* fsm_;

  QMap<meteo::rpc::Controller*, conf::AntennaResponse*> subscriber_;
};

}

}
