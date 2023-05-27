#ifndef METEO_COMMONS_SERVICES_SRCDATA_TGRIB_DATA_SERVICE_H
#define METEO_COMMONS_SERVICES_SRCDATA_TGRIB_DATA_SERVICE_H

#include <meteo/commons/services/srcdata/base_data_service.h>

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/zond/turoven.h>
#include <meteo/commons/grib/iface/tgribiface.h>

class TGribDb;


namespace meteo {
  class GeoData;
}

namespace meteo {
namespace surf {

enum kGribReqType{
  rGetNotAnalysedGribData,
  rRemoveGribDataAnalysed,
  rGetGribData,
  rGetGribMeteoData,
  rGetGribAvailable
};

class Control;

class TGribDataService  : public TBaseDataService {
  Q_OBJECT

  struct Grib2_Param {
    int32_t category;
    int32_t discipline;
    int32_t parameter;
  };

public:
  TGribDataService(google::protobuf::Closure* done);
  ~TGribDataService();

  void setGribDataRequest(const GribDataRequest* r)     { grib_data_request_ = r; }
  void setGribDataReply(GribDataReply* r)               { grib_data_reply_ = r; }
  void setDataReply(DataReply* r)               { data_reply_ = r; }
  void setSimpleDataRequest(const SimpleDataRequest* r) { simple_data_request_ = r; }
  void setSimpleDataReply(SimpleDataReply* r)           { simple_data_reply_ = r; }
  void setReplyType(kGribReqType rt)                    { req_type_ = rt; }
  void setDataRequest(const DataRequest* r)             { data_request_ = r; }
  void setDataReply(GribDataAvailableReply* r)          { data_avail_reply_ = r; }


  GribDataReply* getGribDataReply() { return grib_data_reply_; }


public slots:
  void run();

private :
  /*! \brief  markGribDataAnalysed - отметить гриб-ы проанализированными*/
  bool markGribDataAnalysed(const QStringList &idList);

  /*! \brief markGribDataAnalysed - отметить GRIB проанализированными */
  bool markGribDataAnalysed(const SimpleDataRequest* req, SimpleDataReply* resp);

  /*! \brief markBrokenGrib - помечает GRIB, как содержащий ошибки и устанавливает comment */
  bool markBrokenGrib( const QString& brokenGribId, const QString& comment );

  /*! \brief getNotAnalysedGribData - Запрос GRIB для анализа */
  bool getNotAnalysedGribData(GribDataReply* resp);

  bool getAvailableGrib(const DataRequest* req, GribDataAvailableReply* res);
  bool getGribData(const GribDataRequest* req, GribDataReply* res);

  bool getGribMeteoData(const GribDataRequest *req, DataReply* res);
  bool getGribForMeteo(const GribDataRequest* req, GribDataReply* res);

  void fillMeteoData(const GribDataReply& gres, QVector<TMeteoData>* md);
  void fill(const meteo::GeoData& gdata, bool withGrid, int descr, const QString& unit, QVector<TMeteoData>* md);
  void unionMeteodata(const QVector<TMeteoData>& src, QVector<TMeteoData>* dst);

  /*! \brief getGrib2Param - Запрос продуктов для GRIB1 и GRIB2 */
//  void getGribParam(const int32_t param, QStringList& grib1_param, QList<Grib2_Param>& grib2_param);

  //bool getOneGribData(const GribDataRequest* req, GribDataReply* resp);

  const GribDataRequest* grib_data_request_ = nullptr;
  GribDataReply* grib_data_reply_ = nullptr;
  DataReply* data_reply_ = nullptr;
  const SimpleDataRequest* simple_data_request_ = nullptr;
  SimpleDataReply* simple_data_reply_ = nullptr;
  const DataRequest* data_request_ = nullptr;
  GribDataAvailableReply* data_avail_reply_ = nullptr;
  TGribIface iface_;
  kGribReqType req_type_;
};

} // surf
} // meteo

#endif // METEO_COMMONS_SERVICES_SRCDA_TATGRIB_DATA_SERVICE_H
