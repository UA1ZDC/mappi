#ifndef COMMONS_GEOBASIS_GEODATA_H
#define COMMONS_GEOBASIS_GEODATA_H

#include "geopoint.h"

#include <qvector.h>

namespace meteo {
  
  enum typeGeoData{
    DEFAULT_GEO_DATA = 0, //!< исходное значение
    FLUX_X_GEO_DATA = 1,    //!< первая производная по х
    FLUX_Y_GEO_DATA = 2     //!< первая производная по y
  };
  
  class MData{
    
  public:
    meteo::GeoPoint point; //!< координаты
    float data; //!< значение метео-параметра
    int mask;    //!< TRUE_DATA - данные походят для интерполяции, FALSE_DATA - не подходят
    QString id; //!< идентификатор записи в БД
    typeGeoData type_data;
    float w;    //!<Вес значения
    MData(){
      point.setLat(0.);
      point.setLon(0.);
      data = -9999.;
      mask = 0;
      type_data = DEFAULT_GEO_DATA;
      id = "";
      w =1.;
    }
    MData(float fi,float la, float znach, int amask = 1, const QString& aid = "",
      typeGeoData atype_data= DEFAULT_GEO_DATA, float aw = 1.){
      point.setLat(fi);
      point.setLon(la);
      data = znach;
      id = aid;
      mask = amask;
      type_data = atype_data;
      w = aw;
    }

   MData( const meteo::GeoPoint& gp, float znach, int amask = 1, const QString& aid = "",
   typeGeoData atype_data= DEFAULT_GEO_DATA, float aw = 1.)
      : point(gp),
      data(znach),
      mask(amask),
      id(aid),
      type_data(atype_data),
      w(aw)
    {
    }

    friend QDataStream& operator<<(QDataStream& in,const MData& gc);
    friend QDataStream& operator>>(QDataStream& out, MData& gc);
    
  };
  
  inline bool operator==(const MData &p1, const MData &p2)
  {
    return qFuzzyCompare(p1.point.lat(), p2.point.lat()) 
    && qFuzzyCompare(p1.point.lon(), p2.point.lon())
    && qFuzzyCompare(p1.point.alt(), p2.point.alt())
    && qFuzzyCompare(p1.data, p2.data)
    && (p1.mask == p2.mask)
    && (p1.w == p2.w);
  }
  
  inline bool operator!=(const MData &p1, const MData &p2)
  {
    return !qFuzzyCompare(p1.point.lat(), p2.point.lat()) 
    || !qFuzzyCompare(p1.point.lon(), p2.point.lon())
    || !qFuzzyCompare(p1.point.alt(), p2.point.alt())
    || !qFuzzyCompare(p1.data, p2.data)
    || (p1.mask != p2.mask)
    || (p1.w != p2.w);
  }
  
  

 
class GeoData : public QVector<MData> {
  public:
    GeoData();
    GeoData(int size );
    ~GeoData();
     
    void to360();
    void to180();
    
    const GeoPoint& maxAlt() const;
    void move(const GeoPoint&);
    void sortByFi();
    void sortByLa();
    void addDeltaData(float );
    void mnozData(float );
    
    
    inline GeoData& operator=( const QVector<MData>& gv );
    inline GeoData& operator+=( const QVector<MData>& gv );

  
  friend TLog& operator<<( TLog& log, const GeoData& gc ); 
    
};

GeoData& GeoData::operator=( const QVector<MData>& gv )
{
  QVector<MData>::operator=(gv);
  return *this;
}

GeoData& GeoData::operator+=( const QVector<MData>& gv )
{
  QVector<MData>::operator+=(gv);
  return *this;
}

}

#endif
