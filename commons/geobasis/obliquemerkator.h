#ifndef COMMONS_GEOBASIS_OBLIQUIEMERCAT_H 
#define COMMONS_GEOBASIS_OBLIQUIEMERCAT_H 

#include "projection.h"

namespace meteo
{
class ObliqueMerkat : public Projection
{
  public:
    ObliqueMerkat();
    ~ObliqueMerkat();

    enum {
      Type = OBLIQUEMERKAT
    };
    int type() const { return Type; }

    void setMapCenter( const GeoPoint& cntr, float azimuth );

    bool F2X( const GeoVector& geoPoints, QVector<QPolygon>* scrPoints, bool isClosed ) const;
    bool F2X_one(const GeoPoint& geoCoord, QPoint* meterCoord ) const;
    bool X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const;

    bool F2X_one(const GeoPoint& geoCoord, QPointF* meterCoord ) const;
    bool X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const;

    Projection* copy() const ;
      
  protected:
    void initRamka();

  private:
    float sin_fip_;
    float cos_fip_;
    GeoPoint origin_;
    float azimuth_;
    GeoPoint pole_;
};

}

#endif

