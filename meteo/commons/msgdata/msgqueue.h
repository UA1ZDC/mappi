#ifndef METEO_COMMONS_MSGDATA_MSGQUEUE_H
#define METEO_COMMONS_MSGDATA_MSGQUEUE_H

#include <qmap.h>
#include <qhash.h>
#include <qmutex.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include <meteo/commons/proto/msgparser.pb.h>

namespace meteo {

//!
class MsgInfo
{
public:
  explicit MsgInfo(const tlg::MessageNew& msg);

  inline QDateTime convertedDt() const { return dt_; }
  inline QByteArray md5() const { return md5_; }
  inline QStringList distributed() const { return distributed_; }

private:
  QDateTime dt_;
  QByteArray md5_;
  QStringList distributed_;
};


class MsgQueueItem;

//! Этапы жизненого цикла сообщения.
enum MsgStage {
  kNone,              //!<
  kRoute,             //!< Маршрутизация и рассылка.
  kReceive,           //!< Усвоение (запись в БД и в ФС).
  kReceiveBySrcData,  //!< Передача сообщения сервису SrcData для усвоения.
  kAsyncSaveMsg,      //!< Ожидание ответа на асинхронных запрос SaveMsg().
  kSendReceived,      //!< Передача усвоенных телеграмм.
  kCreate,            //!< Создание телеграмм.
  kProcess,           //!< Обработка сообщений (запрос в БД и пр.).
  kMaxMsgStage,
};

//! Класс MsgQueue предназначен для управления очередью сообщений.
//! Основные принципы организации очереди:
//!   1. Приоритетность.
//! Сообщения с более высоким приоритетом располагаются в начале очереди.
//!   2. Последовательность.
//! Жизненый цикл сообщения делится на несколько этапов (проверка, распределение, усвоение и т.д.). Набор этапов для
//! отдельных сообщений может быть разным в зависимости от различных условий (например, формата сообщения). В каждый
//! момент времени сообщение доступно только для одного обработчика.
//! Этапы позволяют разделить сообщения на группы, которые могут обрабатываться параллельно.
//!   3. Автоматическое удаление.
//! После того как все этапы жизненого цикла были пройдены сообщение удаляется из очереди.
//!
//! Все методы потокобезопасные.
class MsgQueue
{
public:
  //! Конструирует пустую очередь.
  MsgQueue();

  //! Возвращает первое сообщение с наивысшим приоритетом из очереди для этапа stage.
  MsgQueueItem head(MsgStage stage) const;
  //! Возвращает сообщение c идентификатором id.
  MsgQueueItem item(int id) const;

  //! Добавляет сообщение в очередь.
  //! Максимальный размер очереди INT_MAX;
  MsgQueueItem append(const tlg::MessageNew& msg, MsgStage stage);
  //! Удаляет из очереди элемент item.
  void remove(const MsgQueueItem& item);
  //! Удаляет из очереди элемент с идентификатором id.
  void remove(int id);

  //! Возвращает количество сообщений для всех этапов.
  int size() const;
  //! Возвращает количество сообщений для этапа stage.
  int size(MsgStage stage) const;

  //! Удаляет все сообщения из очереди.
  void clear();

  //! Возвращает текущий этап для элемента с идентификатором id.
  MsgStage stage(int id) const;
  //! Возращает сообщение из очереди для элемента с идентификатором id.
  tlg::MessageNew msg(int id) const;
  //! Заменяет сообщение в очереди на msg для элемента с идентификатором id.
  //! Если элемента с идентификатором  id нет в очереди вернёт false.
  bool setMsg(int id, const tlg::MessageNew& msg);
  //! Устанавливает следующий этап для сообщения. Если список этапов пуст, сообщение будет удалено.
  //! Возвращает true, если этап был осуществлён переход на следующий этап, иначе вернёт false.
  bool nextStage(int id);
  //! Возвратит true, если в очереди присутствует элемент с идентификаторм id.
  bool contains(int id) const;
  //!
  void appendStage(int id, MsgStage stage, bool unique = false);

private:
  // данные
  int counter_;
  QHash<int, tlg::MessageNew> msg_;
  QHash<int, QList<MsgStage> > lifeCycle_;
  QMap<int, QList<int> > index_[kMaxMsgStage]; // priority:(indexes)

  // служебные
  mutable QMutex mutex_;
};

//!
class MsgQueueItem
{

public:
  //!
  MsgQueueItem() : queueId_(-1), queue_(0) { cached_ = false; }
  //!
  MsgQueueItem(MsgQueue* queue, int id) : queueId_(id), queue_(queue) { cached_ = false; }

  //! Возвращает идентификатор в очереди.
  int id() const { return queueId_; }
  //! Возвращает указатель на очередь.
  MsgQueue* queue() const { return queue_; }
  //! Возвращает текущий этап.
  MsgStage stage() const;
  //! Добавляет этап stage в список этапов обработки сообщения.
  void appendStage(MsgStage stage, bool unique = false);
  //! Устанавливает следующий этап для сообщения. Если список этапов пуст, сообщение будет удалено.
  //! Возвращает true, если этап был переключен, иначе вернёт false.
  bool nextStage();

  //!
  const tlg::MessageNew& msg() const;
  //!
  bool setMsg(const tlg::MessageNew& msg);

  //! Возвращает true, если MsgQueueItem указывает на существующее сообщение в очереди, иначе вернёт false.
  bool isValid() const;

private:
  // данные
  int queueId_;
  MsgQueue* queue_;

  bool cached_;
  mutable tlg::MessageNew cachedMsg_;
};

} // meteo

#endif // METEO_COMMONS_MSGDATA_MSGQUEUE_H
