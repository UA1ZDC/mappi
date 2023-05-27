#ifndef METEO_COMMONS_SERVICES_SRCDATA_GEOPHYS_DATA_SERVICE_H
#define METEO_COMMONS_SERVICES_SRCDATA_GEOPHYS_DATA_SERVICE_H

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/services/srcdata/base_data_service.h>


class NS_PGBase;
class TMeteoData;

namespace meteo {
namespace surf {
    
enum kGeophysReqType {
  rGetAvailableGeophysMap,
  rGetGeophysMapById
};

class Control;

class GeophysDataService : public TBaseDataService {
  Q_OBJECT

public:
  GeophysDataService(const DataRequest* req, google::protobuf::Closure* done);
  GeophysDataService(const SimpleDataRequest* req, google::protobuf::Closure* done);
      
  void setDataReply(DataReply* r)                { data_reply_ = r; }
  void setDataReply(GeophysMapAvailableReply* r) { available_reply_ = r; }
  void setReplyType(kGeophysReqType rt)          { req_type_ = rt; }

public slots:
  void run();
      
private:
  bool getAvailable(const DataRequest* req, GeophysMapAvailableReply* resp);
  bool getById(const SimpleDataRequest* req, DataReply* resp);
 
private:
  const DataRequest* data_request_;
  const SimpleDataRequest* simpledata_request_;
  DataReply* data_reply_;
  GeophysMapAvailableReply* available_reply_;
  kGeophysReqType req_type_;
};

} // surf
} // meteo

#endif
