#ifndef COMMONS_GEOM_NODE_H
#define COMMONS_GEOM_NODE_H

#include "point.h"

namespace meteo {
namespace geom {

class Node
{
  public:
   inline Node()
     : next_(this), prev_(this) {}
   inline virtual ~Node()
    {}
   inline Node* next() const
     { return next_; }
   inline Node* prev() const
     { return prev_; }
   Node* insert( Node* node );
   Node* remove();
   void splice( Node* node );

  protected:
   Node* next_;
   Node* prev_;
};

class Vertex : public Node, public Point
{
  public:
    enum {
      CLOCKWISE         = 0,
      SMACK_CLOCKWISE   = 1
    };
    inline Vertex (float x, float y)
      : Point( x, y )
      {}
    inline Vertex ( const Point& p)
      : Point(p) {}
    inline Vertex* cw() const
      { return (Vertex*)next_; }
    inline Vertex* ccw() const
      { return (Vertex*)prev_; }
    inline Vertex* neighbor (int rotation) const
      { return ( CLOCKWISE == rotation ) ? cw() : ccw() ; }
    inline const Point& point () const
      { return *((const Point*)this); }
    inline Vertex* insert (Vertex* v)
      { return (Vertex*)( Node::insert(v) ); }
    inline Vertex* remove ()
      { return (Vertex*)(Node::remove()); }
    inline void splice (Vertex* b)
      { Node::splice(b); }
    Vertex *split( Vertex* v );
};

}
}

#endif
