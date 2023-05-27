#include "windpath.h"

#include "wind.h"

#include <qpolygon.h>
#include <qpainter.h>

#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {

WindPath::WindPath(
    Wind* w,
    int d,
    int f,
    control::QualityControl ddqual,
    control::QualityControl ffqual,
    int rad )
  : dd_(d),
  ff_(f),
  dqual_(ddqual),
  fqual_(ffqual),
  radius_(rad),
  wind_(w),  
  dfrect_("DF"),
  lenft_(40.0)
{
}

WindPath::WindPath( Wind* w )
  : dd_(0),
  ff_(0),
  dqual_(control::NO_OBSERVE),
  fqual_(control::NO_OBSERVE),
  radius_(0),
  wind_(w),  
  dfrect_("DF"),
  lenft_(40.0)
{
}

WindPath::~WindPath()
{
}

QRect WindPath::boundingRect() const
{
  if ( false == cached_rect_.isEmpty() ) {
    return cached_rect_;
  }

  if ( false == wind_->ddok() ) {
    if ( false == wind_->draw_bad_dd() ) {
      return QRect();
    }
    buildDF();
    QImage testpix( QSize(1,1), QImage::Format_ARGB32 );
    QPainter testpntr(&testpix);
    testpntr.setPen( wind_->qpen() );
    testpntr.setFont( wind_->qfont() );
    testpntr.setBrush( wind_->qbrush() );
    QRect r = testpntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, dfrect_ );
    QPoint pnt = QPoint( r.left(), pos_.y() - radius_ - r.height() - 2 );
//    debug_log << "pnt =" << pnt << "pos =" << pos_ << "r =" << r;
    r.moveTo(pnt);
    r.setLeft( r.left() - 1 );
    r.setRight( r.right() + 1 );
    r.setBottom( r.bottom() + 1 );
    cached_rect_ = r;
    return r;
  }

  if ( true == line_.isEmpty() && true == poly_.isEmpty() && true == circle_.isEmpty() ) {
    buildPrimitivies();
  }
  for ( int i = 0, sz = line_.size(); i < sz; ++i ) {
    QRect rf = QRectF( line_[i].p1(), line_[i].p2() ).normalized().toRect();
    rf.setTopLeft( rf.topLeft()-QPoint(1,1) );
    rf.setBottomRight(rf.bottomRight()+QPoint(1,1));
    cached_rect_ = cached_rect_.united(rf);
  }
  for ( int i = 0, sz = poly_.size(); i < sz; ++i ) {
    cached_rect_ = cached_rect_.united( poly_[i].boundingRect().toRect() );
  }
  cached_rect_ = cached_rect_.united( circle_.boundingRect().toRect() );

  return cached_rect_;
}

void WindPath::clearPrimitivies() const
{
  poly_.clear();
  line_.clear();
  circle_ = QPainterPath();
  cached_rect_ = QRect();  
}

void WindPath::buildPrimitivies() const
{
  if ( false == line_.isEmpty() || false == poly_.isEmpty() || false == circle_.isEmpty() ) {
    return;
  }

  if ( false == wind_->ddok() && false == wind_->ffok() ) {
    if ( false == wind_->draw_bad_dd() ) {
      return;
    }
    buildDF();
    return;
  }
  else if ( false == wind_->ddok() ) {
    if ( false == wind_->draw_bad_dd() ) {
      return;
    }
    buildDF();
    return;
  }

//  if ( 9999 == dd_ ) {
//    return;
//  }

  const float kWscale = 1;
  float LenRs;
  float px, py, epx, epy;
 
  const float LenFt = lenft_*kWscale;
  const float LenSh = 6.5*kWscale;
  const float LenLn = 12.0*kWscale;
  const float LenCt = 11.0*kWscale;
  const float DivSh = 4.0*kWscale;
  const float DivLn = 6.0*kWscale;
 
  int dd = 180 - dd_;
  if ( ( 990 == dd_ ) || ( 0 == dd_ && 0 < ff_ ) ) {
    dd = 180 - 270;
  }
 
  px = pos_.x();
  py = pos_.y();
  float angle = dd;

  if ( 0 == dd_ && 0 == ff_ ) {
    addCircle( pos_, radius_ + 3 );
    return;
  }

  float radangle = DEG2RAD*angle;
  float sinradangle = ::sin(radangle);
  float cosradangle = ::cos(radangle);

  px += radius_*sinradangle;
  py += radius_*cosradangle;

  addLine( angle, LenFt, px, py );
  QPolygonF poly;

  float angl60 = angle - 60.0;
  float angl90 = angle - 90.0;

  if ( ( 990 == dd_ ) || ( 0 == dd_ && 0 < ff_ ) ) {
    poly.clear();
    epx = px + LenFt/2.0*sinradangle;
    epy = py + LenFt/2.0*cosradangle;
    addLine( &poly, angle - 45.0, LenSh, epx, epy);
    addLine( &poly, angle + 45.0, LenSh, epx, epy);
    addLine( &poly, angle + 135.0, LenSh, epx, epy);
    addLine( &poly, angle + 225.0, LenSh, epx, epy);
    poly_.append(poly);
  }
  if ( false == wind_->ffok() ) {
    poly.clear();
    epx = px + LenFt*sinradangle;
    epy = py + LenFt*cosradangle;
    addLine( &poly, angle - 45.0, LenSh, epx, epy);
    addLine( &poly, angle + 45.0, LenSh, epx, epy);
    addLine( &poly, angle + 135.0, LenSh, epx, epy);
    addLine( &poly, angle + 225.0, LenSh, epx, epy);
    poly_.append(poly);
//    pntr->drawPolygon(poly);
    return;
  }
 
  int nt = (ff_+1)/25;
  for ( int i = 0; i < nt; ++i ) {
     poly.clear();
     epx = px + float(LenFt-i*DivLn) * sin(radangle);
     epy = py + float(LenFt-i*DivLn) * cos(radangle);
     addLine( &poly, angl90, LenCt, epx, epy );
     epx = px + float(LenFt - (i+1)*DivLn) * sin(radangle);
     epy = py + float(LenFt - (i+1)*DivLn) * cos(radangle);
     poly.append( QPoint( epx, epy ) );
     poly.append( poly[0] );
     poly_.append(poly);
//     pntr->drawPolygon(poly);
  }
  
  int nl = ( ff_ - nt*25)/5;
  LenRs = LenFt;
  if ( 0 != nt ) {
    LenRs = LenFt - nt*DivLn;
    if ( 0 != nl ) {
      LenRs -= DivSh;
    }
  }
 
  for( int i = 0; i < nl; ++i ) {
    epx = px + float(LenRs - i*DivSh)*sin(radangle);
    epy = py + float(LenRs - i*DivSh)*cos(radangle);
    addLine( angl60, LenLn, epx, epy );
  }
  int m = ff_ - nt*25 - nl*5;
  if ( 1 < m ) {
    if ( 0 == nl && 0 != nt ) {
      nl += 1;
    }
    epx = px + float(LenRs - (nl)*DivSh) * sin(radangle);
    epy = py + float(LenRs - (nl)*DivSh) * cos(radangle);
    if( 4 == m ) {
      addLine( angl60, LenLn,epx,epy);
    }
    else {
      addLine( angl60, LenSh,epx,epy);
    }
  }
}

void WindPath::buildDF() const
{
  if ( false == wind_->ddok() && true == wind_->ffok() ) {
    dfrect_ = QString("D%1").arg(ff_);
  }
  else {
    dfrect_ = "DF";
  }
}

void WindPath::drawDF( QPainter* p ) const
{
  QRect r = boundingRect();
  p->save();
  p->setRenderHint( QPainter::HighQualityAntialiasing, true );
  p->setRenderHint( QPainter::TextAntialiasing, true );

  QFont fnt = wind_->qfont();
  QBrush br = wind_->qbrush();
//  QPen pn = wind_->qpen();
//
//  p->setPen(pn);
  p->setFont(fnt);
//  p->setBrush(br);

  p->drawText( r, Qt::AlignHCenter | Qt::AlignBottom, dfrect_ );
  br.setStyle( Qt::NoBrush );
  p->setBrush(br);
//  p->setPen(pn);
  p->drawRect(r);
  p->restore();
}

void WindPath::addLine( float angle, float len, float px, float py ) const
{
  QLineF line;
  bool xstep = false;
  if ( 0.0 == px ) {
    px += 1.0;
    xstep = true;
  }
  bool ystep = false;
  if ( 0.0 == py ) {
    py += 1.0;
    ystep = true;
  }
  line.setP1(QPointF(px,py));
  line.setLength(len);
  line.setAngle(angle-90.);
  QPointF p1 = line.p1();
  QPointF p2 = line.p2();
  if ( true == xstep ) {
    p1.setX(p1.x()-1.0);
    p2.setX(p2.x()-1.0);
  }
  if ( true == ystep ) {
    p1.setY(p1.y()-1.0);
    p2.setY(p2.y()-1.0);
  }
  line_.append( QLineF(p1, p2) );
//  p->drawLine(p1,p2);
}

void WindPath::addLine( QPolygonF* p, float angle, float len, float px, float py ) const
{
  QLineF line;
  line.setP1(QPointF(px,py));
  line.setLength(len);
  line.setAngle(angle-90.);
  p->append(line.p1().toPoint());
  p->append(line.p2().toPoint());
}
    
void WindPath::addCircle( const QPoint& pnt, int rad ) const
{
  circle_.addEllipse( pnt, rad, rad );
}

void WindPath::draw( QPainter* pntr ) const
{
  if ( true == cached_rect_.isEmpty() ) {
    buildPrimitivies();
    if ( true == cached_rect_.isEmpty() ) {
      return;
    }
  }
  if ( false == wind_->ddok() ) {
    if ( false == wind_->draw_bad_dd() ) {
      return;
    }
    drawDF(pntr);
    return;
  }
  for ( int i = 0, sz = line_.size(); i < sz; ++i ) {
    pntr->drawLine( line_[i] );
  }
  for ( int i = 0, sz = poly_.size(); i < sz; ++i ) {
    pntr->drawPolygon( poly_[i] );
  }
  pntr->save();
  QPen pen = pntr->pen();
  pen.setStyle(Qt::DashLine);
  pntr->setPen(pen);
  QBrush br = pntr->brush();
  br.setStyle( Qt::NoBrush );
  pntr->setBrush(br);
  pntr->drawPath(circle_);
  pntr->restore();
}

QSize WindPath::size() const
{
  return QSize( 40+40, 40+40 );
}

void WindPath::setLenft( float l )
{
  lenft_ = l;
  clearPrimitivies();
}

}
}
