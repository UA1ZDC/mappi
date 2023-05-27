#ifndef PUNKTDB_H
#define PUNKTDB_H

#include <QObject>
#include <meteo/commons/proto/forecast.pb.h>
namespace meteo {
namespace forecast {

class PunktDB
{

public:
  //сил больше не было думать, что означают эти имена функций...
  static bool getForecastPunkts( PunktResponce* responce);
  static bool removeForecastPunkts(const PunktRequest* request);
  static bool addForecastPunkts(const PunktRequest* request);
  static bool changeActiveForecastPunkts(const PunktRequest* request);

  static bool addForecastPunkts(  QString punktName, double fi, double la, double height,
				  QString stationId, int stationType,
				  bool isActive);

  static const PunktValue getPunktByStationIndex(const QString &stationIndex, int stationType);
};

}
}

#endif // PUNKTDB_H
