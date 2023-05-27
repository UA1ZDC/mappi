#include "simple.h"

#include <qpoint.h>
#include <qpolygon.h>
#include <qpainter.h>
#include <qalgorithms.h>
#include <qdebug.h>

#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>

#include "../object.h"

namespace meteo {
namespace map {

SimpleArrow::SimpleArrow( ArrowPlace place, QPolygon* gv )
  : Arrow( place, gv ),
  width_(0),
  length_(0)
{
}

SimpleArrow::~SimpleArrow()
{
}

void SimpleArrow::render( QPainter* painter, const meteo::Property& property )
{

  if ( true == property.double_line() ) {
    width_ = 5 + 3*property.pen().width()/2;
  }
  else {
    width_ = 5 + property.pen().width()/2;
  }
  length_ = MnMath::ftoi_norm( (10 + property.pen().width())*0.87 );
  QPoint start;
  QPoint end;
  if ( false == closurePoints( &start, &end ) ) {
    error_log << QObject::tr("Не удалось получить экранные координаты стрелки");
    return;
  }

  QPolygon poly(4);
  QPoint dist = end - start;
  float angle = ::atan2( dist.x(), dist.y() ) + M_PI*0.5;
  int sx = MnMath::ftoi_norm( ::sin(angle)*width_);
  int cx = MnMath::ftoi_norm( ::cos(angle)*width_ );
  if ( kBeginArrow == place_ ) {
    poly[0] = start;
    poly[1] = end + QPoint( sx, cx );
    poly[2] = end - QPoint( sx, cx );
    poly[3] = start;
  }
  else if ( kEndArrow == place_ ) {
    poly[0] = end;
    poly[1] = start - QPoint( sx, cx );
    poly[2] = start + QPoint( sx, cx );
    poly[3] = end;
  }
  else {
    return;
  }
  painter->save();
  QPen pen = pen2qpen(property.pen());
  pen.setJoinStyle( Qt::MiterJoin );
  pen.setStyle( Qt::SolidLine );
  painter->setPen(pen);
  QBrush brush = brush2qbrush(property.brush());
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(pen.color());
  painter->setBrush(brush);
  painter->drawPolygon(poly);
  painter->restore();
}

bool SimpleArrow::closurePoints( QPoint* begin, QPoint* end )
{
  if ( 2 > vector_->size() ) {
    return false;
  }
  int step = 1;
  int startnum = 1;
  int endnum = vector_->size();
  int beginPointNumber = -1;
  int endPointNumber = -1;
  if ( kBeginArrow == place_ ) {
    *begin = vector_->first();
    beginPointNumber = 0;
  }
  else if ( kEndArrow == place_ ) {
    *begin = vector_->last();
    step = -1;
    startnum = vector_->size() - 2;
    endnum = -1;
    beginPointNumber = vector_->size() - 1;
  }
  else {
    error_log << QObject::tr("Неверно указано полжение стрелки");
    return false;
  }
  int r = length_*length_;
  bool found = false;
  QPoint pnt1;
  QPoint pnt2;
  for ( int i = startnum; i != endnum;  i += step ) {
    QPoint screen = vector_->at(i);
    int dx = screen.x() - begin->x();
    int dy = screen.y() - begin->y();
    int rr = dx*dx + dy*dy;
    if ( rr > r ) {
      found = true;
      pnt1 = vector_->at(i-step);
      pnt2 = screen;
      endPointNumber = i;
      break;
    }
  }
  if ( false == found ) {
    pnt1 = vector_->at(endnum-step-step);
    pnt2 = vector_->at(endnum-step);
  }
  QPoint dist = pnt1 - pnt2;
  while ( 2 <= ::abs( dist.x() ) || 2 <= ::abs( dist.y() ) ) {
    int x = MnMath::ftoi_norm( 0.5*( pnt1.x() + pnt2.x() ) );
    int y = MnMath::ftoi_norm( 0.5*( pnt1.y() + pnt2.y() ) );
    int dx = x - begin->x();
    int dy = y - begin->y();
    int rr = dx*dx + dy*dy;
    if ( r <= rr ) {
      pnt2.setX(x);
      pnt2.setY(y);
    }
    else {
      pnt1.setX(x);
      pnt1.setY(y);
    }
    dist = pnt1 - pnt2;
  }
  *end = pnt1;
  if ( kEndArrow == place_ ) {
    qSwap( *begin, *end );
    qSwap( beginPointNumber, endPointNumber );
  }

  int removed_count = endPointNumber - (beginPointNumber + 1);
  if ( kBeginArrow == place_ ) {
    while ( 0 < removed_count ) {
      vector_->pop_front();
      --removed_count;
    }
    vector_->first() = *end;
  }
  else if ( kEndArrow == place_ ) {
    while ( 0 < removed_count ) {
      vector_->pop_back();
      --removed_count;
    }
    vector_->last() = *begin;
  }

  return true;
}

}
}
