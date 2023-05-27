#ifndef COMMONS_GEOBASIS_GENERALPROJ_H 
#define COMMONS_GEOBASIS_GENERALPROJ_H 

#include "projection.h"

namespace meteo {

class GeneralProj;

class GeneralProj : public Projection
{
  public:
    void setFuncTransform( fF2X_one f2x, fX2F_one x2f );
    void setFuncTransform( fF2X_onef f2x, fX2F_onef x2f );

    static bool linearF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord );
    static bool linearX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord );

    static bool linearF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord );
    static bool linearX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord );

    GeneralProj();
    GeneralProj( const GeoPoint& center );
    ~GeneralProj();

    enum {
      Type = GENERAL
    };
    int type() const { return Type; }

    bool F2X( const GeoVector& geoPoints, QVector<QPolygon>* scrPoints, bool isClosed ) const;
    
//    void setRamka(int x0, int y0,int x1, int y1);
    void setRamka( float x0, float y0, float x1, float y1);
    
    virtual void initRamka();

    Projection* copy() const ;
    
  private:
    bool F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const;
    bool X2F_one( const QPoint& meterCoord, GeoPoint* geoCoord ) const;
    bool F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const;
    bool X2F_one( const QPointF& meterCoord, GeoPoint* geoCoord ) const;
};

}

#endif

