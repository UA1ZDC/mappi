#ifndef METEO_COMMONS_GLOBAL_APPSTATUSTHREAD_H
#define METEO_COMMONS_GLOBAL_APPSTATUSTHREAD_H

#include <qhash.h>
#include <qmutex.h>
#include <qthread.h>
#include <qsemaphore.h>
#include <qdatetime.h>

#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/proto/state.pb.h>
#include <meteo/commons/rpc/rpc.h>

class QTimer;

namespace meteo {

//! Класс AppStatusThread предоставляет интерфейс для отправки информации о статусе в сервис контроля и диагностики
//! Статус процесса - это набор контролируемых параметров, каждый из которых характеризуется
//! названием, значением, временем измерения и состоянием (нормальное, предупреждение, ошибка).
//! \note Все методы потокобезопасные.
class AppStatusThread : public QThread
{
  Q_OBJECT

public:
  static const quint64 kBytesInKiB;
  static const quint64 kBytesInMiB;
  static const quint64 kBytesInGiB;
  static const quint64 kBytesInTiB;

  static const QString kSpeedByte;
  static const QString kSpeedKiB;
  static const QString kSpeedMiB;
  static const QString kSpeedGiB;

  static const QString kSizeByte;
  static const QString kSizeKiB;
  static const QString kSizeMiB;
  static const QString kSizeGiB;
  static const QString kSizeTiB;

  static const std::string kUnkTitle;

public:
  static QString speedStr(quint64 sizeInBytes);
  static QString sizeStr(quint64 bytes);

public:
  explicit AppStatusThread(QObject* parent = 0);
  virtual ~AppStatusThread();

  //! Устанавливает автоматическую отправку статуса после каждого n-го изменения занчения параметра.
  //! Значение -1 (по умолчанию) выключает автоматическую отправку.
  void setUpdateLimit(int n);
  //! Устанавливает автоматическую отправку статуса каждые msec мсек.
  //! Значение -1 (по умолчанию) выключает автоматическую отправку.
  //! TODO
  //  void setInterval(int msec);
  //! Устанавливает ограничение на частоту отправки статуса. .
  //! Значение -1 (по умолчанию) снимает ограничение.
  void setSendLimit(int msec);

  //! Изменяет название параметра с номером paramId на title.
  void setTitle(int paramId, const QString& title);
  //! Изменяет название параметра с номером paramId на title Изменяет значение параметра с номером paramId на value
  void setTitle(int paramId, const QString& title, qint64 value);
  //! Изменяет значение параметра с номером paramId на value и устанавливает состояние равное state.
  void setParam(int paramId, const std::string& value, app::OperationState state = app::OperationState_NORM);
  //! Изменяет значение параметра с номером paramId на value и устанавливает состояние равное state.
  void setParam(int paramId, const QString& value, app::OperationState state = app::OperationState_NORM);
  //! Изменяет значение параметра с номером paramId на value и устанавливает состояние равное state.
  void setParam(int paramId, qint64 value1, qint64 value2, app::OperationState state = app::OperationState_NORM);
  void setParam(int paramId, qint64 value1, app::OperationState state = app::OperationState_NORM);
  //! Удаляет значение параметра с номером paramId.
  void unsetParam(int paramId);
  //! Очищает информацию о статусе.
  void clear();

  //! Блокирует отправку статуса до тех пор пока не будет вызвана функция end(). Минимизирует количество
  //! передаваемой информации при единовременном обновлении сразу нескольких параметров.
  void begin();
  //! Разблокирует отправку и отправляет статус сервису контроля и диагностики.
  void end();

  //! Отправляет статус сервису контроля и диагностики.
  void send();

  //!Получить значение параметра
  QString getParamValue(int paramId);
  QString getTitle(int paramId);
  bool hasId(int paramId);

protected:
  virtual void run();

  bool sendStatus();

private slots:
  void slotTimeout();

private:
  void messageHandler(app::Dummy* reply );

private:
  // параметры
  int countLimit_ = -1;
  int sendLimit_  = -1;

  // данные
  QHash<int,std::string>  titles_;
  app::OperationStatus status_;

  // служебные
  QMutex m_;
  QSemaphore sem_;
  bool sending_ = false;

  rpc::Channel* channel_ = nullptr;

  QHash<int,int> param2index_;
  QTimer* tm_ = nullptr;
};

}

#endif
