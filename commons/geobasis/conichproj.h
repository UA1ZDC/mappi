#ifndef COMMONS_GEOBASIS_CONICHPROJ_H
#define COMMONS_GEOBASIS_CONICHPROJ_H

#include "projection.h"

namespace meteo
{

class ConichProj : public Projection
{
  public:
    ConichProj();
    ConichProj( const GeoPoint& center );
    ~ConichProj();

    enum {
      Type = CONICH
    };
    int type() const { return Type; }

    bool F2X(const GeoVector& geoPoints, QVector<QPolygon>* resultPolygonVector, bool isClosed ) const;
    bool F2X(const GeoVector&,  QVector<QPolygon>* resultPolygonVector) const;

    GeoPoint start() const ;
    GeoPoint end() const ;
    float startFi() const ;
    float startLa() const ;
    float endFi() const ;
    float endLa() const ;

    void setMapCenter( const GeoPoint& mapcenter );

    GeoPoint getProjCenter() const ;

    void setStart( const GeoPoint& gp ) ;
    void setEnd( const GeoPoint& gp ) ;

    void setStartFi( float lat ) ;
    void setStartLa( float lon ) ;
    void setEndFi( float lat ) ;
    void setEndLa( float lon ) ;

    Projection* copy() const ;

    bool isEqual( const Projection& proj ) const ;

    float conich_n() const { return conich_n_; }
    float lonfactor() const { return conich_n_*hemisphere_; }

  protected:
    void initRamka();

  private:
    bool F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const;
    bool X2F_one( const QPoint& meterCoord, GeoPoint* geoCoord ) const;

    bool F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const;
    bool X2F_one( const QPointF& meterCoord, GeoPoint* geoCoord ) const;

  private:
    float phi0_;
    float phi1_;
    float phi2_;
    float conich_n_;
    float conich_c_;
    bool hasPointInside(int x1, int x2, QList<int> points) const;
    int countPolygonEntry( const QPolygon& poly, QList<QPolygon> polyList ) const;
    QPolygon combineIntersectPolygons( const QPolygon& base, const QList<QPolygon>& polyList ) const;
};

}

#endif
