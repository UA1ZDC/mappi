#ifndef METEO_COMMONS_SERVICES_OBANAL_TCALCFHI_H
#define METEO_COMMONS_SERVICES_OBANAL_TCALCFHI_H

#include <qdatetime.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/primarydb/ancdb.h>


namespace meteo {
namespace obanal {

double calcKv( const double ff );
double FHItestR24( const double FHI, const double R24 );
double calcFHI( double oldkp, const double ff, const double T, const double Td );
bool calcAndSaveFHI( const QDate& date );
void addFHIMeteoData( const StationInfo& info, const TMeteoData& md, const QDate& dt );

}
}

#endif
