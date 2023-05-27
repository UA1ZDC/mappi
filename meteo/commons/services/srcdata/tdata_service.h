#ifndef METEO_COMMONS_SERVICES_SRCDATA_TDATA_SERVICE_H
#define METEO_COMMONS_SERVICES_SRCDATA_TDATA_SERVICE_H

#include <qobject.h>
#include <qhash.h>

#include <commons/container/ts_list.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/requestcard.h>
#include <meteo/commons/global/appstatusthread.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/services.pb.h>

#include <meteo/commons/services/srcdata/meteodata_service.h>
#include <meteo/commons/services/srcdata/tsurface_data_service.h>
#include <meteo/commons/services/srcdata/tzond_data_service.h>
#include <meteo/commons/services/srcdata/tradar_data_service.h>
#include <meteo/commons/services/srcdata/tgrib_data_service.h>
#include <meteo/commons/services/srcdata/ocean_data_service.h>
#include <meteo/commons/services/srcdata/geophys_data_service.h>

#include <meteo/commons/msgdata/msgqueue.h>

#include "types.h"
#include "dataloader.h"

class NS_PGBase;

namespace meteo {
  class Dbi;
  class DbiQuery;
}

namespace meteo {
namespace surf {

enum StatusParam {
  kRequestsCount,
  kSuccessResponseCount,
  kErorrResponseCount,
};

class TDataServiceObj : public QObject {
  Q_OBJECT

public:
  TDataServiceObj();
  ~TDataServiceObj();

  void appendThread(TBaseDataService *ts, QThread *th);

public slots:
  void finished(TBaseDataService *obj);

private :
  QMap<TBaseDataService*, QThread*> pool;
  QMutex mutex_;
};

class Control;

using DataMultiCard = RequestCard<const DataRequest, DataReply>;
using CurStateCard = RequestCard<const CurStateRequest, DataReply>;

class TDataService : public QObject, public SurfaceService
{
  Q_OBJECT
  using RpcController = google::protobuf::RpcController;
  using Closure = google::protobuf::Closure;

public:
  explicit TDataService(::meteo::AppStatusThread* status, int serviceType = settings::proto::kSrcData, QObject* parent = 0);

  void setMsgQueue(MsgQueue* queue) { msgQueue_  = queue; }
  void setRequestQueue(TSList<msgcenter::DecodeRequest>* queue) { decodeQueue_ = queue; }
  void setLoaderQueue(TSList<LoaderCard>* queue) { loaderQueue_ = queue; }

  void SaveMsg(google::protobuf::RpcController *controller,
                         const tlg::MessageNew *request,
                                  SaveMsgReply *response,
                     google::protobuf::Closure *done);

  void SetDecode(google::protobuf::RpcController* controller,
         const ::meteo::msgcenter::DecodeRequest* request,
                       ::meteo::msgcenter::Dummy* response,
                       google::protobuf::Closure* done);

  //!< Запрос на получение полноту данных (отношение количества станций с пришедшими данными к работающим станциям)
  void GetDataCover(RpcController*, const DataRequest*, DataReply*, Closure* );

  //! установить данным флаг проанализировано (Grib)
  void RemoveGribDataAnalysed(RpcController* c, const SimpleDataRequest* req, SimpleDataReply* res, Closure* d);

  void GetNotAnalysedGribData(RpcController* c, const EmptyDataRequest* req, GribDataReply* res, Closure* d);

  //! Доступные грибы по дате и типу уровня
  void GetAvailableGrib(RpcController* c, const DataRequest* req, GribDataAvailableReply* res, Closure* d);

  void GetGribData(RpcController* c, const GribDataRequest* req, GribDataReply* res, Closure* d);

  //! обработка запросов на получение данных по стации(ям) в виде TMeteoData
  void GetMeteoDataOnStation(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);

  //! обработка запросов на получение данных по стации(ям) в виде TMeteoData за сутки
  void GetMeteoDataByDay(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);

  //! обработка запросов на получение данных по стации(ям) в виде TMeteoData
  void GetMeteoData(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);

  //! обработка запросов на получение данных по стации(ям) в виде TMeteoData
  void GetMeteoDataFromGrib(RpcController* c, const GribDataRequest *req, DataReply* res, Closure* d);

  //! С восстановлением иерархии, по индексу, дате, типу данных
  void GetMeteoDataTree(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);

  //! обработка запросов на получение данных по стации(ям) в виде массивов значений
  void GetValueOnStation(RpcController* c, const DataRequest* req, ValueReply* res, Closure* d);


  void GetAvailableZond(RpcController* c, const DataRequest* req, StationsDataReply* res, Closure* d);

  void GetTZondOnStation(RpcController* c, const DataRequest* req, TZondValueReply* res, Closure* d);

  void GetZondDataOnStation(RpcController* c, const DataRequest* req, ZondValueReply* res, Closure* d);

  void GetZondDates(RpcController* c, const DataRequest* req, DateReply* res, Closure* d);

  void GetMrlDataOnAllStation(RpcController* c, const DataRequest* req, ManyMrlValueReply* res, Closure* d);

  void GetAvailableRadar(RpcController* c, const DataRequest* req, DataDescResponse* res, Closure* d);

  void GetDataCount(RpcController* c, const DataRequest* req, CountDataReply* res, Closure* d);

  void GetAvailableSigwx(RpcController* c, const DataRequest* req, SigwxDescResponse* res, Closure* d);

  void GetSigwx(RpcController* c, const SigwxDesc* req, ::meteo::sigwx::SigWx* res, Closure* d);

  void GetGmiSenderStations(RpcController* c, const DataRequest* req, StationsDataReply* res, Closure* d);

  //! Станции с данными по дате и типу данных
  void GetAvailableStations(RpcController* c, const DataRequest* req, StationsDataReply* res, Closure* d);

  //! данные по глубин в регионе
  void GetOceanDataOnPoint(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);

  //! данные по глубинам за срок
  void GetOceanByDt(RpcController* c, const DataRequest* req, DataReply* res, Closure* d);

  //! Доступные данные по дате/времени и дескриптору
  void GetAvailableOcean(RpcController* c, const DataRequest* req, OceanAvailableReply* res, Closure* d);

  //! Океан по id
  void GetOceanById(RpcController* c, const SimpleDataRequest* req, DataReply* res, Closure* d);

  void GetAvailableSynMobSea(RpcController* c, const DataRequest* req, OceanAvailableReply* res, Closure* d);

  void UpdateMeteoData(RpcController* c, const UpdateDataRequest* req, Value* res, Closure* d);

  //! Доступные геофизические карты по дате/времени и типу
  void GetAvailableGeophysMap(RpcController* c, const DataRequest* req, GeophysMapAvailableReply* res, Closure* d);

  //! Геофизическая карта по id
  void GetGeophysById(RpcController* c, const SimpleDataRequest* req, DataReply* res, Closure* d);

  //!< Обработка запросов на добавление данных
  void AddMeteoData(RpcController *controller, const DataReply *request, SimpleDataReply *response, Closure *done);

  void GetMsgMulti(google::protobuf::RpcController* controller,
             const msgcenter::DistributeMsgRequest* request,
                                   tlg::MessageNew* response,
                         google::protobuf::Closure* done);

  void GetDataMulti(RpcController *controller, const DataRequest *request, DataReply *response, Closure *done);

  void GetCurStateMulti(RpcController *controller, const CurStateRequest *request, DataReply *response, Closure *done);

  void GetDtsForPeriod(RpcController *controller, const DataRequest *request, DataReply *response, Closure *done);

signals:
  void subscribeDistributeMsg(const ::meteo::DistributeMsgCard& card);

private slots:
//  void slotClientSubscribed(rpc::Channel* ch, google::protobuf::Closure* done);
  void slotClientSubscribed(rpc::Controller* call);
//  void slotClientDisconnected(rpc::Channel* ch);
  void slotClientUnsubscribed( rpc::Controller* call );
  void slotRunDataMulti();

  void slotMsgProcessed(int id);
  void slotMsgProcessed(int id, const QString& error, qint64 ptkppId, bool duplicate);

  public slots:
  void slotAnsverSendedError();
  void slotAnsverSendedOK();
  void slotRequestReceive();

  signals:
  void signalRequestReceive();


private:
  void moveToThread(TBaseDataService *obj);
//
//  inline NS_PGBase* db() const
//  { return settings::proto::kSrcData == serviceType_ ? meteo::global::dbMeteo() : meteo::global::dbMeteoLocal(); }

private:
  TRadarDataService rds;
  TDataServiceObj dobj;

  TSList<LoaderCard>* loaderQueue_ = nullptr;
  TSList<msgcenter::DecodeRequest>* decodeQueue_ = nullptr;

  quint32 requestCounter_ = 0;

  int serviceType_;
  QHash<rpc::Channel*, Closure*> subs_;
  QHash<rpc::Channel*, DataMultiCard> dataMultiCards_;
  QHash<rpc::Channel*, CurStateCard> curStateCards_;
  bool sendingDataMulti_;
  bool sendingCurState_;

  MsgQueue* msgQueue_ = nullptr;
  QHash<int, SaveMsgCard>  saveMsgCards_;
  ::meteo::AppStatusThread* status_;


  //QString dbname_;
};

}
}

#endif
