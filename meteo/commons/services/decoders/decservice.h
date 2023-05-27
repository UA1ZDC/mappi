#ifndef METEO_COMMONS_SERVICES_DECODERS_DECSERVICE_H
#define METEO_COMMONS_SERVICES_DECODERS_DECSERVICE_H

#include <meteo/commons/global/global.h>
//#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/tlgpool/telegramspool.h>
#include <cross-commons/debug/tlog.h>

#include <qstring.h>
#include <qbytearray.h>
#include <QMutex>
#include <QThread>

#include <google/protobuf/service.h>

class TServiceStatus;

namespace meteo {
  namespace decoders {
    class ProcessMsgThread;
  }
}

namespace meteo {
  namespace app {
    class OperationStatus;
  }
}

namespace meteo {
  //! Адреса сервисов
  struct ServiceOpt {
    meteo::settings::proto::ServiceCode stype; //!< тип сервиса
    ServiceOpt() {}
    ServiceOpt(const ServiceOpt& opt) { 
      stype = opt.stype;
    }
  };
}

class DecodersService : public QObject
{
    Q_OBJECT
public:
  DecodersService(const meteo::ServiceOpt& opt);
  virtual ~DecodersService();
  
  meteo::SafeTelegramPool<meteo::tlg::MessageNew>* pool() { return _pool; }
  
  virtual const QString& sprinfAddress();

  void messageHandler(meteo::msgcenter::Dummy *reply) {
    delete reply;
    reply = nullptr;
  }
					
public slots:
  bool init();
  void sendDecoded(long long id);

  bool checkChannel() const;
  
protected:

  virtual void receivedMsg(meteo::tlg::MessageNew* tlg);

  std::string sbscrType()const { return _sbscrType; }

private:

  bool createChannel();


private slots:
  
  void controlChannels();
  bool createSubscribe();
  void sendDecoded();

  private:
  ::meteo::rpc::Channel* _chSubscr = nullptr; //!< Для подписки

    QTimer* _updateTimer;
    meteo::SafeTelegramPool<meteo::tlg::MessageNew>* _pool;
    QList<long long> _decodedId;
 
    QString _sprinfAddr;

    QDateTime _dt; //!< Время запуска сервиса
    meteo::tlg::MessageNew* _res;
    meteo::ServiceOpt _option; //!< Параметры запуска
    std::string _sbscrType;
    QMutex _mutex;
};

class DecodersServiceThread : public QObject {
  Q_OBJECT
public:
  DecodersServiceThread(const meteo::ServiceOpt& opt);
  ~DecodersServiceThread();

  void start();

  virtual ulong processMsg(const meteo::tlg::MessageNew& tlg) = 0;
  bool setStatusControl(TServiceStatus* statMng);

public slots:
  void processMsg();
  void updateStatus();

protected:
  virtual void fillStatus(meteo::app::OperationStatus* status) const;

signals:
  void sendDecoded(long long id);
  void sendStatus(const meteo::app::OperationStatus&);

private:
  DecodersService* _serv;
  QThread _pthr;
  meteo::SafeTelegramPool<meteo::tlg::MessageNew>* _pool;
  TServiceStatus* _statusMng; //!< Отвечает за своевременную отправку статуса (контролируемых параметров)
};

//! декодеры
template <class Object, class Service = DecodersServiceThread> class TMeteoService: public Object, public Service {
public:
  TMeteoService(const meteo::ServiceOpt& opt);
  TMeteoService();
  virtual ~TMeteoService() {}

  void clear() {
    _idPtkpp = 0;
    _result = -1;
    _tlgIdx = 0;
    _tlgDecoded = false;
  }

  ulong processMsg(const meteo::tlg::MessageNew& req) {
     //var(tlg->DebugString());
    _tlgDecoded = false;

    if (!req.has_msg()) {
      return 0;
    }

    if (req.has_metainfo() && req.metainfo().has_id()) {
      _idPtkpp = req.metainfo().id();
    } else {
      _idPtkpp = 0;
    }
    if (req.has_header() && req.header().has_ii()) {
      _tlgIdx = req.header().ii();
    } else {
      _tlgIdx = 0;
    }
    if (req.has_metainfo() && req.metainfo().has_converted_dt()) {
      _dt = QDateTime::fromString(QString::fromStdString(req.metainfo().converted_dt()), Qt::ISODate);
    } else {
      _dt = QDateTime();
    }
    if ( true == req.has_cor_number() ) {
      _cor_number = req.cor_number();
    }
    else {
      _cor_number = 0;
    }
    _dt.setTimeSpec(Qt::UTC);
    //    debug_log << "recv" << _idPtkpp << "ii_tlg="<< _tlgIdx;
    QMap<QString, QString> t;
    t.insert("id", QString::number(_idPtkpp));
    if (req.format() == ::meteo::tlg::kGMS) {
      t.insert("ttaaii", QString::fromStdString(req.header().t1() + req.header().t2() +
						req.header().a1() + req.header().a2()) +
	       QString::number(req.header().ii()));
    }
    _result = Object::decode(t, QByteArray(req.msg().data(), req.msg().size()));

    if (_result > 0) {
      return _idPtkpp;
    }
    return 0;
  }


  virtual void dataReady() { save(); } //!< функция, вызываемая во всех декодерах при готовности данных
  
  
  uint32_t save(); //!< сохранение обработанных данных
  void fillStatus(meteo::app::OperationStatus* status) const;
  const QString& sprinfAddress() { return Service::sprinfAddress(); }

  virtual QString sourceId() { return QString::number(_idPtkpp); }
  virtual QDateTime headerDt() { return _dt; }
  virtual int32_t corNumber() { return _cor_number; }

private:
  long long _idPtkpp; //!< id телеграмы в БД telegrams (ObjectId, 12 байт, raw)
  int _result; //!< Результат раскодирования
  int _tlgIdx;
  bool _tlgDecoded; //!< true - хоть одна сводка сохранена после раскодирования
  QDateTime _dt;
  int32_t _cor_number;
};


#endif
