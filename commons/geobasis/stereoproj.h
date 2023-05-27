#ifndef COMMONS_GEOBASIS_STEREOPROJ_H 
#define COMMONS_GEOBASIS_STEREOPROJ_H 

#include "projection.h"

namespace meteo
{
  class StereoProj : public Projection
  {
  public:
    StereoProj();
    StereoProj( const GeoPoint& center );
    ~StereoProj();

    enum {
      Type = STEREO
    };
    int type() const { return Type; }

    bool F2X( const GeoVector& geoPoints, QVector<QPolygon>* scrPoints, bool isClosed ) const;
    bool checkPolush( int map );

    GeoPoint getProjCenter() const ;

    float startFi() const ;
    float startLa() const ;
    float endFi() const ;
    float endLa() const ;

    void setStart( const GeoPoint& gp );
    void setEnd( const GeoPoint& gp );

    void setStartFi( float lat );
    void setStartLa( float lon );
    void setEndFi( float lat );
    void setEndLa( float lon );

    void setHemisphere( Hemisphere hs );
    void setNorthern();
    void setSouthern();

    Projection* copy() const ;
    bool isInMap(const GeoPoint& );
      
  protected:
    void initRamka();

  private:
    bool F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const;
    bool X2F_one( const QPoint& meterCoord, GeoPoint* geoCoord ) const;
    bool F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const;
    bool X2F_one( const QPointF& meterCoord, GeoPoint* geoCoord ) const;
    void setMapCenter( const GeoPoint& );
};

}

#endif

