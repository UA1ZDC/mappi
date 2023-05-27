#pragma once

#include "fsm.h"
#include "context.h"
#include <qobject.h>


namespace mappi {

namespace receiver {

/**
 * Основная задача сервиса - работа с приёмником (относится к сервисам приёма):
 * - настройка приёмника;
 * - включение\выключение приёмника;
 * - создание файла для записи принимаемого raw потока (+ StreamHeader);
 * - создание ссылки на файл для записи принимаемого raw потока;
 * - запись в БД информации об успешном приёме сеанса (TODO вынести в отдельный метод RPC, который работает с БД).
 *
 * Поддерживает режимы работы:
 * - автоматический - работа по расписанию;
 * - демострационный - в этом режиме работает проигрыватель dump файлов;
 * - сервисный - переход сервиса в этот режим означает прекращение работы по расписанию.
 *   В случае активного приёма, приём прекращается.
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

  void acceptSession(const conf::SessionBundle* req, conf::ReceiverResponse* resp);
  void serviceMode(const ServiceRequest* req, conf::ReceiverResponse* resp);
  void healthCheck(const Dummy* req, conf::ReceiverResponse* resp);

private :
  void setResponse(conf::ReceiverResponse* resp);

private :
  Context* ctx_;
  FSM* fsm_;    // QObject
};

}

}
