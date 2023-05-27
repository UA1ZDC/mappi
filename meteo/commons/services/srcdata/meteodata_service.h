#ifndef METEO_COMMONS_SERVICES_SRCDATA_METEODATA_SERVICE_H
#define METEO_COMMONS_SERVICES_SRCDATA_METEODATA_SERVICE_H

#include <qrunnable.h>
#include <qobject.h>

#include <meteo/commons/proto/surface_service.pb.h>

#include "base_data_service.h"

#include <sql/nosql/nosqlquery.h>

class QString;
class TMeteoData;

namespace meteo {
namespace surf {

class Control;

enum kMeteoDataReqType{
  rRunService,
  rGetMeteoData,
  rGetMeteoDataTree,
  rGetSigwx,
  rGetValuePoCoord,
  rGetDataCount,
  rMGetNo,
  rGetSumValue,
  rGetDts,
  rAvGetSigwx,
  rAvStations,
  rGetDataCover

};

class MeteoDataService  : public TBaseDataService {
  Q_OBJECT
   void setupDbname(){}

public:
  MeteoDataService(const DataRequest*, FullValue*, google::protobuf::Closure* done,kMeteoDataReqType, int serviceType = settings::proto::kSrcData );
  MeteoDataService(const DataRequest*, DataReply*, google::protobuf::Closure* done,kMeteoDataReqType, int serviceType = settings::proto::kSrcData );
  MeteoDataService(const DataRequest*, CountDataReply*, google::protobuf::Closure* done,kMeteoDataReqType, int serviceType = settings::proto::kSrcData );
  MeteoDataService(const SigwxDesc*, meteo::sigwx::SigWx*, google::protobuf::Closure* done, int serviceType = settings::proto::kSrcData );
  MeteoDataService(const DataRequest* r, SigwxDescResponse* dr, google::protobuf::Closure* done, int serviceType = settings::proto::kSrcData);
  MeteoDataService(const DataRequest* r, StationsDataReply* dr, google::protobuf::Closure* done, int serviceType = settings::proto::kSrcData);


   ~MeteoDataService();

  bool runService(const DataRequest* req, DataReply* res);
  bool getMeteoData(const DataRequest* req, DataReply* res);
  //! С восстановлением иерархии, по индексу, дате, типу данных
  bool getMeteoDataTree( const DataRequest* req, DataReply* res );

   bool getDataCount( const DataRequest* , CountDataReply* );
   bool getDtsForPeriod(const DataRequest* req, DataReply* res);
   bool getSigwx( const SigwxDesc* req, meteo::sigwx::SigWx* resp );
   bool getDataCover(const DataRequest* req, DataReply* res);

private:
   bool meteoDataByCoordQuery(const DataRequest* req,DbiQuery* query);
   bool meteoDataLastValueQuery(const DataRequest* req, DbiQuery *query);
   bool meteoDataQuery(const DataRequest* req, DbiQuery* query);
   bool meteoDataMaxMinValueQuery(const DataRequest* req, DbiQuery* query);

  // NosqlQuery meteoDataByCoordQuery(const DataRequest* );
   bool meteoDataNearestTimeQuery( const DataRequest* req, DbiQuery* query );

   bool getAvailableSigwx(const DataRequest* req, SigwxDescResponse* res);
   bool getAvailableStations(const DataRequest* req, StationsDataReply* res);

  //только одно значение у земли - по индексу, времени и дескриптору
  bool getOneSurfaceData(const DataRequest* req, DataReply* res);

  //несколько значений - по индексу, дескриптору и интервалу времени
  bool getManySurfaceData(const DataRequest* req, DataReply* res);


  //! С восстановлением иерархии, по станции, дате, типу уровня, дексприторам
  bool getMeteoDataTreeOnStation(const DataRequest* req, DataReply* res);

  bool readData(const QString& path, TMeteoData* data, QString* err);

  const meteo::surf::DataRequest* data_request_=nullptr;
  DataReply* data_reply_=nullptr;
  FullValue* value_reply_=nullptr;
  const SigwxDesc* sigwx_request_=nullptr;
  meteo::sigwx::SigWx* sigwx_resp_=nullptr;
  SigwxDescResponse* sigwxDescRes_ =nullptr;
  StationsDataReply* statReply_ =nullptr;

  CountDataReply* count_resp_=nullptr;
  kMeteoDataReqType req_type_;

protected:
  QString dbname_;

public slots:
   void run();
};

}
}

#endif
