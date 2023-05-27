#include "polygon.h"

namespace meteo {
namespace geom {

Polygon::Polygon( Polygon& p )
{
  size_ = p.size_;
  if ( 0 == size_ ) {
    v_ = 0;
  }
  else {
    v_ = new Vertex( p.point() );
    for ( int i = 1; i < size_; ++i ) {
      p.advance( Vertex::CLOCKWISE );
      v_ = v_->insert( new Vertex( p.point() ) );
    }
    p.advance( Vertex::CLOCKWISE );
    v_ = v_->cw();
  }
}

Polygon::Polygon( Vertex* v )
  : v_(v)
{
  resize();
}

Polygon::~Polygon()
{
  if ( 0 != v_ ) {
    Vertex* w = v_->cw();
    while ( v_ != w ) {
      delete w->remove();
      w = v_->cw();
    }
    delete v_;
  }
}

void Polygon::resize()
{
  if ( 0 == v_ ) {
    size_ = 0;
  }
  else {
    Vertex* v = v_->cw();
    for ( size_ = 1; v != v_; ++size_, v = v->cw() ) {}
  }
}

Vertex* Polygon::insert( const Point& p )
{
  if ( 0 == size_++ ) {
    v_ = new Vertex(p);
  }
  else {
    v_ = v_->insert( new Vertex(p) );
  }
  return v_;
}

void Polygon::remove()
{
  Vertex* v = v_;
  v_ = ( 0 == --size_ ) ? 0 : v_->ccw() ;
  delete v->remove();
}

Polygon::PointPlace Polygon::inPolygon( const Point& p )
{
  int parity = 0;
  for ( int i = 0; i < size_; ++i, advance(Vertex::CLOCKWISE) ) {
    Edge e = edge();
    switch ( e.edgeType(p) ) {
      case Edge::TOUCHING:
        return BOUNDARY;
      case Edge::CROSSING:
        parity = 1 - parity;
      default:
        break;
    }
  }
  return ( 0 != parity ) ? INSIDE : OUTSIDE ;
}

}
}
