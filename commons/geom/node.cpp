#include "node.h"

namespace meteo {
namespace geom {

Node* Node::insert( Node* b )
{
  Node* c = next_;
  b->next_ = c;
  b->prev_ = this;
  next_ = b;
  c->prev_ = b;
  return b;
}

Node* Node::remove()
{
  prev_->next_ = next_;
  next_->prev_ = prev_;
  next_ = prev_ = this;
  return this;
}

void Node::splice( Node* b )
{
  Node* a = this;
  Node* an = a->next_;
  Node* bn = b->next_;
  a->next_ = bn;
  b->next_ = an;
  an->prev_ = b;
  bn->prev_ = a;
}

Vertex* Vertex::split( Vertex* v )
{
  Vertex* vp = v->ccw()->insert( new Vertex( v->point() ) );
  insert( new Vertex( point() ) );
  splice(vp);
  return vp;
}

}
}
