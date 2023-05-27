#ifndef TForecastdb_H
#define TForecastdb_H

#include <meteo/commons/proto/field.pb.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/forecast.pb.h>


namespace meteo{

  class Array;
  
  namespace forecast{

class TForecastDb {
public:
  TForecastDb();
  ~TForecastDb();

  bool saveForecast(const meteo::forecast::ForecastPointData* data);
  bool saveManyForecasts(const meteo::forecast::ForecastManyPointsData* data);
  bool savecAccuracyForecast(const meteo::field::OnePointData* data);
  bool  getForecastResult(const ForecastResultRequest* repl, ForecastResultReply* res);
  bool  getForecastFResult(const ForecastResultRequest* repl, ForecastResultReply* res);
  static bool setOpr(const QStringList &ids, bool opr);
  static bool setError(const QStringList &ids);

  bool saveManualForecast(const ForecastManualData* data, ForecastManualReply* response);
  bool getManualForecastResult(const ForecastManualRequest* request, ForecastManualReply* response);

  bool getForecastAV12( const ForecastAv12Request* req, ForecastAv12Responce* res);


private:
  bool getPunktId(const QString & station_number, QString *punkt_id);
  QString generateSaveForecastQUitem(const meteo::forecast::ForecastPointData* d);

  void setOpravd(meteo::Array& reals, meteo::forecast::ForecastResult* forecast);
};



}}
#endif
