#include "point.h"

#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mathematical_constants.h>

#include "edge.h"

namespace meteo {
namespace geom {

Point::PointPos Point::classify( const Point& p0, const Point& p1 ) const
{
  Point p2 = *this;
  if ( p0 == p2 ) {
    return ORIGIN;
  }
  else if ( p1 == p2 ) {
    return DESTINATION;
  }
  Point a = p1 - p0;
  Point b = p2 - p0;
  float sa = a.x() * b.y() - b.x() * a.y();
  if ( 0.0 < sa ) {
    return LEFT;
  }
  else if ( 0.0 > sa ) {
    return RIGHT;
  }
  else if ( ( a.x() * b.x() < 0.0 ) || ( a.y() * b.y() < 0.0 ) ) {
    return BEHIND;
  }
  else if ( a.length() < b.length() ) {
    return BEYOND;
  }
  return BETWEEN;
}

Point::PointPos Point::classify( const Edge&e ) const 
{
  return classify( e.org(), e.dest() );
}

float Point::polarAngle() const
{
  if ( 0.0 == x_ && 0.0 == y_ ) {
    return -1.0;
  }
  else if ( 0.0 == x_ ) {
    return (0.0 > y_ ) ? M_PI_2 : M_PI_2 + M_PI;
  }
  float theta = ::atan( y_ / x_ );
  if ( 0.0 < x_ ) {
    return ( y_ >= 0.0 ) ? theta : theta + M_PI * 2.;
  }
  return M_PI + theta;
}

float Point::distance( const Edge& e, QPointF* p ) const
{
  Edge ab = e;
  ab.flip().rot();
  Point n( ab.dest() - ab.org() );
  n = n * ( 1.0 / n.length() );
  Edge f( *this, *this + n );
  float t;
  f.intersect( e, &t, p );
  return t;
}

}
}
