#ifndef METEO_COMMONS_UI_MAP_GRAPH_FUNCS_H
#define METEO_COMMONS_UI_MAP_GRAPH_FUNCS_H

class QPoint;
class QPointF;

namespace meteo {
class GeoPoint;
class Projection;
} // meteo

namespace meteo {

bool graphLinearF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord );
bool graphLinearX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord );

bool graphLinearF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord );
bool graphLinearX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord );

} // meteo

#endif // METEO_COMMONS_UI_MAP_GRAPH_FUNCS_H
