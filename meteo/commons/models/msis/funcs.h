#ifndef METEO_COMMONS_MODELS_MSIS_FUNCS_H
#define METEO_COMMONS_MODELS_MSIS_FUNCS_H

#include <qlist.h>
#include <qdatetime.h>

#include <meteo/commons/models/msis/nrlmsise-00.h>

namespace meteo {
namespace msis {

class DataPoint
{
public:
  inline double densityHe() const { return data.d[0]; }
  inline double densityO()  const { return data.d[1]; }
  inline double densityN2() const { return data.d[2]; }
  inline double densityO2() const { return data.d[3]; }
  inline double densityAR() const { return data.d[4]; }
  inline double densityH()  const { return data.d[6]; }
  inline double densityN()  const { return data.d[7]; }
  inline double anomalousOxygen() const { return data.d[8]; }
  inline double exosphericTemperature() const { return data.t[0]; }
  inline double temperature() const { return data.t[1]; }
  inline double totalMass() const { return data.d[5]; }

  float alt;
  float lat;
  float lon;
  nrlmsise_output data;
};

typedef QList<DataPoint> DataPointList;

enum CalcFlag {
  kConvertK2C = 0x01, // преобразовывать кельвины в цельсии
};

QList<DataPoint> heightProfile(float latDeg,
                               float lonDeg,
                 const QList<float>& altKmList,
                    const QDateTime& dt,
                                 int calcFlags = 0,
                               float f107 = 150.0,
                               float f107a = 150.0,
                               float ap = 4.0);

} // msis
} // meteo

#endif // METEO_COMMONS_MODELS_MSIS_FUNCS_H
