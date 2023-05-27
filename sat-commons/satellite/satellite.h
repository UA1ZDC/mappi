#ifndef COMMONS_SATELLITE_SATELLITE_H
#define COMMONS_SATELLITE_SATELLITE_H

#include <commons/geobasis/coords.h>
//#include <commons/settings/mnsettings.h>

#include <sat-commons/satellite/satellitebase.h>
#include <sat-commons/satellite/tracktopopoint.hpp>


namespace MnSat {
  //! Общая информация о данных полученных из потока
  // struct SatInfo {
  //   MnSettings::StreamType streamType;
  //   QString name; //!< Название спутника (кодовое)
  //   QDateTime beg; //!< Время начала приёма
  //   QDateTime end; //!< Время окончания приёма
  //   Coords::GeoCoord site; //!< Координаты пункта приёма
  //   MnSat::TLEParams tle;  //!< Параметры орбиты

  //   int getSatInfo( const QString& infoName );
  // };
  // int getSatInfo( const QString& infoName, MnSat::SatInfo& sInfo );
  
  //! кандидат для попадания в координатную сетку
  /* struct CgCandidte { */
  /*   float value; */
  /*   float remainder; */
  /*   bool flag; */
  /*   CgCandidte():value(0), remainder(0), flag(false) {} */
  /* }; */
  
  /* struct CgValue { */
  /*   uint row; */
  /*   uint col; */
  /*   float val; */
  /*   CgValue(uint r=0, uint c=0, float v=0):row(r),col(c),val(v) {} */
  /* }; */

  //bool spherToSat(const Satellite& sat, const TOrbParam *opar, double lat, double lon,
	//	  double *time, double *jj);

};

//!расчет орбитальных параметров спутника
class Satellite : public SatelliteBase {
  public:
  Satellite();
  ~Satellite();

  //int setTLEParams( const QString& infoName );

  bool search_AOS_LOS(time_t, const Coords::GeoCoord&, time_t*, time_t*, float threshold = 0.0)const;
  bool searchMaximum(time_t , const Coords::GeoCoord&, double*, time_t* = 0 ) const;
  bool searchNearLatTime(time_t aos, time_t los, const Coords::GeoCoord& site, time_t* near)const;
  bool searchNearLonTime(time_t aos, time_t los, const Coords::GeoCoord& site, time_t* near)const;
  

	bool trajectory(const QDateTime& beg, const QDateTime& end, float step,
			const meteo::GeoPoint& site, QList<MnSat::TrackTopoPoint>& track);
	bool trajectory(const QDateTime& beg, const QDateTime& end, float step,
			const Coords::GeoCoord& site,const QString& fileN,
			QVector<MnSat::TrackTopoPoint>& track);
	bool trajectory(const QDateTime& beg, const QDateTime& end, float step, QVector<Coords::GeoCoord>& track);
  bool trajectory(const QDateTime& beg, float step, float maxAngle, const Coords::GeoCoord& site, 
		  QVector<Coords::GeoCoord>& futur, QVector<Coords::GeoCoord>& past);
  bool trajectory(const QDateTime& beg, const QDateTime& end, float step, QVector<Coords::EciPoint>& track);
bool trajectoryTimeIn(const QDateTime& beg, const QDateTime& end, float step,int type_map,QDateTime& ret_time_start,QDateTime& ret_time_end,bool ug_);
/*  bool sat2TOrbParam(TOrbParam *Seans,const Coords::GeoCoord& site,QDateTime
     dt=QDateTime::currentDateTime(Qt::UTC),QDateTime
     dtends=QDateTime::currentDateTime(Qt::UTC),double angle=0.,int typ=HRPT_NOAA, int width = 0, int height = 0);*/
  /* bool sat2TOrbParam(TOrbParam *Seans, QDateTime dt = QDateTime::currentDateTime(Qt::UTC), QDateTime dtends = QDateTime::currentDateTime(Qt::UTC),
     int typ = HRPT_NOAA, int width = 0, int height = 0);*/
  int direction(const QDateTime& beg, const QDateTime& end);


 private:

	bool trajectory(const QDateTime& beg, const QDateTime& end, float step,
			const Coords::GeoCoord& site, QList<MnSat::TrackTopoPoint>& track);
  

};

#endif
