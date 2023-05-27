#ifndef COMMONS_GEOM_POLYGON_H
#define COMMONS_GEOM_POLYGON_H

#include "node.h"
#include "edge.h"

namespace meteo {
namespace geom {

class Polygon
{
  public:

    enum PointPlace {
      INSIDE    = 0,
      OUTSIDE   = 1,
      BOUNDARY  = 2
    };

    inline Polygon()
      : v_(0), size_(0) {}
    Polygon( Polygon& p );
    Polygon( Vertex* v );
    ~Polygon();

    inline Vertex* vertex() const
      { return v_; }
    inline int size() const
      { return size_; }
    inline Point point() const
      { return v_->point(); }
    inline Edge edge() const
      { return Edge( point(), v_->cw()->point() ); }
    inline Vertex* cw()
      { return v_->cw(); }
    inline Vertex* ccw()
      { return v_->ccw(); }
    inline Vertex* neighbor( int rotation )
      { return v_->neighbor(rotation); }
    inline Vertex* advance( int rotation )
      { return v_ = v_->neighbor(rotation); }
    inline Vertex* setV( Vertex* v )
      { v_ = v; resize(); return v_; }
    Vertex* insert( const Point& p );
    void remove();
    Polygon* split( Vertex* v );

    PointPlace inPolygon( const Point& p );

  private:
    Vertex* v_;
    int size_;
    void resize();

};

}
}

#endif
