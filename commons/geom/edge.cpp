#include "edge.h"

#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>

namespace meteo {
namespace geom {

Edge& Edge::rot()
{
  Point m = ( org_ + dest_ ) * 0.5;
  Point v = dest_ - org_;
  Point n( v.y(), -v.x() );
  org_ = m - n * 0.5;
  dest_ = m + n * 0.5;
  return *this;
}

Edge& Edge::flip()
{
  return rot().rot();
}

Point Edge::point( float d ) const
{
  return Point( org_ + ( dest_ - org_ ) * d );
}

float dotProduct( const Point& p, const Point& q )
{
  return ( p.x() * q.x() + p.y() * q.y() );
}

Edge::Orient Edge::intersect( const Edge& e, float* t, QPointF* p ) const
{
  Point a = org_;
  Point b = dest_;
  Point c = e.org();
  Point d = e.dest();
  Point n = Point( (d-c).y(), (c-d).x() );
  float denom = dotProduct( n, b-a );
  if ( true == MnMath::isZero(denom) ) {
    int aclass = org_.classify(e);
    if ( Point::LEFT == aclass || Point::RIGHT == aclass ) {
      return PARALLEL;
    }
  }
  float num = dotProduct( n, a - c );
  *t = -num / denom;
  if ( 0 != p ) {
    *p = Edge::point(*t).qpointf();
  }
  return SKEW;
}

Edge::Orient Edge::cross( const Edge& e, float* t ) const
{
  float s;
  Orient crossType = e.intersect( *this, &s );
  if ( COLLINEAR == crossType || PARALLEL == crossType ) {
    return crossType;
  }
  if ( 0.0 > s || 1.0 < s ) {
    return SKEW_NO_CROSS;
  }
  intersect( e, t );
  if ( 0.0 <= *t && 1.0 >= *t ) {
    return SKEW_CROSS;
  }

  return SKEW_NO_CROSS;
}

Edge::EdgePlace Edge::edgeType( const Point& p ) const
{
  Point v = org_;
  Point w = dest_;
  switch ( p.classify(*this) ) {
    case Point::LEFT:
      return ( ( v.y() < p.y() ) && ( p.y() <= w.y() ) ) ? CROSSING : INESSENTIAL ;
    case Point::RIGHT:
      return ( ( w.y() < p.y() ) && ( p.y() <= v.y() ) ) ? CROSSING : INESSENTIAL ;
    case Point::BETWEEN:
    case Point::ORIGIN:
    case Point::DESTINATION:
      return TOUCHING;
    default:
      return INESSENTIAL;
  }
}

}
}
