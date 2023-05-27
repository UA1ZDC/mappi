#ifndef METEO_COMMONS_SERVICES_FORECAST_METHODBAG_H
#define METEO_COMMONS_SERVICES_FORECAST_METHODBAG_H

#include <qmutex.h>

#include <meteo/commons/rpc/clienthandler.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/proto/field.pb.h>


#include <sql/nosql/nosqlquery.h>

namespace meteo {
namespace forecast {

class StatusSender;
class Service;
class Document;

class TForecastService;

class MethodBag : public QObject
{
  Q_OBJECT
public:
  MethodBag( TForecastService* s );
  ~MethodBag();
  
  struct UniqOprKey {
    QString method;
    int hour;
    int center;
    int level;
    int type_level;
    int descr;
    bool operator==(const UniqOprKey& other) {
      if (method != other.method) return false;
      if (hour   != other.hour)   return false;
      if (center != other.center) return false;
      if (level  != other.level)  return false;
      if (type_level != other.type_level) return false;
      if (descr  != other.descr)  return false;
      return true;
    }
  };
  
    // получаем оправдываемость прогнозов из базы
    void GetAccuracy( QPair< rpc::ClientHandler*, const AccuracyRequest* > param, AccuracyReply* resp );
    // запрашиваем данные оправдываемости по всем методам по одной станции
    void GetManyAccuracy( QPair< rpc::ClientHandler*, const ManyAccuracyRequest* > param, ManyAccuracyReply* response );
    // Считаем оправдавшиеся прогнозы по станции 
    void CalcForecastOprStation( QPair< rpc::ClientHandler*, const meteo::forecast::AccuracyRequest* > param, meteo::field::SimpleDataReply* response );

    // Метод сохранения введенных вручную прогнозов погоды по региону или по станции 
    void SaveManualForecast( QPair< rpc::ClientHandler*, const meteo::forecast::ForecastManualData* > param, meteo::forecast::ForecastManualReply* response );
    // Метод получения прогнозов введенных вручную прогнозов погоды по региону или по станции 
    void GetManualForecastResult( QPair< rpc::ClientHandler*, const meteo::forecast::ForecastManualRequest* > param, meteo::forecast::ForecastManualReply* response );
    
private:
  bool setOprFlags(const QList<long unsigned int>& ids,
		   const QList<long unsigned int>& opr_ids,
		   const QList<long unsigned int>& not_opr_ids);
  
  
private:
  TForecastService* service_ = nullptr;
  
  friend class StatusSender;
};

}
}

#endif
