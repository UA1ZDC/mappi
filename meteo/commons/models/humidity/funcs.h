#ifndef METEO_COMMONS_MODELS_HUMIDITY_FUNCS_H
#define METEO_COMMONS_MODELS_HUMIDITY_FUNCS_H

#include <qvector.h>

#include <cross-commons/app/paths.h>

#include <commons/geobasis/geopoint.h>

class QDateTime;

extern "C"
{
//!
void vlazh_(float* lat_deg, float* lon_deg, float* pressure_arr, int* date, float* alt_arr, float* hum_arr);
}

namespace meteo {

inline const QString humidityDataPath() { return MnCommon::projectPath() + "/" + HUMIDITY_DATA_PATH; }

bool calcHumidity(const GeoPoint& pos, const QDateTime& dt, QVector<float>& pressure, QVector<float>* humres);

} // meteo

#endif // METEO_COMMONS_MODELS_HUMIDITY_FUNCS_H
