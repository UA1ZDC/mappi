#ifndef COMMONS_GEOM_POINT_H
#define COMMONS_GEOM_POINT_H

#include <math.h>

#include <qpoint.h>

namespace meteo {
namespace geom {

class Edge;

class Point //!< Точка в прямоугольной системе координат
{
  public:
    enum PointPos      //!< Положение точки относительно отрезка
    {
      LEFT        = 0, //!< слева
      RIGHT       = 1, //!< справа
      BEYOND      = 2, //!< впереди
      BEHIND      = 3, //!< позади
      BETWEEN     = 4, //!< между
      ORIGIN      = 5, //!< начало
      DESTINATION = 6  //!< конец
    };

    inline Point( float _x = 0.0, float _y = 0.0 )
      : x_(_x), y_(_y) {}
    inline Point( const QPoint& pnt ) : x_(pnt.x()), y_(pnt.y()) {}
    inline Point( const QPointF& pnt ) : x_(pnt.x()), y_(pnt.y()) {}
    inline ~Point() {}

    inline QPointF qpointf() const { return QPointF(x_,y_); }

    inline const float& x() const { return x_; }
    inline const float& y() const { return y_; }

    inline void setX( float _x ) { x_ = _x; }
    inline void setY( float _y ) { y_ = _y; }
    inline void setXY( float _x, float _y ) { x_ = _x; y_ = _y; }

    friend inline Point operator+( const Point& p0, const Point& p1 )
      { return Point( p0.x() + p1.x(), p0.y() + p1.y() ); }
    friend inline Point operator-( const Point& p0, const Point& p1 )
      { return Point( p0.x() - p1.x(), p0.y() - p1.y() ); }
    inline const float& operator[]( int i ) const
      { return ( 0 == i ) ? x_ : y_; }
    friend inline Point operator*( const Point& p0,  float mul )
      { return Point( p0.x() * mul, p0.y() * mul ); }
    inline bool operator==( const Point& p ) const
      { return ( p.x() == x_ && p.y() == y_ ); }
    inline bool operator!=( const Point& p ) const
      { return ( p.x() != x_ || p.y() == y_ ); }
    inline bool operator<( const Point& p ) const
      { return ( ( x_ < p.x() ) || ( ( x_ == p.x() ) && ( y_ < p.y() ) ) ); }
    inline bool operator>( const Point& p ) const
      { return ( ( x_ > p.x() ) || ( ( x_ == p.x() ) && ( y_ > p.y() ) ) ); }

    PointPos classify( const Point& p0, const Point& p1 ) const ;
    PointPos classify( const Edge& e ) const ;

    float polarAngle() const ;

    inline float length() const
      { return ::sqrt( x_ * x_ + y_ * y_ ); }
    float distance( const Edge& e, QPointF* p = 0 ) const ;


  private:
    float x_;
    float y_;

};
}
}

#endif
