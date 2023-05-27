#ifndef METEO_COMMONS_SERVICES_FIZFORECAST_METHODBAG_H
#define METEO_COMMONS_SERVICES_FIZFORECAST_METHODBAG_H

#include <qmutex.h>

#include <meteo/commons/rpc/clienthandler.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/services/forecast/tforecastlist.h>
#include <meteo/commons/services/forecast/tforecast.h>
#include <meteo/commons/services/forecast/tfitem.h>
#include <meteo/commons/services/forecast/tfmodel.h>


#include <sql/nosql/nosqlquery.h>

namespace nosql {
class NosqlQuery;
}

namespace meteo {
namespace forecast {

class StatusSender;
class Service;
class Document;

class TForecastService;

class FizMethodBag : public QObject
{
  Q_OBJECT
public:
  FizMethodBag( TForecastService* s );
  ~FizMethodBag();
  
  void GetForecastMethodsList( const Dummy* request,  ForecastMethodList* response);
  void GetForecastMethodsResultRun( const ForecastResultRequest*,  ForecastFizResult* response);

private:
  int oprSynSit(const QDateTime& dt, const GeoPoint& station_coord);

  TForecastService* service_ = nullptr;
  //TForecastList* methods_list_ = nullptr;


  friend class StatusSender;
};

}
}

#endif
