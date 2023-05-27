#ifndef METEO_COMMONS_MSGSTREAM_CORE_STREAMPLUGIN_H
#define METEO_COMMONS_MSGSTREAM_CORE_STREAMPLUGIN_H

#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/global/appstatusthread.h>

namespace meteo {

  enum StatusParam {
    kRecvCount    = 1024,
    kRecvCountToday,
    kRecvSize,
    kRecvSizeToday,
    kSendCount,
    kSendCountToday,
    kSendSize,
    kSendSizeToday,
    kConfirmRecv,
    kConfirmRecvToday,
    kConfirmSended,
    kConfirmSendedToday,
    kLinkStatus,
  };

  const int kTimeToLastActivity = 600000;

class StreamModule;

//! Интерфейс плагина для создания модуля приёма (передачи) метеосообщений.
class StreamPlugin : public QObject
{
  Q_OBJECT
public:
  StreamPlugin(QObject* parent = nullptr) : QObject(parent)  {}
  virtual ~StreamPlugin() {}

  //! Инициализирует модуль для взаимодействия с внешними системами.
  //! \arg name название модуля
  virtual StreamModule* create(const QString& name) const = 0;
  //! Возвращает список модулей поддерживамых плагином.
  virtual QStringList names() const = 0;
};

//! Базовый класс для реализации модулей для взаимодействия со внешними системами (приёма и передачи сообщений).
class StreamModule : public QObject
{
  Q_OBJECT
public:
  StreamModule(QObject* parent = nullptr);
  virtual ~StreamModule(){}

  void setStatus(AppStatusThread* status)         { status_ = status; }
  void setIncoming(TSList<tlg::MessageNew>* list) ;
  void setOutgoing(TSList<tlg::MessageNew>* list) ;

  virtual void setOptions(const msgstream::Options& options)  { Q_UNUSED( options ); }
  virtual void appendNewIncoming(const tlg::MessageNew& new_incoming);

public slots:
  virtual bool slotInit() = 0;
  virtual void slotNewIncoming() {}
signals:
  void newIncomingMsg();
  void newOutgoingMsg();

public slots:
  virtual void slotMsgReceived( qint64 );
  virtual void slotSizeReceived( qint64  );
  virtual void slotMsgSended( qint64 );
  virtual void slotSizeSended( qint64  );
  virtual void slotConfirmSended( qint64 );
  virtual void slotConfirmReceived( qint64  );

protected:
  void resetChecker();

  virtual void timerEvent(QTimerEvent* event);

  AppStatusThread* status_;
  TSList<tlg::MessageNew>* incoming_;
  TSList<tlg::MessageNew>* outgoing_;

  qint64 recvCount_  = 0;
  qint64 recvSize_   = 0;
  qint64 recvCountToday_  = 0;
  qint64 recvSizeToday_   = 0;
  qint64 sendCount_  = 0;
  qint64 sendSize_   = 0;
  qint64 sendCountToday_  = 0;
  qint64 sendSizeToday_   = 0;

  qint64 confirmSended_  = 0;
  qint64 confirmSendedToday_  = 0;
  qint64 confirmRecv_   = 0;
  qint64 confirmRecvToday_   = 0;
private:
  QDateTime lc_ ;

};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_CORE_STREAMPLUGIN_H
