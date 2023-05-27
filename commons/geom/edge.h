#ifndef COMMONS_GEOM_EDGE_H
#define COMMONS_GEOM_EDGE_H

#include <float.h>

#include "point.h"

namespace meteo {
namespace geom {

class Edge
{
  public:

    enum Orient {
      COLLINEAR = 0,
      PARALLEL  = 1,
      SKEW      = 2,
      SKEW_CROSS        = 3,
      SKEW_NO_CROSS     = 4
    };

    enum EdgePlace {
      TOUCHING          = 0,
      CROSSING          = 1,
      INESSENTIAL       = 2
    };

    inline Edge( const Point& o, const Point& d )
      : org_(o), dest_(d){}
    inline Edge( const QPoint& o, const QPoint& d )
      : org_(o), dest_(d){}
    inline Edge( const QPointF& o, const QPointF& d )
      : org_(o), dest_(d){}
    inline Edge()
      : org_( Point( 0.0, 0.0 ) ), dest_( Point( 0.0, 1.0 ) ){}

    Edge& rot();
    Edge& flip();
    Point point( float d ) const ;
    Orient intersect( const Edge& e, float* d, QPointF* p = 0 ) const ;
    Orient cross( const Edge& e, float* d ) const ;
    inline bool isVerical() const
      { return ( org_.x() == dest_.x() ); }
    inline float slope() const
      { return ( false == isVerical() )
        ? ( dest_.y() - org_.y() )/( dest_.x() - org_.x() ) : DBL_MAX ; }
    inline float y( float x ) const
      { return slope() * ( x - org_.x() ) + org_.y(); }

    inline const Point& org() const
      { return org_; }
    inline const Point& dest() const
      { return dest_; }

    EdgePlace edgeType( const Point& p ) const ;

  private:
    Point org_;
    Point dest_;
};

}
}

#endif
