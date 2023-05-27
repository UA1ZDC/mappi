#ifndef METEO_COMMONS_SERVICES_SPRINF_CLIENTHANDLER_H
#define METEO_COMMONS_SERVICES_SPRINF_CLIENTHANDLER_H

#include <memory>
#include <functional>

#include <google/protobuf/repeated_field.h>

#include <qobject.h>
#include <qmap.h>
#include <qfuturewatcher.h>
#include <QtConcurrent/qtconcurrentrun.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/rpc/channel.h>

namespace meteo {
  class Dbi;
  class DbiEntry;
  class DbiQuery;

namespace sprinf {

class TSprinfService;
class HandlerConnection;
template<typename Request, typename Response> class DeferredCallback;

class ClientHandler : public QObject
{
  Q_OBJECT
  friend class HandlerConnection;

public:
  ClientHandler(TSprinfService* service);
  ~ClientHandler();

  HandlerConnection* connection(rpc::Channel* channel) const;
  void removeConnection(rpc::Channel* channel);
  bool contains(rpc::Channel *channel) const;

  template<typename Request, typename Response>
  bool runMethod( rpc::Channel*, void (HandlerConnection::*method)(Request, Response), Request, Response, ::google::protobuf::Closure*);

  void setUnlockEditDb(); // setUnlockEditDb - После операции редактирования базы данных

private:
  TSprinfService* service_;
  QMap<rpc::Channel*, HandlerConnection*> *connections_;

public slots:
  void slotClientConnected(meteo::rpc::Channel* client);
};

class HandlerConnection : public QObject
{
  Q_OBJECT
public:
  //service thread function
  void GetStationTypes(const TypesRequest* request, StationTypes* response);
  void GetGmiTypes(const TypesRequest* request, GmiTypes* response);
  void GetLevelTypes(const TypesRequest* request, LevelTypes* response);

  // получаем список регионов из базы (регионы по которым составляются пронрозы)
  void GetRegions(const MultiStatementRequest* request, Regions* response);
  void GetMilitaryDistrict(const StatementRequest* request, Regions* response);

  // работа с регионами
  void GetRegionGroups(const RegionGroupsRequest* request, RegionGroupsReply* response);
  void SaveRegionGroups(const RegionGroupsRequest* request, RegionGroupsReply* response);
  void DeleteRegionGroups(const RegionGroupsRequest* request, RegionGroupsReply* response);

  void GetStations(const MultiStatementRequest* request, Stations* response);
  void GetStationsByPosition(const CircleRegionRequest* request, StationsWithDistance* res);

  void GetMeteoCenters(const MeteoCenterRequest* request, MeteoCenters* response);
  void GetMeteoParametersByBufr(const MeteoParameterRequest* request, MeteoParameters* response);
  void GetBufrParametersTables(const BufrParametersTableRequest* request, BufrParametersTables* response);
  void GetCountry(const ::meteo::sprinf::CountryRequest* req, ::meteo::sprinf::Countries* resp);

  void UpdateStation(const Station* request, ReportStationsAdded* response);
  void DeleteStation (const Station* request, ReportStationsAdded* response);

  void GetTlgStatistic (const ::meteo::sprinf::TlgStatisticRequest* request, ::meteo::sprinf::TlgStatisticResponse* response);
  void GetCities( const ::meteo::sprinf::Dummy* request, ::meteo::map::proto::Cities* response );
  void GetCityDisplayConfig( const ::meteo::sprinf::Dummy* request, ::meteo::map::proto::CityDisplayConfig* response );

public:
  HandlerConnection(ClientHandler* handler, rpc::Channel* channel);
  ~HandlerConnection();

  template<typename Request, typename Response>
  void pushCallback( void (HandlerConnection::*method)(Request, Response), Request, Response , ::google::protobuf::Closure*);
  void removeCallback(::google::protobuf::Closure* c) { deferredcalls_.removeAll(c); }

  rpc::Channel* channel() const { return channel_; }

private:
  ClientHandler* handler_;
  rpc::Channel* channel_;
  QList<google::protobuf::Closure*> deferredcalls_;
  bool used_;

  bool stationFromNoSql(const DbiEntry& doc, Station* result);
  bool countryFromNoSql(const DbiEntry& doc, Country* result);
  Station stationFromNoSql(const DbiEntry& doc);
  StationFull fullStationFromNoSql(const DbiEntry& doc);
  MeteoCenter centerFromNoSql(const DbiEntry& doc);
  MeteoParameter parameterFromNoSql(const DbiEntry& doc);
  BufrParametersTable bufrParameterTableFromNoSql(const DbiEntry& doc);

  void GetParameterByParameter(QString parameter, const MeteoParameterRequest* request, MeteoParameters* response);

  template < class T, template <class C> class A > std::unique_ptr<DbiQuery> prepareSimpleQuery( Dbi* db,
                                                const QString& queryname,
                                                const A<T>& param,
                                                const QString& parameter = QString() );

private slots:
  void slotFutureFinished();
  void slotClientDisconnect();
};

template<typename Request, typename Response>
bool ClientHandler::runMethod(rpc::Channel* ch, void (HandlerConnection::*method)(Request, Response)
                              , Request request, Response response, ::google::protobuf::Closure* closure)
{
  HandlerConnection *conn = connection(ch);
  if(nullptr == conn) {
    error_log << QObject::tr("Не найден обработчик ответа");
    return false;
  }
  conn->pushCallback(method, request, response, closure);
  return true;
}

template<typename Request, typename Response>
class DeferredCallback : public ::google::protobuf::Closure
{
public:
  typedef void (HandlerConnection::*Method)(Request request, Response response);
  DeferredCallback(HandlerConnection* conn, Method method, Request request, Response response, ::google::protobuf::Closure* done)
    : conn_(conn), method_(method), request_(request), response_(response), done_(done), watcher_(new QFutureWatcher<void>(conn_)) {}
  ~DeferredCallback()
  {
    conn_->removeCallback(this);
    watcher_->waitForFinished();
    if(nullptr != done_) {
      done_->Run();
      done_ = nullptr;
    }
    delete watcher_; watcher_ = nullptr;
  }

  void Run()
  {
    if(true == hasConnection()) {
      QObject::connect(watcher_, SIGNAL(finished()), conn_, SLOT(slotFutureFinished()));
      QFuture<void> future = QtConcurrent::run(conn_, method_, request_, response_);
      watcher_->setFuture(future);
    }
  }
  bool hasConnection() const { return (nullptr != conn_ && nullptr != conn_->channel()); }

private:
  HandlerConnection* conn_;
  Method method_;
  Request request_;
  Response response_;
  ::google::protobuf::Closure* done_;
  QFutureWatcher<void>* watcher_;
};

template<typename Request, typename Response>
void HandlerConnection::pushCallback( void (HandlerConnection::*method)(Request, Response),
                                     Request request, Response response, ::google::protobuf::Closure* done)
{
  DeferredCallback<Request, Response>* dcb = new DeferredCallback<Request, Response>(this, method, request, response, done);
  deferredcalls_.append(dcb);
  if(1 == deferredcalls_.size()) {
    dcb->Run();
  }
}

}
}
#endif
