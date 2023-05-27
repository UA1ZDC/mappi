#ifndef COMMONS_GEOBASIS_MERKATPROJ_H
#define COMMONS_GEOBASIS_MERKATPROJ_H

#include "projection.h"

namespace meteo
{
  class MerkatProj : public Projection
  {
  public:
    MerkatProj();
    MerkatProj( const GeoPoint& center );
    ~MerkatProj();

    enum {
      Type = MERCAT
    };
    int type() const { return Type; }

    bool F2X(const GeoVector& geoPoints, QVector<QPolygon>* resultPolygonVector, bool isClosed ) const;
    bool F2X(const GeoVector&,  QVector<QPolygon>* resultPolygonVector) const;

    bool F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const;
    bool X2F_one( const QPoint& meterCoord, GeoPoint* geoCoord ) const;

    bool F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const;
    bool X2F_one( const QPointF& meterCoord, GeoPoint* geoCoord ) const;

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

  protected:
    void initRamka();

  private:
    bool hasPointInside(int x1, int x2, QList<int> points) const;
    int countPolygonEntry( const QPolygon& poly, QList<QPolygon> polyList ) const;
    QPolygon combineIntersectPolygons( const QPolygon& base, const QList<QPolygon>& polyList ) const;


};

}

#endif
