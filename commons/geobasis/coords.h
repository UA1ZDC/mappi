#ifndef COORDS_H
#define COORDS_H

#include <commons/geobasis/geopoint.h>

#include <commons/coord_systems/jullian.h>

struct Position { //FIXME добавить в namespace Coords
	float latitude;
	float longitude;
	int x;
	int y;
	Position();
};

namespace Coords {
//! геоцентрическая инерциальная система координат
struct EciCoord {
	float x; //!< в точку весеннего равноденствия, км
	float y; //!< дополняет x и z до правосторонней ортогональной системы, км
	float z; //!< вдоль оси на север, км
	float r; //!< расстояние, км
	EciCoord();
	EciCoord(float xx, float yy, float zz, float rr = 0);
	void mul(float f);
};

//!геодезическая система координа
struct GeoCoord {
	float lat; //!< широта,  радиан [-PI,PI]
	float lon; //!< долгота,радиан [0,2*PI)
	float alt; //!< высота над уровнем моря, км
	GeoCoord(float latit = 0, float longit = 0, float altit = 0) :
			lat(latit), lon(longit), alt(altit) {
	}
	void reset();
	void setGeoPoint(float alat, float alon, float aalt);
	void setGeoPoint(const meteo::GeoPoint& gpoint);
	meteo::GeoPoint getGeoPoint();
	bool operator==(const GeoCoord& gc) const {
		if (lat == gc.lat && lon == gc.lon && alt == gc.alt) {
			return true;
		} else
			return false;
	}
};

//!геоцентрическая прямоугольная система координат (Universal Space Rectangular System)
struct UsrCoord {
	float x; //!< расстояние по оси OX направленной в точку пересечения Гринвического меридиана с экватором,км
	float y; //!< расстояние по оси OY дополняющей систему координат до правой, км
	float z; //!< расстояние по оси OZ направленной на средний северный полюс Земли, км
	UsrCoord(float ax = 0, float ay = 0, float az = 0) :
			x(ax), y(ay), z(az) {
	}
	void reset();
	void setUsrPoint(float ax, float ay, float az);

//    QDataStream& operator<<(QDataStream& in, const GeoCoord& gc);
//    QDataStream& operator>>(QDataStream& out, GeoCoord& gc);
	UsrCoord& operator=(const UsrCoord& rc) {
		x = rc.x;
		y = rc.y;
		z = rc.z;
		return *this;
	}
	bool operator==(const UsrCoord& rc) const {
		if (x == rc.x && y == rc.y && z == rc.z) {
			return true;
		} else
			return false;
	}
};

//! координаты точки в геоцентрической инерциальной СК и скорость их изменения
struct EciPoint {
	EciCoord pos; //!<положение, км
	EciCoord vel; //!<скорость, км/с
	JullianDate date; //!<момент времени измерения pos и vel

	EciPoint();
	EciPoint(const EciCoord& p, const EciCoord& v, const JullianDate& d);
	EciPoint(const GeoCoord& geo, const JullianDate& d);

	GeoCoord posToGeo();
};

//!  EciPoint + имя
struct EciPointName {
	EciPoint e_point; //!<координаты точки в геоцентрической инерциальной СК и скорость их изменения
	QString name_ka;
};

//! topocentric coordinates
struct TopoCoord {
	float az;         //!< Azimuth, radians
	float el;         //!< Elevation, radians
	float range;      //!< Range, kilometers
	float rangeRate;  //!< Range rate of change, km/sec
	//!< Negative value means "towards observer"

	TopoCoord(float a = 0, float e = 0, float r = 0, float rr = 0) :
			az(a), el(e), range(r), rangeRate(rr) {
	}
};

struct tSphericalCoord {
    float az;
    float el;

    tSphericalCoord(float _azimuth=0, float _elevation=0) : az(_azimuth), el(_elevation){};
};

struct TopoPoint{
    TopoCoord pos;  //!< угловые координаты, радиан
    tSphericalCoord vel; //!< угловые скорости, радиан/с
    JullianDate date; //!<момент времени измерения pos и vel
    TopoPoint(){};
    TopoPoint(const TopoCoord& p, const tSphericalCoord& v, const JullianDate& d) : pos(p), vel(v), date(d){};
};

//!  EciPoint + имя
struct TopoPointName {
	TopoPoint e_point; //!<координаты точки в геодезической СК и скорость их изменения
	QString name_ka;
};

TopoPoint eciToTopoPoint(const GeoCoord& recv, const EciPoint& target);

//! параметры солнца в градусах
struct SunParams {
	float az; //!< азимут, радиан
	float h; //!< высота, радиан
	float dec; //!< склонение, радиан

	SunParams(float azimuth = 0, float height = 0, float decl = 0) :
			az(azimuth), h(height), dec(decl) {
	}
};

//! координаты экрана
struct Screen {
	int x;
	int y;
	Screen();
};

struct DayDuration {
	DayDuration();
	DayDuration(int type);
	enum {
		DAY_LENGTH, NIGHT_LENGTH
	};
	int type;
	QDate date;
	meteo::GeoPoint geopos;
	QTime start;
	int length;

	bool calcLength(const meteo::GeoPoint& geopos, const QDate& date);
	meteo::GeoPoint solarPosition(int day, float hour);
};

SunParams sun_H(const GeoCoord& geo, const QDateTime& datetime);
bool calcDay(const GeoCoord& coord, const QDateTime& beg, float thresh);

GeoCoord eciToGeo(const EciCoord& eci, float lgmst);
EciCoord geoToEci(const GeoCoord& geo, float lgmst);
EciCoord geoToEciEasy(const GeoCoord& geo);
GeoCoord eciToGeoEasy(const EciCoord& eci);
TopoCoord eciToTopo(const GeoCoord& recv, const EciPoint& target);
GeoCoord usrToGeo(const UsrCoord& usr);
UsrCoord geoToUsr(const GeoCoord& geo);

float geoDistance(const GeoCoord& geo1, const GeoCoord& geo2);
float geoAzimooth(const GeoCoord& geo1, const GeoCoord& geo2);
GeoCoord findSecondCoord(const GeoCoord& geo1, const float d, const float a);
bool sputnik_distance(const GeoCoord& geo1, const float alpha,
		const float azimut, GeoCoord *acoord);
int findFrameForQuad(const Screen quad[], Screen frame[]);
int findFrameForQuad(const Screen quad[], Screen* leftTop, Screen* rightBottom);
int findFrameForQuad(const Position quad[], Screen* leftTop,
		Screen* rightBottom);

///Earth geometry///
bool surfacePointZ(float x0, float x1, float y0, float y1, float z0, float z1,
		float& x, float& y, float& z);
bool surfacePointY(float x0, float x1, float y0, float y1, float z0, float z1,
		float& x, float& y, float& z);
bool surfacePointX(float x0, float x1, float y0, float y1, float z0, float z1,
		float& x, float& y, float& z);
bool surfacePoint(const GeoCoord& posSatellite, const GeoCoord& posIIO,
		GeoCoord& posSurface, float& iioPath, float& surfacePath);

void beamPoint(const GeoCoord& posSatellite, const GeoCoord& posSurface,
		GeoCoord& posBeam, float dl);

float distance(const GeoCoord& posTransmitter, const GeoCoord& posReceiver);
float vectorsAngle(float projX1, float projY1, float projZ1, float projX2,
		float projY2, float projZ2); //rad

///Sun
float sunTerminatorLat(int year, int month, int day, int hour, int min, int sec,
		float lon);
void sunPoint(int year, int month, int day, int hour, int min, int sec,
		GeoCoord& pos);

QDataStream& operator<<(QDataStream& in, const UsrCoord& rc);
QDataStream& operator>>(QDataStream& out, UsrCoord& rc);
QDataStream& operator<<(QDataStream& in, const GeoCoord& gc);
QDataStream& operator>>(QDataStream& out, GeoCoord& gc);

//  GeoCoord& operator=(GeoCoord& gc1, GeoCoord& gc2);

}
;

#endif //COORDS_H

