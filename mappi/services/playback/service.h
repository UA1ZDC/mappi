#pragma once

#include "fsm.h"
#include <qobject.h>


namespace mappi {

namespace playback {

/**
 * Основная задача сервиса - "воспроизведение" расписания, последовательное прохождение по сеансам.
 * Сеанс который выбран сервисом для приёма является активным и может находится в одной из фаз:
 * - ожидание (может быть пропущен при пересечении сеансов);
 * - подготовка - уведомление сервисов приёма (настройка всей аппаратуры приёма);
 * - приём (споровождение);
 * - завершен (состояние используется расписанием).
 *
 * Является оркестратором для сервисов приёма (оповещает сервисы приёма о ближайщем сеансе):
 * - receiver.service;
 * - antenna.service.
 *
 * Поддерживает режимы работы:
 * - автоматический - работа по расписанию, выдерживаются все временные интервалы, работа в реальном времени;
 * - демострационный - работа так же идет по расписанию, но сеансы принимаются друг за другом,
 *   без учета фазы предварительного ожидания (используется для тестирования).
 * - сервисный - переход в этот режим означает прекращение работы по расписанию.
 *   При этом все сервисы приёма также переходят в сервисный режим.
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

  void serviceMode(const ServiceRequest* req, conf::ReceptionResponse* resp);
  void healthCheck(const Dummy* req, conf::ReceptionResponse* resp);

private :
  void setResponse(conf::ReceptionResponse* resp);

private :
  Context* ctx_;
  FSM* fsm_;            // QObject
};

}

}
