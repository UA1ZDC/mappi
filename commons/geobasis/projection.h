#ifndef COMMONS_GEOBASIS_PROJECTION_H
#define COMMONS_GEOBASIS_PROJECTION_H

#include <qstring.h>
#include <qlist.h>
#include <qpolygon.h>
#include <qpoint.h>
#include <qtransform.h>

#include <cross-commons/includes/tcommoninclude.h>
#include <commons/mathtools/mnmath.h>

#include <commons/geobasis/geovector.h>

namespace meteo {

class Projection;

typedef bool (*fF2X_one)( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord ) ;
typedef bool (*fX2F_one)( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord ) ;

typedef bool (*fF2X_onef)( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord ) ;
typedef bool (*fX2F_onef)( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord ) ;

const float kSTEREO_MIN_LAT_DEG = -12.1;
const float kSTEREO_PARAM2 = kSTEREO_MIN_LAT_DEG*DEG2RAD;

const int    kRVS        = 67108864;
const int    kRVSG       = 1000000;
const float kEarthRadius   = 6371000.0;
const float kEarthRadiusKM   = kEarthRadius*0.001;

const float  kPR_DOPUSK     = 12.*kPI180;           //!< нижняя широта для геостроф. ветра

//!тип проекции
enum ProjType
{
  GENERAL        =-1,  //!< специальный тип проекции для построения графиков
  STEREO         = 0,  //!< стереографическая,
  MERCAT         = 1,  //!< меркатор
  CONICH         = 2,  //!< коническая
  OBLIQUEMERKAT  = 3,  //!< проекция косая меркатора
  POLARORBSAT    = 4,  //!< проекция полярно-орбиального спутника)
  NONE_PROJ      = 5
};

enum Hemisphere {
  Northern = 1,
  Southern = -1
};

class Projection
{
  public:
    Projection();
    Projection( const GeoPoint& center );

    enum {
      Type = NONE_PROJ,
      UserType = 65536
    };
    virtual int type() const { return Type; }

    static Projection* createProjection( ProjType type );
    static Projection* createProjection( ProjType type, const GeoPoint& pnt );

    virtual ~Projection() {}

    virtual Projection* copy() const  = 0;

    virtual bool F2X( const GeoVector& geoPoints, QVector<QPolygon>* scrPoints, bool isClosed ) const = 0;
    virtual bool X2F( const QPolygon& poly, GeoVector* gv ) const ;

    virtual bool F2X_one(const GeoPoint& geoCoord, QPoint* meterCoord ) const = 0;
    virtual bool X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const = 0;

    virtual bool F2X_one(const GeoPoint& geoCoord, QPointF* meterCoord ) const = 0;
    virtual bool X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const = 0;

    virtual void setHemisphere( Hemisphere hs ) { hemisphere_ = hs; }
    virtual void setNorthern() { hemisphere_ = Northern; }
    virtual void setSouthern() { hemisphere_ = Southern; }

    virtual void setMapCenter( const GeoPoint& mapcenter );
    GeoPoint getMapCenter() const { return centerMap_; }

    virtual GeoPoint getProjCenter() const { return centerMap_; }

    Hemisphere hemisphere() const { return hemisphere_; }
    virtual bool isEqual( const Projection& proj ) const ;
    bool isEqual( Projection* proj ) const ;

    bool checkPolush(int map);

    const GeoVector& ramka() const { return ramka_; }
    void setRamka( const GeoVector& r ) { ramka_ = r; }

    virtual GeoPoint start() const { return start_; }
    virtual GeoPoint end() const { return end_; }
    virtual float startFi() const { return start_.lat(); }
    virtual float startLa() const { return start_.lon(); }
    virtual float endFi() const { return end_.lat(); }
    virtual float endLa() const { return end_.lon(); }

    virtual void setStart( const GeoPoint& gp ) { start_ = gp; }
    virtual void setEnd( const GeoPoint& gp ) { end_ = gp; }

    virtual void setStartFi( float lat ) { start_.setLat(lat); }
    virtual void setStartLa( float lon ) { start_.setLon(lon); }
    virtual void setEndFi( float lat ) { end_.setLat(lat); }
    virtual void setEndLa( float lon ) { end_.setLon(lon); }

    float xfactor() const { return xfactor_; }
    float yfactor() const { return yfactor_; }

    void setXfactor( float factor ) { xfactor_ = factor; }
    void setYfactor( float factor ) { yfactor_ = factor; }
    void setXYFactor( float factor ) { xfactor_ = factor; yfactor_ = factor; }

    virtual float lonfactor() const { return 1.0; }
    virtual bool isInMap(const GeoPoint& gp);
     
  protected:
    void init();
    virtual void initRamka() = 0;
    
    GeoPoint centerMap_;        //!< Точка - центр проекции (рад.)
    Hemisphere hemisphere_;     //!< Полушарие (1 - северное, -1 - южное)
    ProjType type_;             //!< Тип проекции
    float projRadius;           //!< Радиус сферы (м)
    float projRadius_1;         //!< Радиус сферы (м-1)
    float projRadius_MPI2;      //!< Радиус сферы (м)* M_PI_2
    GeoVector ramka_;           //!< Массив географических координат, описанный вокруг поверхности, представленной в данной проекции
    float xfactor_;            //!< множитель экранных координат по оси x
    float yfactor_;            //!< множитель экранных координат по оси y
    GeoPoint start_;            //!< начальные широта и долгта
    GeoPoint end_;              //!< конечные широта и долгота

    fF2X_one funcF2X_one_;      //!< функция преобразования географических координат в плоские (в проекции GENERAL можно установить произвольную)
    fX2F_one funcX2F_one_;      //!< функция преобразования плоских координат в географические (в проекции GENERAL можно установить произвольную)

    fF2X_onef funcF2X_onef_;      //!< функция преобразования географических координат в плоские (в проекции GENERAL можно установить произвольную)
    fX2F_onef funcX2F_onef_;      //!< функция преобразования плоских координат в географические (в проекции GENERAL можно установить произвольную)

    Projection* copyTo( Projection* proj ) const;

  private:
    DISALLOW_EVIL_CONSTRUCTORS(Projection);
};

inline bool Projection::checkPolush( int map )
{
  if ((map==0) || (map==61)) {
    return false;
  }
  return true;
}

template <class T> inline T projection_cast( meteo::Projection* p )
{
  return int(static_cast<T>(0)->Type) == int(meteo::Projection::Type)
    || ( 0 != p && int(static_cast<T>(0)->Type) == p->type() ) ? static_cast<T>(p) : 0;
}

template <class T> inline T projection_cast(const meteo::Projection* p )
{
  return int(static_cast<T>(0)->Type) == int(meteo::Projection::Type)
    || ( 0 != p && int(static_cast<T>(0)->Type) == p->type() ) ? static_cast<T>(p) : 0;
}

}


#endif

