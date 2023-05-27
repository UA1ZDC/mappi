#include "funcs.h"

#include <qdir.h>

namespace meteo {

const int kMaxHeightKm = 60;

bool calcHumidity(const GeoPoint& pos, const QDateTime& dt, QVector<float>& pressure, QVector<float>* humres)
{
  if ( !pos.isValid() || !dt.isValid() || pressure.isEmpty() ) {
    return false;
  }

  QVector<float> h(kMaxHeightKm);

  float lat = static_cast<float>(pos.latDeg());
  float lon = static_cast<float>(pos.lonDeg());
  if ( lon < 0.0 ) {
    lon += 360.0;
  }
  int mmdd = dt.date().month()*100 + dt.date().day();

  QString oldDir = QDir::currentPath();
  QDir::setCurrent(humidityDataPath());
  QString oldnumlocale = ::getenv("LC_NUMERIC");
  ::setlocale(LC_NUMERIC, "C");
  vlazh_(&lat, &lon, &(*pressure.begin()), &mmdd, &(*h.begin()), &(*humres->begin()));
  ::setlocale(LC_NUMERIC, oldnumlocale.toStdString().c_str());
  QDir::setCurrent(oldDir);

  return true;
}

} // meteo
