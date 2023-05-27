#ifndef COMMONS_GEOM_LINE_H
#define COMMONS_GEOM_LINE_H

#include "edge.h"

#include <qpoint.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>

#include <cmath>

namespace meteo {
namespace geom {

class Line
{
  enum AlfaFlag {
    Alfa_0,   // угловой, коэффициент не возможно вычислить, т.к. угол равен 0
    Alfa_ok
  };

public:
  enum AlphaType {
    Alpha_DEG,
    Alpha_RAD
  };


public:
  Line( const QPointF& p1, const QPointF& p2 ) :
    _x1( p1.x() ),
    _y1( p1.y() ),
    _x2( p2.x() ),
    _y2( p2.y() ),
    _a( 0 )
  {
    calcAlpha();
  }

  Line( float x1, float y1, float x2, float y2 ) :
    _x1(x1),
    _y1(y1),
    _x2(x2),
    _y2(y2),
    _a( 0 )
  {
    calcAlpha();
  }

  // a - угол наклона к оси OX в градусах
  Line( const QPointF& p, float a ) :
    _x1( p.x() ),
    _y1( p.y() ),
    _a( 0 )
  {
    setAlpha( a );
    updatePoint();
  }

  // a - угол наклона к оси OX в градусах
  Line( float x, float y, float a ) :
    _x1( x ),
    _y1( y ),
    _a( 0 )
  {
    setAlpha( a );
    updatePoint();
  }

  float x( float y ) const {
    return ((y-_y1)*(_x2-_x1) / (_y2-_y1) + _x1);
  }

  float y( float x ) const {
    return ((x-_x1)*(_y2-_y1) / (_x2-_x1) + _y1);
  }

  float alpha( AlphaType t = Alpha_DEG ) const {
    return (Alpha_DEG == t) ? _a : _a*DEG2RAD;
  }

  // поворачивает прямую относительно (x1,y1) на угол a (в град.)
  Line& rotate( float a ){
    setAlpha(_a + a);
    updatePoint();
    return *this;
  }

  QPointF crossPoint( const Line& l ) const
  {
    Edge e1( Point(_x1,_y1), Point(_x2,_y2) );
    Edge e2( Point(l.basePoint().x(),l.basePoint().y()),
              Point(l.basePoint().x()+1,l.y(l.basePoint().x()+1))
              );
    return getCrossPoint( e1, e2 );
  }

  QPointF basePoint() const {
    return QPointF( _x1, _y1 );
  }

  static QPointF point( const QPointF& begP, const QPointF& endP, float L ) {
    if( 0.f == L ){
      return begP;
    }

    float dx = endP.x()-begP.x();
    float dy = endP.y()-begP.y();

    float D = std::sqrt( dx*dx + dy*dy );

    float x3 = begP.x() + L*dx/D;
    float y3 = begP.y() + L*dy/D;

    return QPointF( x3, y3 );
  }

private:
  QPointF getCrossPoint( const Edge& e, const Edge& f ) const
  {
    float t;
    Point iri_point;
    if (e.intersect(f,&t) == Edge::SKEW){
      iri_point = e.point(t);
    }
    else{
      return QPointF();
    }
    float x = iri_point.x();
    float y = iri_point.y();

    return QPointF(x, y);
  }

  void setAlpha( float a ){
    _a = (a<0) ? (180+a) : a;
  }

  void calcAlpha(){
    float a = _x1-_x2;
    float b = _y1-_y2;
    float c = std::sqrt( a*a + b*b );

    float alpha = std::atan( (b/c) / (a/c) )*RAD2DEG;
    setAlpha( alpha );
  }

  void updatePoint(){
    // округляем до 6 знаков после запятой, т.к. sin(0), sin(180), cos(90) и cos(270)
    // вычисляются с большим количеством знаков после запятой
    _x2 = _x1 + round(std::cos(_a*DEG2RAD)*1000000)/1000000;
    _y2 = _y1 + round(std::sin(_a*DEG2RAD)*1000000)/1000000;
  }

private:
  float _x1;
  float _y1;
  float _x2;
  float _y2;

  float _a;
  AlfaFlag  _flag;
};

}
}

#endif // TLINE_H
