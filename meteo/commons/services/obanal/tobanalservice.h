#ifndef METEO_COMMONS_SERVICES_OBANAL_TOBANALSERVICE_H
#define METEO_COMMONS_SERVICES_OBANAL_TOBANALSERVICE_H

#include <memory>

#include <qstring.h>

#include "tobanaldb.h"

#include <sql/psql/psql.h>

#include <commons/geobasis/geodata.h>

#include <meteo/commons/proto/state.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/weather.pb.h>


class TObanal;
class TObanalDb;

namespace grib {
  class TGribData;
} // grib

namespace obanal {
  //! Состояние
  enum StatType {
    kReqUnknown = -1,   //!< Для задания описания
    kParamErr   = 0,    //!< Некорректные параметры
    kObanalOk   = 1,    //!< Объективный анализ выполнен
    kObanalErr  = 2,    //!< Ошибка выполнения объективного анализа
  };
} // obanal

namespace meteo {

namespace prop {
  class MeteoRequest;
} // prop

//! сервис объективного анализа
class TObanalService
{
  using ObanalPtr = std::unique_ptr<TObanal>;

  ObanalPtr obanal_;

public:
  TObanalService();
  ~TObanalService();
  
  bool haveValueResp(const meteo::surf::DataRequest& req, meteo::surf::DataReply* resp);
  bool haveValueRespGrib(meteo::surf::GribDataReply*  resp);
  bool haveValueRespWind(const meteo::surf::DataRequest& req, meteo::surf::DataReply* resp);
  bool haveValueResp(GeoData *all_data,  const  meteo::map::proto::WeatherLayer &info);

private:

  void addCorners(QVector<RegionParam>& corners, const TGrid::LatLon& grid);

  bool checkParam(int64_t descr, float val, int level , int level_type );
};

} // meteo

#endif // METEO_COMMONS_SERVICES_OBANAL_TOBANALSERVICE_H
