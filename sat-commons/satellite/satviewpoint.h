#ifndef COMMONS_SATELLITE_SATVIEWPOINT_H
#define COMMONS_SATELLITE_SATVIEWPOINT_H

#include <qvector.h>
#include <commons/geobasis/coords.h>
#include <commons/geobasis/geovector.h>
#include <sat-commons/satellite/satellitebase.h>

//! расчёт координат области сканирования спутника
class SatViewPoint: public SatelliteBase {
public:
  
  SatViewPoint();
  SatViewPoint(const QDateTime& dtStart, const QDateTime& dtEnd);
  ~SatViewPoint();

  QDateTime dtStart() const { return _dtStart; }
  QDateTime dtEnd() const   { return _dtEnd;   }
  void setDateTime(const QDateTime& dtStart, const QDateTime& dtEnd);
  void setDtStart(const QDateTime& dtStart) {setDateTime(dtStart, _dtEnd);};
  void setDtEnd(const QDateTime& dtEnd) {setDateTime(_dtStart, dtEnd);};

  //долгота +/-Pi
  bool countGrid(float gStep, float vStep, float maxAngle, QVector<Coords::GeoCoord>& points) const;
  bool countGridPart(float gStep, int vStep, int linesCnt, float maxAngle, QVector<Coords::GeoCoord>& points) const;
  bool countBorder(int vStep, float maxAngle, meteo::GeoVector* points) const;
  bool countMinMaxBorders(float gStep, int vStep, float maxAngle,
			  Coords::GeoCoord* min, Coords::GeoCoord* max) const;

  bool countGridCorners(float maxAngle, meteo::GeoPoint* bl, meteo::GeoPoint* br, meteo::GeoPoint* el, meteo::GeoPoint* er) const;
  bool countGridCorners(float maxAngle, meteo::GeoPoint* gp ) const;

  bool countGridCorners(float maxAngle, Coords::GeoCoord* bl, Coords::GeoCoord* br, Coords::GeoCoord* el, Coords::GeoCoord* er) const;
  bool countGridCorners(float maxAngle, Coords::GeoCoord* gc ) const;

private:

  bool countScanLine( const QDateTime& dt, float sigma_step, float maxAngle,
		      QVector<Coords::GeoCoord>& points, Coords::GeoCoord* oldGeo ) const;
  bool addViewedPoint(const Coords::GeoCoord& geo, float sigma, int sign, int lonSign,
		      QVector<Coords::GeoCoord>& points) const;
  void countViewedPoint( float lat, float alt, double sigma, 
			 double* v_lat, double* delta_lon, int) const;


  bool countViewedMaxPoints(const QDateTime& cur, float maxAngle, 
			    Coords::GeoCoord* left, Coords::GeoCoord* right) const;
  void countViewedMaxPoints(const Coords::GeoCoord& geo, double maxAngle, int sign, 
			    Coords::GeoCoord* lgeo, Coords::GeoCoord* mgeo) const;


private:
  QDateTime _dtStart; //!< время начала сканирования в UTC
  QDateTime _dtEnd;   //!< время окончания сканирования в UTC
  bool _isOk;
  
};

#endif
