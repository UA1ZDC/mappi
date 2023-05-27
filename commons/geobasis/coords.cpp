#define _USE_MATH_DEFINES 
#include <stdio.h>
#include <math.h>
#include <new>

#include <cross-commons/funcs/mn_errdefs.h>
#include <commons/mathtools/mnmath.h>
#include <commons/coord_systems/projection_functions.h>
#include <commons/coord_systems/jullian.h>

#include "coords.h"

using namespace std;
using namespace MnMath;

Position::Position() {
	x = 0;
	y = 0;
	latitude = 0.0;
	longitude = 0.0;
}

namespace Coords {

void GeoCoord::reset() {
	lat = 0.;
	lon = 0.;
	alt = 0.;
}

void GeoCoord::setGeoPoint(float alat, float alon, float aalt) {
	lat = alat;
	lon = alon;
	alt = aalt;
}

void GeoCoord::setGeoPoint(const meteo::GeoPoint &gpoint) {
	lat = gpoint.lat();
	lon = gpoint.lon();
	alt = gpoint.alt();
}

meteo::GeoPoint GeoCoord::getGeoPoint() {
	return meteo::GeoPoint(lat, lon, alt);
}

void UsrCoord::reset() {
	x = 0.;
	y = 0.;
	z = 0.;
}

void UsrCoord::setUsrPoint(float ax, float ay, float az) {
	x = ax;
	y = ay;
	z = az;
}

EciCoord::EciCoord() :
		x(0), y(0), z(0), r(0) {
}

EciCoord::EciCoord(float xx, float yy, float zz, float rr /*= 0*/) :
		x(xx), y(yy), z(zz) {
	if (rr == 0) {
		r = sqrt(x * x + y * y + z * z);
	} else {
		r = rr;
	}
}

void EciCoord::mul(float f) {
	x *= f;
	y *= f;
	z *= f;
	r *= fabs(f);
}

EciPoint::EciPoint() {
}

EciPoint::EciPoint(const EciCoord &p, const EciCoord &v, const JullianDate &d) :
		pos(p), vel(v), date(d) {
}

EciPoint::EciPoint(const GeoCoord &geo, const JullianDate &d) {
	date = d;
	float lgmst = date.toLGMST(geo.lon);
	pos = geoToEci(geo, lgmst/*d.toLGMST( geo.lon )*/); //км

	float earthSpeed = M_2PI * (EARTH_SPEED / SEC_PER_DAY);

	vel.x = -earthSpeed * pos.x; //км/с
	vel.y = earthSpeed * pos.y;
	vel.z = 0.0;
	vel.r = sqrt(sqr(vel.x) + sqr(vel.y));
}

Screen::Screen() {
	x = 0;
	y = 0;
}

GeoCoord EciPoint::posToGeo() {
	return eciToGeo(pos, date.toGMST());
}

DayDuration::DayDuration(int t) {
	switch (t) {
	case DAY_LENGTH:
	case NIGHT_LENGTH:
		type = t;
		break;
	default:
		type = DAY_LENGTH;
		break;
	}
}

DayDuration::DayDuration() {
	type = DAY_LENGTH;
}

meteo::GeoPoint DayDuration::solarPosition(int day, float hour) {
	meteo::GeoPoint pos;
	float lat = sin(MnMath::deg2rad(23.5))
			* sin(2. * M_PI * float(day - 83) / 365.25);
	lat = lat / ::sqrt(1.0 - lat * lat);
	lat = atan(lat);
	float lon = MnMath::deg2rad(180.0 - hour * 15.0);
	pos.setLat(lat);
	pos.setLon(lon);
	return pos;
}

bool DayDuration::calcLength(const meteo::GeoPoint &geopos, const QDate &dt) {
	if (false == dt.isValid()) {
		return false;
	}
	date = dt;
	int day = date.dayOfYear() - 1;

	const float DEG_TO_HRS = (24.0 / (M_PI * 2)); // should this be sidereal day?
//   float sunlat, sunlon,hod;

	// find the position of the sun
	float hour = 0.;
	meteo::GeoPoint solarPos = solarPosition(day, hour);

	float x = -1 * tan(solarPos.lat()) * tan(geopos.lat());
	if (-1.0 >= x || 1.0 <= x) {
		length = -1;
//    rise = set = -999;
		return false;
	}
	x = acos(x);
	float rise = DEG_TO_HRS * (solarPos.lon() - geopos.lon() - x);
	float set = DEG_TO_HRS * (solarPos.lon() - geopos.lon() + x);

	//adjust for height above sea level
	float dht = 0;
	float radius = 6371000.;
	dht = 24.0 * acos(radius / (radius + geopos.alt())) / (2. * M_PI);
	rise -= dht;
	set += dht;

	while (rise < 0)
		rise += 24;
	while (rise >= 24)
		rise -= 24;
	while (set < 0)
		set += 24;
	while (set >= 24)
		set -= 24;

	int hbeg = int(rise);
	int mbeg = int((rise - hbeg) * 60.);
	int sbeg = int(((rise - hbeg) * 60. - mbeg) * 60.);
	start = QTime(hbeg, mbeg, sbeg);
	length = int(3600. * (set - rise));
//  length = set - rise;
//  start = QTime( int(rise), rise - int(rise) );

	return true;
}

//геодезические координаты в геоцентрические
EciCoord geoToEci(const GeoCoord &geo, float lgmst) {
	EciCoord eci;
	float c = 1.0
			/ sqrt(1.0 + FLATTENING * (FLATTENING - 2.0) * sqr(sin(geo.lat)));
	float s = sqr(1.0 - FLATTENING) * c;
	float rad = (EQUATOR_RAD * c + geo.alt) * cos(geo.lat);

	eci.x = rad * cos(lgmst);
	eci.y = rad * sin(lgmst);
	eci.z = (EQUATOR_RAD * s + geo.alt) * sin(geo.lat);
	eci.r = sqrt(sqr(eci.x) + sqr(eci.y) + sqr(eci.z));
	return eci;
}

//геодезические координаты в геоцентрические (упрощенная формула эксцентриситет = 1)
EciCoord geoToEciEasy(const GeoCoord &geo) {
	EciCoord eci;

	float c = 1.0
			/ sqrt(1.0 + FLATTENING * (FLATTENING - 2.0) * sqr(sin(geo.lat)));
	float s = sqr(1.0 - FLATTENING) * c;
	float rad = (EQUATOR_RAD * c) * cos(geo.lat);

//  float rad =  MEAN_RAD * cos( geo.lat );
	eci.x = rad * cos(geo.lon);
	eci.y = rad * sin(geo.lon);
	eci.z = EQUATOR_RAD * s * sin(geo.lat);
	eci.r = sqrt(sqr(eci.x - geo.alt) + sqr(eci.y) + sqr(eci.z));
	return eci;
}

//геоцентрические в геодезические(упрощенная формула эксцентриситет = 1)
GeoCoord eciToGeoEasy(const EciCoord &eci) {
	GeoCoord geo;

	geo.lon = fmod(atan2(eci.y, eci.x), M_2PI);
	if (geo.lon < 0.0) {
		geo.lon += M_2PI;
	}

	float r = sqrt(eci.x * eci.x + eci.y * eci.y);
	geo.lat = atan2(eci.z, r);
	const float delta = 1.0e-07;
	float phi = 0.0;
	float e2 = FLATTENING * (2 - FLATTENING);
	float c = 0.0;

	do {
		phi = geo.lat;
		c = 1.0 / sqrt(1.0 - e2 * sqr(sin(phi)));
		geo.lat = atan2(eci.z + EQUATOR_RAD * c * e2 * sin(phi), r);
	} while (fabs(geo.lat - phi) > delta);

	geo.alt = r / cos(geo.lat) - EQUATOR_RAD * c; //km

	return geo;
}

//геоцентрические в геодезические
GeoCoord eciToGeo(const EciCoord &eci, float gmst) {
	GeoCoord geo;

	geo.lon = fmod(atan2(eci.y, eci.x) - gmst, M_2PI);
	if (geo.lon < 0.0) {
		geo.lon += M_2PI;
	}

	float r = sqrt(eci.x * eci.x + eci.y * eci.y);
	float e2 = FLATTENING * (2 - FLATTENING);
	geo.lat = atan2(eci.z, r);
	const float delta = 1.0e-07;
	float c = 0.0;
	float phi = 0.0;

	do {
		phi = geo.lat;
		c = 1.0 / sqrt(1.0 - e2 * sqr(sin(phi)));
		geo.lat = atan2(eci.z + EQUATOR_RAD * c * e2 * sin(phi), r);
	} while (fabs(geo.lat - phi) > delta);

	geo.alt = r / cos(geo.lat) - EQUATOR_RAD * c; //km

	return geo;
}

//геоцентрические в топоцентрические
TopoCoord eciToTopo(const GeoCoord &recv, const EciPoint &target) {
	JullianDate date = target.date;
	EciPoint recvPoint(recv, date);

	EciCoord vecRgRate(target.vel.x - recvPoint.vel.x,
			target.vel.y - recvPoint.vel.y, target.vel.z - recvPoint.vel.z);

	float x = target.pos.x - recvPoint.pos.x;
	float y = target.pos.y - recvPoint.pos.y;
	float z = target.pos.z - recvPoint.pos.z;
	float w = sqrt(sqr(x) + sqr(y) + sqr(z));

	EciCoord vecRange(x, y, z, w);

	// The site's Local Mean Sidereal Time at the time of interest.
	float theta = date.toLGMST(recv.lon);

	float sin_lat = sin(recv.lat);
	float cos_lat = cos(recv.lat);
	float sin_theta = sin(theta);
	float cos_theta = cos(theta);

	float top_s = sin_lat * cos_theta * vecRange.x
			+ sin_lat * sin_theta * vecRange.y - cos_lat * vecRange.z;
	float top_e = -sin_theta * vecRange.x + cos_theta * vecRange.y;
	float top_z = cos_lat * cos_theta * vecRange.x
			+ cos_lat * sin_theta * vecRange.y + sin_lat * vecRange.z;
	float az = atan(-top_e / top_s);

	if (top_s > 0.0)
		az += M_PI;

	if (az < 0.0)
		az += M_2PI;

	if (az == M_2PI)
		az = 0.0;

	float el = asin(top_z / vecRange.r);
	float rate = (vecRange.x * vecRgRate.x + vecRange.y * vecRgRate.y
			+ vecRange.z * vecRgRate.z) / vecRange.r;

	TopoCoord topo(az,  // azimuth,   radians
			el,  // elevation, radians
			vecRange.r,  // range, km
			rate);  // rate,  km / sec

#ifdef WANT_ATMOSPHERIC_CORRECTION
	// Elevation correction for atmospheric refraction.
	// Reference:  Astronomical Algorithms by Jean Meeus, pp. 101-104
	// Note:  Correction is meaningless when apparent elevation is below horizon
	topo.m_El += deg2rad((1.02 /
					tan(deg2rad(rad2deg(el) + 10.3 /
									(rad2deg(el) + 5.11)))) / 60.0);
	if (topo.m_El < 0.0)
	topo.m_El = el;// Reset to true elevation

	if (topo.m_El > (M_PI / 2))
	topo.m_El = (M_PI / 2);
#endif

	return topo;
}
TopoPoint eciToTopoPoint(const GeoCoord& recv, const EciPoint& target){
    JullianDate date = target.date;
    EciPoint recvPoint(recv, date);

    EciCoord vecRgRate(target.vel.x - recvPoint.vel.x, target.vel.y - recvPoint.vel.y, target.vel.z - recvPoint.vel.z);

    float x = target.pos.x - recvPoint.pos.x;
    float y = target.pos.y - recvPoint.pos.y;
    float z = target.pos.z - recvPoint.pos.z;
    float w = sqrt(sqr(x) + sqr(y) + sqr(z));

    EciCoord vecRange(x, y, z, w);

    // The site's Local Mean Sidereal Time at the time of interest.
    float theta = date.toLGMST(recv.lon);

    float sin_lat = sin(recv.lat);
    float cos_lat = cos(recv.lat);
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);

    float top_s = sin_lat * cos_theta * vecRange.x + sin_lat * sin_theta * vecRange.y - cos_lat * vecRange.z;
    float top_e = -sin_theta * vecRange.x + cos_theta * vecRange.y;
    float top_z = cos_lat * cos_theta * vecRange.x + cos_lat * sin_theta * vecRange.y + sin_lat * vecRange.z;
    float az = atan(-top_e / top_s);

    if (top_s > 0.0) az += M_PI;
    if (az < 0.0) az += M_2PI;
    if (az == M_2PI) az = 0.0;

    float el = asin(top_z / vecRange.r);
    float rate = (vecRange.x * vecRgRate.x + vecRange.y * vecRgRate.y + vecRange.z * vecRgRate.z) / vecRange.r;

    TopoCoord targetSphericalCoords(az, el, vecRange.r, rate);

    float elevationVelocity, azimuthVelocity;
    {
        float topoVelocityProjectionSouth = sin_lat * cos_theta * vecRgRate.x
                                            + sin_lat * sin_theta * vecRgRate.y - cos_lat * vecRgRate.z;
        float topoVelocityProjectionEast = -sin_theta * vecRgRate.x
                                           + cos_theta * vecRgRate.y;
        float topoVelocityProjectionZenith = cos_lat * cos_theta * vecRgRate.x
                                             + cos_lat * sin_theta * vecRgRate.y + sin_lat * vecRgRate.z;

        struct {
            float south;
            float east;
            float zenith;
        } vectorTopoRadius { top_s, top_e, top_z };

        struct {
            float south;
            float east;
            float zenith;
        } vectorTopoVelocity { topoVelocityProjectionSouth, topoVelocityProjectionEast, topoVelocityProjectionZenith };

        float topoRadiusProjectionHorizon = sqrt(sqr(vectorTopoRadius.south) + sqr(vectorTopoRadius.east)); //km's
        //float topoVelocityProjectionHorizon = sqrt( sqr( vectorTopoVelocity.south ) + sqr( vectorTopoVelocity.east ) );	//kilomeeter's per second

        // [a,b] = ( ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx );
        // | a x b | = |a|*|b|*sin(alpha);

        //calculate azimuth angle velocity
        //in horizon projection az, bz = 0
        float tangentialVelocityHorizon = (vectorTopoVelocity.south
                                           * vectorTopoRadius.east
                                           - vectorTopoVelocity.east * vectorTopoRadius.south);
        tangentialVelocityHorizon /= topoRadiusProjectionHorizon;//note, that it must be signed

        float normalVelocityHorizon = (vectorTopoVelocity.south
                                       * vectorTopoRadius.south
                                       + vectorTopoVelocity.east * vectorTopoRadius.east);
        normalVelocityHorizon /= topoRadiusProjectionHorizon;//note, that it must be signed

        //in radian per second
        azimuthVelocity = tangentialVelocityHorizon
                          / topoRadiusProjectionHorizon;//may be infinite value at this point
        azimuthVelocity = std::max(-(float) 10000, std::min(azimuthVelocity, (float) 10000.0));//10000.0 is some randome magic big value

        //calculate elevation angle velocity
        //go to specific 2-dimention coord system, determinied by vector r and vertical plane, orthogonal to horizon plane:
        //in this specific coord system vector's coordinate are:
        struct {
            float horizonPlane;
            float zenith;
        } vectorInVerticalPlaneVelocity { normalVelocityHorizon, topoVelocityProjectionZenith };

        struct {
            float horizonPlane;
            float zenith;
        } vectorInVerticalPlaneRadius { topoRadiusProjectionHorizon, vectorTopoRadius.zenith };

        float tangentialVelocityVerticalPlane =
                (vectorInVerticalPlaneVelocity.horizonPlane
                 * vectorInVerticalPlaneRadius.zenith
                 - vectorInVerticalPlaneVelocity.zenith
                   * vectorInVerticalPlaneRadius.horizonPlane);
        tangentialVelocityVerticalPlane /= vecRange.r;

        //TODO: Знак поменян, надо разобраться и вернуть
        elevationVelocity = -tangentialVelocityVerticalPlane / vecRange.r; //radian's per second;
    }

    tSphericalCoord targetAngleVelocities(azimuthVelocity, elevationVelocity);
    TopoPoint returnTopoPoint(targetSphericalCoords, targetAngleVelocities, date);
    return returnTopoPoint;
};
GeoCoord usrToGeo(const UsrCoord &rc) {
	int itemp;
	float n, a, b, e2, eps, rho, latold, altold, dpi2;
	float lat, lon, alt;

	/*  Define convergence limit */
	eps = 1.0E-13;
	dpi2 = 1.570796326794897E0;

	/* Define WGS-84 spheroid parameters:
	 a - semimajor axis
	 b - semiminor (polar) axis
	 e2 - first eccentricity squared */
	a = 6378137.0E0;
	b = 6356752.3142E0;
	e2 = 0.00669437999013E0;

	/* If the position is on the Z axis, the computation
	 must be handled as a special case to keep it from
	 blowing up. */
	rho = sqrt(rc.x * rc.x + rc.y * rc.y);
	if (rho <= eps) {

		/* Come here if we are on the Z axis. */
		lat = dpi2;
		if (rc.z < 0.0)
			lat = -(lat);
		lon = 0.0E0;
		alt = fabs(rc.z) - b;

		return GeoCoord(lat, lon, alt);
	}

	/* Come here in the typical case.  Since latitude and spheroid
	 height depend on one another, the solution must be done
	 iteratively. */
	lon = atan2(rc.y, rc.x);
	lat = atan2(rc.z, rho * (1.0 - e2));
	n = a * a / sqrt(a * a * cos(lat) * cos(lat) + b * b * sin(lat) * sin(lat));
	alt = rho / cos(lat) - n;
	latold = lat + 1.0E0;
	altold = alt + 1.0E0;
	itemp = 20; /* Set up safety exit */
	while ((fabs(lat - latold) >= eps) || (fabs(alt - altold) >= 0.01)) {

		/* Require latitude to converge to about the precision of the
		 machine, and height to the precision of about a centimeter. */
		latold = lat;
		altold = lat;
		n = a * a
				/ sqrt(
						a * a * cos(lat) * cos(lat)
								+ b * b * sin(lat) * sin(lat));
		alt = rho / cos(lat) - n;
		lat = atan2(rc.z, rho * (1.0 - e2 * n / (n + alt)));
		if (--itemp)
			continue;
		/* Didn't converge, something is wrong. */
		// lat = lon = alt = 0.0;
		return GeoCoord(lat, lon, alt);
	}
	return GeoCoord(lat, lon, alt);
}

UsrCoord geoToUsr(const GeoCoord &gc) {
	float n, d, a, e, ome2, nph, clat, slat, clon, slon;
	float x, y, z;
	/* Define WGS-84 spheroid parameters:
	 a - semimajor axis
	 e - first eccentricity
	 ome2 = (1.0E0 - e2) */
	a = 6378137.0E0;
	e = 0.0818191908426E0;
	ome2 = 0.99330562000987;
	clat = cos(gc.lat);
	slat = sin(gc.lat);
	clon = cos(gc.lon);
	slon = sin(gc.lon);
	d = e * slat;

	/* n = Radius of curvature in the prime vertical */
	n = a / sqrt(1.0E0 - d * d);
	nph = n + gc.alt;
	x = nph * clat * clon;
	y = nph * clat * slon;
	z = (ome2 * n + gc.alt) * slat;
	return UsrCoord(x, y, z);
}

/*!
 * \brief Расчёт углового расстояния между двумя точками (географические координаты)
 * \param geo1 Первая точка
 * \param geo2 Вторая точка
 * \return Возвращаемое угловое расстояние, радиан
 */
float geoDistance(const GeoCoord &geo1, const GeoCoord &geo2) {
	float a = spAcos(cosf(geo1.lat) * cosf(fabs(geo1.lon - geo2.lon)));
	float A = M_PI_2f;
	if (false == MnMath::isZero(sinf(a))) {
		A = spAsin(sinf(geo1.lat) / sinf(a));
	}

	float cos_d = cosf(geo2.lat) * cosf(a)
			+ sinf(geo2.lat) * sinf(a) * cosf(M_PI_2f - A);
	return spAcos(cos_d);
}

/*!
 * \brief Определение географических координат второй точки по известным координатам первой точки,
 *  расстоянию между ними и начальному азимуту дуги большого круга
 * \param geo1 Первая точка
 * \param d Угловое расстояние между первой точкой geo1 и искомой точкой
 * \param a Начальный азимут дуги большого круга
 * \return Искомая вторая точка
 */
GeoCoord findSecondCoord(const GeoCoord &geo1, const float d, const float a) {
	GeoCoord geo2;
	float divider = cosf(geo1.lat) * cosf(d)
			- sinf(geo1.lat) * sinf(d) * cosf(a);
	geo2.lat = spAsin(
			sinf(geo1.lat) * cosf(d) + cosf(geo1.lat) * sinf(d) * cosf(a));
	geo2.lon = MnMath::PiToPi(atan2f(sinf(d) * sinf(a), divider) + geo1.lon);
	return geo2;
}

/*!
 * \brief Расчет азимута (-Pi;Pi) из одной точки на поверхности Земли на другую точку на поверхности Земли
 * \brief возвращается азимут в радианах в диапазоне -Pi до Pi
 *
 * \param geo1 Первая точка (положение наблюдателя)
 * \param geo2 Вторая точка (положение объекта)
 * \return Азимут из первой точки на вторую точку, радиан
 */
float geoAzimooth(const GeoCoord &geo1, const GeoCoord &geo2) {
	float a = 0.5f * 6378.160f; //большая полуось эллипсойда Красовского
	float b = 0.5f * 6356.777f; //малая полуось эллипсойда Красовского
	return meteo::calcAzimooth_ellips(a, b, geo1.lat, geo1.lon, geo2.lat,
			geo2.lon);
}

/*!
 * \brief Расчёт координаты точки на поверхности Земли и расстояние до нее от точки над поверхностью Земли если смотреть под углом на Землю
 * \brief  с азимутом 0
 * \brief возвращается расстояние до ближайшей точки
 *
 * \param geo Точка над Землей
 * \param alpha - угол наклона луча rad
 * \param azimut - азимут на точку
 * \param Возвращаемые координаты точки и расстояние, км
 * @return если все в порядке -true
 */
bool sputnik_distance(const GeoCoord &geo, const float alpha,
		const float azimut, GeoCoord *acoord) {

	double b, c1, c2;
	b = geo.alt + MEAN_RAD;
	if (0 > MnMath::teorCosC(MEAN_RAD, b, alpha, &c1, &c2))
		return false;
	float inner_distance = MnMath::teorCosAlpha(MEAN_RAD, b, c1);
	*acoord = findSecondCoord(geo, inner_distance, azimut);
	return true;
}

/*!
 * \brief Вычисляет высоту, азимут и склонение солнца
 * \param geo Координаты места
 * \param datetime Всемирное время на момент расчета
 * \return Структуру, содержащую параметры солнца
 */
SunParams sun_H(const GeoCoord &geo, const QDateTime &datetime) {
	// 1.Вычисление модифицированной  юлианской даты на начало суток
	JullianDate jd(datetime);
	float T0 = jd.getModif(); // мод.юл.дата на начало суток в юлианских столетиях
	float st = jd.toLGMST(geo.lon); // местное звездное время

	float UT = datetime.time().hour()
			+ ((float) datetime.time().minute()) / 60.0
			+ ((float) datetime.time().second()) / 3600.0; //время в часах от полуночи даты

			// 2. Вычисление эклиптических координат Солнца
	float M, L0, L, X, Y, Z;
	M = fmod2p(deg2rad(357.528 + 35999.05 * T0 + 0.04107 * UT)); // средняя аномалия

	L0 = 280.46 + 36000.772 * T0 + 0.04107 * UT;
	L = fmod2p(
			deg2rad(L0 + (1.915 - 0.0048 * T0) * sin(M) + 0.02 * sin(2 * M))); // долгота Солнца

	X = cos(L); // вектор
	Y = sin(L); // в эклиптической
	Z = 0; // системе координат

	// 3.Координаты Cолнца в прямоугольной экваториальной системе координат
	float Eps, X1, Y1, Z1;
	Eps = deg2rad(23.439281); // наклон эклиптики к экватору
	X1 = X; //  вектор
	Y1 = Y * cos(Eps) - Z * sin(Eps); //  в экваториальной
	Z1 = Y * sin(Eps) + Z * cos(Eps); //  системе координат

	// 4. Экваториальные геоцентрические координаты Солнца
	float Ra, Dec;
	Ra = atan2(Y1, X1);
	Dec = atan2(Z1, sqrt(X1 * X1 + Y1 * Y1));

	// 5. Азимутальные координаты Солнца
	float Th, z;
	Th = st - Ra; //часовой угол
	z = MnMath::spAcos(
			sin(geo.lat) * sin(Dec) + cos(geo.lat) * cos(Dec) * cos(Th));

	float h = M_PI_2 - z;
	// float Az = atan2(sin(Th) * cos(Dec) * cos(geo.lat), (sin(h) * sin(geo.lat) - sin(Dec)));
	// Az = atan2(sin(Th) * cos(Dec) * cos(geo.lat), (-sin(h) * sin(geo.lat) + sin(Dec))); такой же рез-т как ниже
	float Az = MnMath::spAcos(
			(sin(Dec) - sin(geo.lat) * cos(z)) / (cos(geo.lat) * sin(z)));

	float b = deg2rad(360.0 / 365.0 * (datetime.date().dayOfYear() - 81));
	float eq = (9.87 * sin(2 * b) - 7.53 * cos(b) - 1.5 * sin(b)) / 60.;//The equation of time (EoT) (in hour)
	float localtime = rad2deg(geo.lon) / 15 + eq + UT;// Local solar time (hrs)

	if (fmod(localtime + 72.0, 24) >= 12.) {
		Az = M_2PI - Az;
	}

	//---------------
	SunParams sunpar(Az, h, Dec);

	return sunpar;
}

/*!
 * \brief Определение дня/ночи
 * \param coord координаты места
 * \param beg  дата/время на момент определения
 * \param thresh порог положения Солнца, радиан
 * \return true - день, false - ночь
 */
bool calcDay(const GeoCoord &coord, const QDateTime &beg, float thresh) {
	SunParams sp = sun_H(coord, beg);
	//If the solar zenith angle is less than or equal to 75°
	//then the day time algorithm is used. Otherwise the night time algorithm is used.
	float zenith = M_PI_2 - sp.h;
	if (zenith <= thresh) {
		return true;
	}
	return false;
}

/*!
 * \brief Найти координаты прямоугольника frame, описывающего четырёхугольник quad
 * \param quad Координаты вписанного четырёхугольника
 * \param frame Координаты описывающего прямоугольника
 * \return ERR_NOERR - нет ошибок, иначе - код ошибки
 */
int findFrameForQuad(const Screen quad[], Screen frame[]) {
	if (!quad || !frame) {
		return ERR_PARAM;
	}
	int res = ERR_NOERR;

	const int count = 4;  //четырёхугольник
	const float M_PI2 = 2 * M_PI;

	Screen _quad[count];
	memcpy(_quad, quad, sizeof(Screen) * count);
	for (int i = 0; i < count; ++i) { //расположить координаты quad[] против часовой стрелки
		if (_quad[2].y < quad[i].y
				|| (_quad[2].y == quad[i].y && _quad[2].x < quad[i].x)) {
			_quad[2] = quad[i];
		}
		if (_quad[3].x > quad[i].x
				|| (_quad[3].x == quad[i].x && _quad[3].y < quad[i].y)) {
			_quad[3] = quad[i];
		}
		if (_quad[0].y > quad[i].y
				|| (_quad[0].y == quad[i].y && _quad[0].x > quad[i].x)) {
			_quad[0] = quad[i];
		}
		if (_quad[1].x < quad[i].x
				|| (_quad[1].x == quad[i].x && _quad[1].y > quad[i].y)) {
			_quad[1] = quad[i];
		}
	}

	float corn[2][count]; //вспомогательная переменная для хранения углов

	int near = 0; //номер вершины, соседней с текущей
	float dx = .0;
	float dy = .0;
	float length = .0;
	for (int i = 0; i < count; ++i) { //найти все углы cторон quad с осью абсцисс
		near = i + 1;
		if (near == count) {
			near = 0;
		}
		dx = float(_quad[near].x) - float(_quad[i].x);
		dy = float(_quad[near].y) - float(_quad[i].y);
		length = hypot(dx, dy);
		if (!length) {
			return ERR_PARAM;
		}
		corn[0][i] = acos(dx / length);
		if (0 > dy) { //3-я и 4-я четверти
			corn[0][i] = M_PI2 - corn[0][i];
		}
	}
	int nMax = 0; //максимальный внутренний угол четырёхугольника
	for (int i = 0; i < count; ++i) { //найти все внтренние углы quad и номер максимального
		near = i - 1;
		if (near < 0) {
			near = count - 1;
		}
		corn[1][i] = corn[0][i] - corn[0][near];
		if (0 > corn[1][i]) {
			corn[1][i] += M_PI2;
		}
		corn[1][i] = M_PI - corn[1][i];
		if (corn[1][i] > corn[1][nMax]) {
			nMax = i;
		}
	}

	int extrimCase = 0;
	if (fabs(sin(corn[0][0] - M_PI_4)) < 1.e-9) {
		extrimCase = 1;
	} else if (fabs(cos(corn[0][0] - M_PI_4)) < 1.e-9) {
		extrimCase = 2;
	}
	if (extrimCase) { //предельный случай, когда стороны описывающего прямоугольника параллельны координатным осям (не посчитать на компьютере)
		for (int i = 0; i < count; ++i) {
			near = i + 1;
			if (near == count) {
				near = 0;
			}
			if ((i + extrimCase) % 2) {
				frame[i].x = _quad[near].x;
				frame[i].y = _quad[i].y;
			} else {
				frame[i].x = _quad[i].x;
				frame[i].y = _quad[near].y;
			}
		}
		return ERR_NOERR;
	}

	float tangent = corn[0][nMax] + (M_PI + corn[1][nMax]) / 2.; //угол между касательной к corn[1][nMax] и осью абсцисс
	float a[count]; //коэффициенты a[i] в уравнениях y[i] = a[i] + b[i]*x[i] (для поиска координат точек пересечения касательных)
	for (int i = nMax; i < nMax + count; ++i) { //найти все коэффциенты a[i] и b[i]
		near = i;
		if (count <= near) {
			near -= count;
		}
		corn[1][near] = tan(tangent + M_PI_2 * (i - nMax)); //b[i]
		a[near] = float(_quad[near].y) - corn[1][near] * float(_quad[near].x);
	}
	for (int i = 0; i < count; ++i) { //найти координаты точек пересечения касательных
		near = i + 1;
		if (near == count) {
			near = 0;
		}
		frame[near].x = int((a[i] - a[near]) / (corn[1][near] - corn[1][i]));
		frame[near].y = int(a[i] + corn[1][i] * frame[near].x);
	}

	return res;
}

int findFrameForQuad(const Screen quad[], Screen *leftTop,
		Screen *rightBottom) {
	if (!quad || !leftTop || !rightBottom) {
		return ERR_PARAM;
	}
	leftTop->x = quad[0].x;
	leftTop->y = quad[0].y;
	rightBottom->x = quad[0].x;
	rightBottom->y = quad[0].y;
	for (int i = 0; i < 4; ++i) {
		leftTop->x = (leftTop->x > quad[i].x) ? quad[i].x : leftTop->x;
		leftTop->y = (leftTop->y > quad[i].y) ? quad[i].y : leftTop->y;
		rightBottom->x =
				(rightBottom->x < quad[i].x) ? quad[i].x : rightBottom->x;
		rightBottom->y =
				(rightBottom->y < quad[i].y) ? quad[i].y : rightBottom->y;
	}
	leftTop->x -= 40;
	leftTop->y -= 40;
	rightBottom->x += 40;
	rightBottom->y += 40;
//  if ( 0 > rightBottom->x ) {
//    rightBottom->x = 0;
//    leftTop->x = 0;
//  }
//  else if ( 0 > leftTop->x ) {
//    leftTop->x = 0;
//  }
//  if ( 0 > rightBottom->y ) {
//    rightBottom->y = 0;
//    leftTop->y = 0;
//  }
//  else if ( 0 > leftTop->y ) {
//    leftTop->y = 0;
//  } 
	return ERR_NOERR;
}

int findFrameForQuad(const Position quad[], Screen *leftTop,
		Screen *rightBottom) {
	if (!quad || !leftTop || !rightBottom) {
		return ERR_PARAM;
	}
	Screen _quad[4];
	for (int i = 0; i < 4; ++i) {
		_quad[i].x = quad[i].x;
		_quad[i].y = quad[i].y;
	}
	return findFrameForQuad(_quad, leftTop, rightBottom);
}

QDataStream& operator<<(QDataStream &in, const GeoCoord &gc) {
	in << gc.lat;
	in << gc.lon;
	in << gc.alt;
	return in;
}

QDataStream& operator>>(QDataStream &out, GeoCoord &gc) {
	out >> gc.lat;
	out >> gc.lon;
	out >> gc.alt;
	return out;
}

QDataStream& operator<<(QDataStream &in, const UsrCoord &rc) {
	in << rc.x;
	in << rc.y;
	in << rc.z;
	return in;
}

QDataStream& operator>>(QDataStream &out, UsrCoord &rc) {
	out >> rc.x;
	out >> rc.y;
	out >> rc.z;
	return out;
}

bool surfacePoint(const GeoCoord &posSatellite, const GeoCoord &posIIO,
		GeoCoord &posSurface, float &iioPath, float &surfacePath) {

	UsrCoord pos0;  //satellite position
	UsrCoord pos1;  //IIO position
	UsrCoord pos;  //surface point position
	bool ok;  //crossing of surface
	pos0 = geoToUsr(posSatellite);
	pos1 = geoToUsr(posIIO);

	if (pos1.z - pos0.z != 0.) {
		ok = surfacePointZ(pos0.x, pos1.x, pos0.y, pos1.y, pos0.z, pos1.z,
				pos.x, pos.y, pos.z);
	} else if (pos1.y - pos0.y != 0.) {
		ok = surfacePointY(pos0.x, pos1.x, pos0.y, pos1.y, pos0.z, pos1.z,
				pos.x, pos.y, pos.z);
	} else if (pos1.x - pos0.x != 0.) {
		ok = surfacePointX(pos0.x, pos1.x, pos0.y, pos1.y, pos0.z, pos1.z,
				pos.x, pos.y, pos.z);
	} else {
		return false;
	}
// printf("x =%e\n",x);
// printf("y =%e\n",y);
// printf("z =%e\n",z);
// printf("R =%e\n",sqrt(x*x+y*y+z*z));

	surfacePath = sqrt(
			(pos.x - pos0.x) * (pos.x - pos0.x)
					+ (pos.y - pos0.y) * (pos.y - pos0.y)
					+ (pos.z - pos0.z) * (pos.z - pos0.z));
	iioPath = sqrt(
			(pos1.x - pos0.x) * (pos1.x - pos0.x)
					+ (pos1.y - pos0.y) * (pos1.y - pos0.y)
					+ (pos1.z - pos0.z) * (pos1.z - pos0.z));

	printf("iioPath = %8.5f\n", iioPath);

	posSurface = usrToGeo(pos);
// if(!squareEquation(a,b,c,root1,root1)) return false;
	return (surfacePath > iioPath) ? ok : false; //if iio at other side of Earth
}

bool surfacePointZ(float x0, float x1, float y0, float y1, float z0, float z1,
		float &x, float &y, float &z) {
	if (z1 - z0 == 0.) {
		x = 0;
		y = 0;
		z = 0;
		return false;
	}
	const float a = 6378137.0E0; //m
	const float ome2 = 0.99330562000987;
	float Ax, Ay, Bx, By, p, q, r; //additional variables
	double root1, root2;
	bool ok;

	Ax = (x1 - x0) / (z1 - z0);
	Bx = z0 * Ax - x0;
	Ay = (y1 - y0) / (z1 - z0);
	By = z0 * Ay - y0;

	p = 1 + (Ax * Ax + Ay * Ay) * ome2;
	q = -2 * (Ax * Bx + Ay * By) * ome2;
	r = (Bx * Bx + By * By - a * a) * ome2;

	ok = squareEquation(p, q, r, root1, root2);

	z = (fabs(root1 - z0) <= fabs(root2 - z0)) ? root1 : root2;
	y = z * Ay - By;
	x = z * Ax - Bx;

	return ok;
}

bool surfacePointY(float x0, float x1, float y0, float y1, float z0, float z1,
		float &x, float &y, float &z) {
	if (y1 - y0 == 0.)
		return false;
	const float a = 6378137.0E0; //m
	const float ome2 = 0.99330562000987;
	float Ax, Az, Bx, Bz, p, q, r; //additional variables
	double root1, root2;
	bool ok;

	Ax = (x1 - x0) / (y1 - y0);
	Bx = y0 * Ax - x0;
	Az = (z1 - z0) / (y1 - y0);
	Bz = y0 * Az - z0;

	p = (1 + Ax * Ax) * ome2 + Az * Az;
	q = -2 * (Ax * Bx * ome2 + Az * Bz);
	r = (Bx * Bx - a * a) * ome2 + Bz * Bz;

	ok = squareEquation(p, q, r, root1, root2);

	y = (fabs(root1 - y0) <= fabs(root2 - y0)) ? root1 : root2;
	z = y * Az - Bz;
	x = y * Ax - Bx;
	return ok;
}

bool surfacePointX(float x0, float x1, float y0, float y1, float z0, float z1,
		float &x, float &y, float &z) {
	if (x1 - x0 == 0.)
		return false;
	const float a = 6378137.0E0; //m
	const float ome2 = 0.99330562000987;
	float Ay, Az, By, Bz, p, q, r; //additional variables
	double root1, root2;
	bool ok;

	Ay = (y1 - y0) / (x1 - x0);
	By = x0 * Ay - y0;
	Az = (z1 - z0) / (x1 - x0);
	Bz = x0 * Az - z0;

	p = (1 + Ay * Ay) * ome2 + Az * Az;
	q = -2 * (Ay * By * ome2 + Az * Bz);
	r = (By * By - a * a) * ome2 + Bz * Bz;

	ok = squareEquation(p, q, r, root1, root2);

	x = (fabs(root1 - x0) <= fabs(root2 - x0)) ? root1 : root2;
	z = x * Az - Bz;
	y = x * Ay - By;
	return ok;
}

void beamPoint(const GeoCoord &posSatellite, const GeoCoord &posSurface,
		GeoCoord &posBeam, float dl) {
	UsrCoord pos0;  //satellite position
	UsrCoord pos1;  //sutface point position
	UsrCoord pos;  //beam point position
	float dr;  //A,B,dr;

	pos0 = geoToUsr(posSatellite);
	pos1 = geoToUsr(posSurface);
	dr = distance(posSatellite, posSurface);

	pos.x = dl / dr * (pos1.x - pos0.x) + pos0.x;
	pos.y = dl / dr * (pos1.y - pos0.y) + pos0.y;
	pos.z = dl / dr * (pos1.z - pos0.z) + pos0.z;

	posBeam = usrToGeo(pos);
	return;
}

float distance(const GeoCoord &posTransmitter, const GeoCoord &posReceiver) {
	UsrCoord pos0, pos1;
	pos0 = geoToUsr(posTransmitter);
	pos1 = geoToUsr(posReceiver);
	return sqrt(
			(pos1.x - pos0.x) * (pos1.x - pos0.x)
					+ (pos1.y - pos0.y) * (pos1.y - pos0.y)
					+ (pos1.z - pos0.z) * (pos1.z - pos0.z));
}

float vectorsAngle(float projX1, float projY1, float projZ1, float projX2,
		float projY2, float projZ2) //rad
		{
	float cosAngle = projX1 * projX2 + projY1 * projY2
			+ projZ1 * projZ2
					/ (sqrt(projX1 * projX1 + projY1 * projY1 + projZ1 * projZ1)
							* sqrt(
									projX2 * projX2 + projY2 * projY2
											+ projZ2 * projZ2));
	cosAngle = (fabs(cosAngle) > 1.) ? 1. : cosAngle;
	return acos(cosAngle);
}

float sunTerminatorLat(int year, int month, int day, int hour, int min, int sec,
		float lon) {
	float littleLat = 1E-5;
	GeoCoord sunPos;
	sunPoint(year, month, day, hour, min, sec, sunPos);
	return (fabs(sunPos.lat - 0.) >= littleLat) ?
			atan(-cos(lon - sunPos.lon) / tan(sunPos.lat)) :
			atan(-cos(lon - sunPos.lon) / tan(littleLat));
}

void sunPoint(int year, int month, int day, int hour, int min, int sec,
		GeoCoord &pos) {
	float utime = hour + min / 60. + sec / 3600.; // universal time
	float md = 367. * year - int(7. * (year + int((month + 9.) / 12.)) / 4.)
			+ int(275. * month / 9.) + day - 678987.;
	float T = (md - 51544.5) / 36525.;
	float w = angle0to360(357.528 + 35999.05 * T + 0.04107 * utime);
	float l = angle0to360(
			280.46 + 36000.772 * T + 0.04107 * utime
					+ (1.915 - 0.0048 * T) * sin(w * M_PI / 180.)
					+ 0.02 * sin(2. * w * M_PI / 180.));
	float x = cos(l * M_PI / 180.); // geocenter coords system
	float y = sin(l * M_PI / 180.);
	float z = 0.;
	float epsilon2 = 84381.488 - 46.815 * T - 0.00059 * T * T
			+ 0.001813 * T * T * T;
	float epsilon1 = angle0to360(epsilon2 / 3600.); //ecliptica angle with equator, rad
	float x1 = x; //equator coords system
	float y1 = y * cos(epsilon1 * M_PI / 180.)
			- z * sin(epsilon1 * M_PI / 180.);
	float z1 = y * sin(epsilon1 * M_PI / 180.)
			+ z * cos(epsilon1 * M_PI / 180.);
	float S0 = 24110.54841 + 8640184.812 * T + 0.093104 * T * T
			- 0.0000062 * T * T * T; // astrotime Grinvitch midmight
	float omega = 1.002737909350795;
	float dT = utime * 3600. * omega; //dtime from midnight, sec
	float S = angle0to360((S0 + dT) / 240.); //astrotime, rad
	float xg = x1 * cos(S * M_PI / 180.) + y1 * sin(S * M_PI / 180.);
	float yg = -x1 * sin(S * M_PI / 180.) + y1 * cos(S * M_PI / 180.);
	float zg = z1;
	float la0 = atan2(yg, xg);
	float fi0 = atan2(zg, sqrt(xg * xg + yg * yg));
	pos.setGeoPoint(fi0, la0, 0.);
// pos.xyzllh(xg*Rav,yg*Rav,zg*Rav); 
	return;
}

}

/*
 GeoCoord& operator=(GeoCoord& gc1, GeoCoord& gc2)
 {
 gc1.lat=gc2.lat;
 gc1.lon=gc2.lon;
 gc1.alt=gc2.alt;
 return gc1;
 }
 */
