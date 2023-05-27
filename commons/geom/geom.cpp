#include "geom.h"

#include <math.h>

#include <qpair.h>
#include <qtransform.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace geom {


int distance( const QPoint& pnt, const QLine& line, QPoint* cross )
{
  QPoint res;
  QPair< float, QPoint > answer( 100000000, QPoint() );
  const QPoint& pnt1 = line.p1();
  const QPoint& pnt2 = line.p2();
  if ( pnt1 == pnt2 ) {
    QPoint delta = pnt1 - pnt;
    int dist = ::sqrt( delta.x()* delta.x() + delta.y()*delta.y() );
    if ( 0 != cross ) {
      *cross = pnt1;
    }
    return dist;
  }
  QTransform tr;
  tr.translate(-pnt1.x(),-pnt1.y());
  float angle = 0.0;
  if ( pnt1.y() != pnt2.y() ) {
    QPoint delta = pnt2 - pnt1;
    angle = ::atan2( delta.y(), delta.x() )*RAD2DEG;
    tr.translate(pnt1.x(), pnt1.y());
    tr.rotate(-angle);
    tr.translate(-pnt1.x(), -pnt1.y());
  }
  QPoint pnt1new = tr.map(pnt1);
  QPoint pnt2new = tr.map(pnt2);
  QPoint pntnew = tr.map(pnt);
  int dist = 0;
  if ( (pnt1new.x() <= pntnew.x() && pnt2new.x() >= pntnew.x())
       ||  (pnt1new.x() >= pntnew.x() && pnt2new.x() <= pntnew.x()) )
  {
    dist = ::abs( pntnew.y() );
    res = QPoint( pntnew.x(), 0 );
  }
  else {
    if ( pnt1new.x() > pnt2new.x() ) {
      qSwap(pnt1new,pnt2new);
    }
    QPointF delta;
    if ( pntnew.x() < pnt1new.x() ) {
      delta = pnt1new - pntnew;
      res = pnt1new;
    }
    else {
      delta = pntnew - pnt2new;
      res = pnt2new;
    }
    dist = ::sqrt( delta.x()*delta.x() + delta.y()*delta.y() );
  }
  if ( dist < answer.first ) {
    answer.first = dist;
    if ( 0 != cross ) {
      *cross = tr.inverted().map(res);
    }
  }
  return answer.first;
}

int distance( const QPoint& pnt, const QPolygon& poly, QPoint* cross )
{
  if ( 0 == poly.size() ) {
    error_log << QObject::tr("Пустой полигон. Не могу посчитать расстояние");
    return kErrorCalcDistanceValue;
  }
  if ( 1 == poly.size() ) {
    QPoint delta = poly[0] - pnt;
    int dist = ::sqrt( delta.x()* delta.x() + delta.y()*delta.y() );
    if ( 0 != cross ) {
      *cross = poly[0];
    }
    return dist;
  }
  int sign = 1;
  if ( poly.first() == poly.last() && true == poly.containsPoint( pnt, Qt::OddEvenFill ) ) {
    sign = -1;
  }
  QPoint res;
  QPair< float, QPoint > answer( 100000000, QPoint() );
  for ( int i = 1, sz = poly.size(); i < sz; ++i ) {
    const QPoint& pnt1 = poly[i-1];
    const QPoint& pnt2 = poly[i];
    if ( pnt1 == pnt2 ) {
      continue;
    }
    QTransform tr;
    tr.translate(-pnt1.x(),-pnt1.y());
    float angle = 0.0;
    if ( pnt1.y() != pnt2.y() ) {
      QPoint delta = pnt2 - pnt1;
      angle = ::atan2( delta.y(), delta.x() )*RAD2DEG;
      tr.translate(pnt1.x(), pnt1.y());
      tr.rotate(-angle);
      tr.translate(-pnt1.x(), -pnt1.y());
    }
    QPoint pnt1new = tr.map(pnt1);
    QPoint pnt2new = tr.map(pnt2);
    QPoint pntnew = tr.map(pnt);
    int dist = 0;
    if ( (pnt1new.x() <= pntnew.x() && pnt2new.x() >= pntnew.x())
        ||  (pnt1new.x() >= pntnew.x() && pnt2new.x() <= pntnew.x()) )
    {
      dist = ::abs( pntnew.y() );
      res = QPoint( pntnew.x(), 0 );
    }
    else {
      if ( pnt1new.x() > pnt2new.x() ) {
        qSwap(pnt1new,pnt2new);
      }
      QPointF delta;
      if ( pntnew.x() < pnt1new.x() ) {
        delta = pnt1new - pntnew;
        res = pnt1new;
      }
      else {
        delta = pntnew - pnt2new;
        res = pnt2new;
      }
      dist = ::sqrt( delta.x()*delta.x() + delta.y()*delta.y() );
    }
    if ( dist < answer.first ) {
      answer.first = dist;
      if ( 0 != cross ) {
        *cross = tr.inverted().map(res);
      }
    }
  }
  return sign*answer.first;
}

int distance( const QPoint& pnt, const QRect& rect, QPoint* res )
{
  return distance( pnt, QPolygon(rect, true), res );
}

bool crossPoint( const QPoint& x2, const QLine& l2, QPoint* pnt = 0 )
{
  QPoint locpoint;
  QPoint* result = &locpoint;
  if ( 0 != pnt ) {
    result = pnt;
  }
  QLine l1( QPoint(0,0), x2 );
  if ( l1.p1() == l2.p1() || l1.p1() == l2.p2() ) {
    *result = l1.p1();
    return true;
  }
  if ( l1.p2() == l2.p1() || l1.p2() == l2.p2() ) {
    *result = l1.p2();
    return true;
  }
  const QPoint& p1l1 = l1.p1();
  const QPoint& p2l1 = l1.p2();
  const QPoint& p1l2 = l2.p1();
  const QPoint& p2l2 = l2.p2();

  if ( 0 != p1l1.y() && 0 != p2l1.y() ) {
    warning_log
      << QObject::tr("Непредвиденная ситуация при попытке определить точку пересечения двух отрезков [%1:%2-%3:%4] и [%5:%6-%7:%8]")
      .arg(l1.p1().x())
      .arg(l1.p1().y())
      .arg(l1.p2().x())
      .arg(l1.p2().y())
      .arg(l2.p1().x())
      .arg(l2.p1().y())
      .arg(l2.p2().x())
      .arg(l2.p2().y());
  }

  if ( p1l2.y() == p2l2.y() ) {
    //линии параллельны
    return false;
  }

  if ( ( 0 < p1l2.y() && 0 < p2l2.y() )
    || ( 0 > p1l2.y() && 0 > p2l2.y() ) ) {
    return false;
  }

  if ( ( p1l1.x() > p1l2.x() && p1l1.x() > p2l2.x() )
    || ( p2l1.x() < p1l2.x() && p2l1.x() < p2l2.x() ) ) {
    return false;
  }

  if ( 0 == p1l2.y() ) {
    if ( p1l1.x() <= p1l2.x() && p2l1.x() >= p1l2.x() ) {
      *result = p1l2;
      return true;
    }
    return false;
  }
  if ( 0 == p2l2.y() ) {
    if ( p1l1.x() <= p2l2.x() && p2l1.x() >= p2l2.x() ) {
      *result = p2l2;
      return true;
    }
    return false;
  }

  float dist1 = ::fabs(p1l2.y());
  float dist2 = ::fabs(p2l2.y());
  float x = float( p2l2.x() - p1l2.x() )/( 1.0 + dist1/dist2 );
  x = p2l2.x() - x;
  if ( x < p1l1.x() || x > p2l1.x() ) {
    return false;
  }
  *result = QPoint( x, 0 );
  return true;
}

bool crossPoint( const QLine& l1, const QLine& l2, QPoint* pnt )
{
  QPoint locpoint;
  QPoint* result = &locpoint;
  if ( 0 != pnt ) {
    result = pnt;
  }
  if ( l1.p1() == l2.p1() || l1.p1() == l2.p2() ) {
    *result = l1.p1();
    return true;
  }
  if ( l1.p2() == l2.p1() || l1.p2() == l2.p2() ) {
    *result = l1.p2();
    return true;
  }
  const QPoint& p1l1 = l1.p1();
  const QPoint& p2l1 = l1.p2();
  const QPoint& p1l2 = l2.p1();
  const QPoint& p2l2 = l2.p2();

  if ( 0 == p1l1.x() && 0 == p1l1.y() && p1l1.y() == p2l1.y() && p1l1.x() <= p2l1.x() ) {
    return crossPoint( p2l1, l2, pnt );
  }
  QTransform tr;
  tr.translate(-p1l1.x(),-p1l1.y());
  float angle = 0.0;
  angle = ::atan2( l1.dy(), l1.dx() )*RAD2DEG;
  tr.translate(p1l1.x(), p1l1.y());
  tr.rotate(-angle);
  tr.translate(-p1l1.x(), -p1l1.y());

  QPoint p2l1new = tr.map(p2l1);
  QPoint p1l2new = tr.map(p1l2);
  QPoint p2l2new = tr.map(p2l2);
  if ( false == crossPoint( p2l1new, QLine( p1l2new, p2l2new ), result ) ) {
    return false;
  }
  *result = tr.inverted().map(*result);
  return true;
}

QList<QPoint> crossPoint( const QLine& l1, const QPolygon& poly, QList<float>* anglist )
{
  QList<QPoint> list;
  QLine l1loc;
  const QPoint& p1 = l1.p1();
  const QPoint& p2 = l1.p2();
  if ( ( true == poly.containsPoint(p1, Qt::OddEvenFill ) && true == poly.containsPoint(p2, Qt::OddEvenFill) )
      || ( false == poly.containsPoint(p1, Qt::OddEvenFill) && false == poly.containsPoint(p2, Qt::OddEvenFill) ) ) {
    return list;
  }
  bool fl = false;
  QTransform tr;
  QTransform trinv;
  if ( 0 == p1.x() && 0 == p1.y() && p1.y() == p2.y() && p1.x() <= p2.x() ) {
    l1loc = l1;
  }
  else {
    tr.translate(-p1.x(),-p1.y());
    float angle = 0.0;
    angle = ::atan2( l1.dy(), l1.dx() )*RAD2DEG;
    tr.translate(p1.x(), p1.y());
    tr.rotate(-angle);
    tr.translate(-p1.x(), -p1.y());
    trinv = tr.inverted();
    l1loc = tr.map(l1);
    fl = true;
  }
  for ( int i = 1, sz = poly.size(); i < sz; ++i ) {
    int step = 1;
    while ( i < sz && poly[i-step] == poly[i] ) {
      step +=1;
      i+=1;
    }
    if ( i >= sz || poly[i-step] == poly[i] ) {
      continue;
    }

    QPoint cross;
    QLine l2( poly[i-step], poly[i] );
    QLine l2loc = l2;
    if ( true == fl ) {
      l2loc = tr.map(l2);
    }
    if ( true == crossPoint( l1loc.p2(), l2loc, &cross ) ) {
      if ( true == fl ) {
        cross = trinv.map(cross);
      }
      list.append(cross);
      if ( 0 != anglist ) {
        anglist->append( ::atan2( l2.dy(), l2.dx() ) );
      }
    }
  }
  return list;
}

QList<QPoint> crossPoint( const QPolygon& poly1, const QPolygon& poly2, QList<float>* ang1list, QList<float>* ang2list )
{
  QList<QPoint> list;
  for ( int i = 1, sz = poly1.size(); i < sz; ++i ) {
    int step = 1;
    while ( i < sz && poly1[i-step] == poly1[i] ) {
      step +=1;
      i+=1;
    }

    if ( i >= sz || poly1[i-step] == poly1[i] ) {
      continue;
    }

    QLine l1( poly1[i-step], poly1[i] );
    QList<QPoint> cross;
    if ( 0 != ang2list ) {
      QList<float> locanglist;
      cross = crossPoint(  l1, poly2, &locanglist );
      if ( 0 == cross.size() ) {
        continue;
      }
      if ( 0 != ang1list ) {
        float angle = ::atan2( l1.dy(), l1.dx() );
        for ( int j = 0, jsz = cross.size(); j < jsz; ++j ) {
          ang1list->append(angle);
          ang2list->append(locanglist[j]);
        }
      }
      else {
        ang2list->append(locanglist);
      }
    }
    else if ( 0 != ang1list ) {
      cross = crossPoint(  l1, poly2 );
      if ( 0 == cross.size() ) {
        continue;
      }
      float angle = ::atan2( l1.dy(), l1.dx() );
      for ( int j = 0, jsz = cross.size(); j < jsz; ++j ) {
        ang1list->append(angle);
      }
    }
    if ( 0 != cross.size() ) {
      list.append(cross);
    }
  }
  return list;
}

}
}
