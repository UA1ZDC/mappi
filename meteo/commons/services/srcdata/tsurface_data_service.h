#ifndef METEO_COMMONS_SERVICES_SRCDATA_TSURFACE_DATA_SERVICE_H
#define METEO_COMMONS_SERVICES_SRCDATA_TSURFACE_DATA_SERVICE_H

#include <qobject.h>
#include <qthread.h>
#include <qrunnable.h>

#include "base_data_service.h"

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/rpc/rpc.h>

class NS_PGBase;

namespace meteo {
namespace surf {

class Control;

class TSurfaceDataService  : public TBaseDataService
{
  using Closure = google::protobuf::Closure;

  const meteo::surf::DataRequest *req_;
  meteo::surf::ValueReply *res_;

public:
  TSurfaceDataService(const DataRequest* req, ValueReply* res, Closure* done);
  ~TSurfaceDataService(  );
    
private:
  bool getValue(const DataRequest* req, ValueReply* res);

  //одно значение у земли - по индексу, дескриптору и  времени
  bool getOneSurfaceData1(const DataRequest* req, ValueReply* res);

  // одно или несколько значений у земли - по индексу(ам ), дескриптору и  времени 
  bool getManySurfaceData3(const DataRequest* req, ValueReply* res);

   //куча значений у земли - по дескриптору и  времени
  bool getManySurfaceData1(const DataRequest* req, ValueReply* res);
  
  bool getManySurfaceData2(const DataRequest* req, ValueReply* res);
  
public slots:
  void run();
};

} // surf
} // meteo

#endif // METEO_COMMONS_SERVICES_SRCDATA_TSURFACE_DATA_SERVICE_H
