#ifndef COMMONS_SATELLITE_SATELLITEBASE_H
#define COMMONS_SATELLITE_SATELLITEBASE_H

////////////////////////
#define R_E 6371.0
//#define GENERAL -1
//#define STEREO   0
//#define MERCAT   1
//#define CONICH   2
//////////////////////


#define CC  1738.3048
#define z_05t 43082.0
#define hGST  42178.0

#define xke   7.43669161E-2
#define xkmper    6.378137E3    /* WGS 84 Earth radius km */
#define xmnpda    1.44E3      /* Minutes per day */
#define tothrd    6.6666666666666666E-1 /* 2/3 */
#define ck2   5.413079E-4
#define ck4   6.209887E-7
#define ae    1.0
#define ssssss    1.012229
#define qoms2t    1.880279E-09
#define xj3   -2.53881E-6   /* J3 Harmonic (WGS '72) */   
#define e6a   1.0E-6
#define pio2    1.57079632679489656 /* Pi/2 */
#define x3pio2    4.71238898038468967 /* 3*Pi/2 */
#define twopi   6.28318530717958623 /* 2*Pi  */
//#define deg2rad   1.745329251994330E-2  /* Degrees to radians */
#define secday    8.6400E4  /* Seconds per day */
#define ffffff    3.35281066474748E-3 /* Flattening factor */
#define omega_E   1.00273790934 /* Earth rotations/siderial day */


const double zns    =  1.19459E-5;     
const double c1ss   =  2.9864797E-6;   
const double zes    =  0.01675;        
const double znl    =  1.5835218E-4;   
const double c1l    =  4.7968065E-7;   
const double zel    =  0.05490;
const double zcosis =  0.91744867;
const double zsinis =  0.39785416;     
const double zsings = -0.98088458;    
const double zcosgs =  0.1945905;      
const double q22    =  1.7891679E-6;   
const double q31    =  2.1460748E-6;   
const double q33    =  2.2123015E-7;   
const double g22    =  5.7686396;      
const double g32    =  0.95240898;     
const double g44    =  1.8014998;
const double g52    =  1.0508330;     
const double g54    =  4.4108898;      
const double root22 =  1.7891679E-6;  
const double root32 =  3.7393792E-7;   
const double root44 =  7.3636953E-9;  
const double root52 =  1.1428639E-7;
const double root54 =  2.1765803E-9;  
const double thdt   =  4.3752691E-3;

#define zcoshs    1
#define zsinhs    0

/* Entry points of Deep() */
#define dpinit   1 /* Deep-space initialization code */
#define dpsec    2 /* Deep-space secular code        */
#define dpper    3 /* Deep-space periodic code       */

/* Flow control flag definitions */
#define ALL_FLAGS              -1
#define SGP_INITIALIZED_FLAG   0x000001 /* not used */
#define SGP4_INITIALIZED_FLAG  0x000002
#define SDP4_INITIALIZED_FLAG  0x000004
#define SGP8_INITIALIZED_FLAG  0x000008 /* not used */
#define SDP8_INITIALIZED_FLAG  0x000010 /* not used */
#define SIMPLE_FLAG            0x000020
#define DEEP_SPACE_EPHEM_FLAG  0x000040
#define LUNAR_TERMS_DONE_FLAG  0x000080
#define NEW_EPHEMERIS_FLAG     0x000100 /* not used */
#define DO_LOOP_FLAG           0x000200
#define RESONANCE_FLAG         0x000400
#define SYNCHRONOUS_FLAG       0x000800
#define EPOCH_RESTART_FLAG     0x001000
#define VISIBLE_FLAG           0x002000
#define SAT_ECLIPSED_FLAG      0x004000

#define size_str  22180
#define beg_AVHRR  1500
#define end_AVHRR  21980
#define step_I  10
#define SIZE_HEAD  1078
#define OMEGAz  0.00007292115

#define TLE_START_CODE1 0
#define TLE_START_CODE2 0
#define TLE_LONG_CODE 1
#define TLE_CODE_LINE1 1
#define TLE_CODE_LINE2 2

#include <qstring.h>
#include <commons/coord_systems/jullian.h>
#include <commons/geobasis/geopoint.h>

class  NoradModelBase;
namespace Coords {
  class EciPointName;
  class EciPoint;
  class GeoCoord;
  class TopoCoord;
  class TopoPoint;
};
class QDateTime;

namespace MnSat {

  //! Параметры TLE телеграммы
  struct TLEParams {
    uint32_t epochYear;     //год
    double epochDay;    //эпоха (день года и дробная часть дня)
    double bstar;       //коэффициент торможения SGP4-типа (bstar drag term), размерность - (радиус земли)^-1
    double inclination; //наклонение, радиан (от 0 до PI)
    double raan;        //Прямое восхождение восходящего узла, радиан
    double eccentr;     //Эксцентриситет
    double perigee;     //Аргумент перигея, радиан
    double meanAnomaly; //Средняя аномалия, радиан
    double meanMotion;  //Среднее движение, радиан/мин
    uint64_t revolNumber; //Номер витка в эпоху [оборотов]
  };

  //! TLE данные
  struct STLEParams {
    QString satName; //!< Название спутника
    QString firstString; //!< Первая строка орбитальных элементов
    QString secondString; //!< Вторая строка орбитальных элементов
  };
  
}

QDataStream& operator<<(QDataStream& out, const MnSat::TLEParams& tp);
QDataStream& operator>>(QDataStream& in, MnSat::TLEParams& tp);

class SatelliteBase {
public:
  SatelliteBase();
  virtual ~SatelliteBase();

  bool readTLE( const QString& satName, const QString& fileNameTLE );
  bool readTLE(const MnSat::STLEParams& stle);
  void setTLEParams(const MnSat::TLEParams& tle);

  bool parseTLE( const QString& satName, const QString& fileNameTLE);
  bool parseTLE(const MnSat::STLEParams& stle);

  static bool getSTLE( const QString& satName, const QString& fileNameTLE, MnSat::STLEParams* stle );
  const MnSat::TLEParams& getTLEParams() const { return _tle; }

  //! Return satellite coords data at given minutes since element's epoch.
  bool getPosition( double tsince, Coords::EciPoint *eci ) const;
  bool getPosition( const QDateTime& dt, Coords::EciPoint *eci ) const;

  bool getPosition( double tsince, Coords::GeoCoord *geo) const;
  bool getPosition( const QDateTime& dt, Coords::GeoCoord *geo ) const;

  bool getPosition( double tsince, const Coords::GeoCoord& site, Coords::TopoCoord* topo) const;
  bool getPosition( const QDateTime& dt, const Coords::GeoCoord& site, Coords::TopoCoord* topo) const;

  bool getPosition( double tsince, const Coords::GeoCoord& site, Coords::TopoPoint* topo) const;
  bool getPosition( const QDateTime& dt, const Coords::GeoCoord& site, Coords::TopoPoint* topo) const;

  bool getPosition( double tsince, const Coords::GeoCoord& site, Coords::TopoCoord* topo, Coords::GeoCoord* geo ) const;
  bool getPosition( const QDateTime& dt, const Coords::GeoCoord& site, Coords::TopoCoord* topo, Coords::GeoCoord* geo ) const;


  double inclination()  const { return _tle.inclination; }
  double eccentricity() const { return _tle.eccentr; }
  double meanAnomaly()  const { return _tle.meanAnomaly; }
  double meanMotion()   const { return _meanMotion; }
  double bstar() const {  return _tle.bstar; }
  double raan()  const {  return _tle.raan; }
  unsigned long revolNumber( const QDateTime& dt ) const;
  unsigned long revolNumber(uint year, double day ) const;

  double semiMinor() const     { return _aeAxisSemiMinorRec; }
  double meanMotionRec() const { return _meanMotionRec; }
  double argPerigee() const    { return _tle.perigee; }
  double apogee()  {return _kmApogeeRec; }  
  double perigee() {return _kmPerigeeRec; }  

  unsigned long revol() { return _tle.revolNumber;}

  JullianDate epoch()const { return _jdEpoch; }

  const QString& name() const { return _name; }

  double timeFromTLE( const QDateTime& dt ) const;

  static bool readSatelliteList( const QString& fileNameTLE, QStringList* nameSattelite, const int type = 0 );
  bool readPozitionNow( const QString& fileNameTLE,const QDateTime& beg,QVector<Coords::EciPointName>&position_s);

  void lastParams( double* raan, double* u, double* dt, double* i ) const ;

 private:
  static bool isValidLineTLE( const QString& line );
  void initVariables();

private:
  MnSat::TLEParams _tle;
  NoradModelBase* _noradModel;
  JullianDate _jdEpoch;
  QString _name = "unknown";

   //!variables recovered from the input TLE elements
   double _aeAxisSemiMinorRec;  //!< semi-minor axis, in AE units
   double _aeAxisSemiMajorRec;  //!< semi-major axis, in AE units
   double _meanMotion;          //!< rev per day
   double _meanMotionRec;       //!< radians per minute
   double _kmPerigeeRec;        //!< perigee, in km
   double _kmApogeeRec;         //!< apogee, in km

};

#endif
