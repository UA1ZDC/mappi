#ifndef METEO_COMMONS_SERVICES_SRCDATA_OCEAN_DATA_SERVICE_H
#define METEO_COMMONS_SERVICES_SRCDATA_OCEAN_DATA_SERVICE_H

#include <memory>

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/services/srcdata/base_data_service.h>

class NS_PGBase;
class TMeteoData;

namespace meteo {
namespace surf {

enum kOceanReqType{
  rGetManyOceanDataField,
  rGetAvailableOcean,
  rGetAvailableSynMobSea,
  rGetOceanById
};

class Control;

class OceanDataService : public TBaseDataService {
  Q_OBJECT

public:
  OceanDataService(const DataRequest* req, google::protobuf::Closure* done);
  OceanDataService(const SimpleDataRequest* req, google::protobuf::Closure* done);

  void setValueReply(ValueReply* r)         { value_reply_ = r; }
  void setDataReply(DataReply* r)           { data_reply_ = r; }
  void setDataReply(OceanAvailableReply* r) { available_reply_ = r; }
  void setReplyType(kOceanReqType rt)       { req_type_ = rt; }

public slots:
  void run();

private:
  bool getManyOceanDataField(const DataRequest* req, ValueReply* resp);

  bool getAvailable(const DataRequest* req, OceanAvailableReply* res);
  bool getAvailableSynMobSea(const DataRequest* req, OceanAvailableReply* res);

  bool getById(const SimpleDataRequest* req, DataReply* resp);

private:
  const DataRequest* data_request_;
  const SimpleDataRequest* simpledata_request_;
  ValueReply* value_reply_;
  DataReply* data_reply_;
  OceanAvailableReply* available_reply_;
  kOceanReqType req_type_;
};

} //surf
} // meteo

#endif
