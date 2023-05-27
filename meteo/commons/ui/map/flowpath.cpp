#include "flowpath.h"

#include "flow.h"

#include <qpolygon.h>
#include <qpainter.h>

#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {

FlowPath::FlowPath(
    Flow* fl,
    int d,
    int f,
    control::QualityControl dnqual,
    control::QualityControl cnqual,
    int rad )
  : dn_(d),
  cn_(f),
  dqual_(dnqual),
  fqual_(cnqual),
  radius_(rad),
  flow_(fl),  
  dfrect_("")
{
}

FlowPath::FlowPath( Flow* fl )
  : dn_(0),
  cn_(0),
  dqual_(control::NO_OBSERVE),
  fqual_(control::NO_OBSERVE),
  radius_(0),
  flow_(fl),  
  dfrect_("")
{
}

FlowPath::~FlowPath()
{
}

QRect FlowPath::boundingRect() const
{
  if ( false == cached_rect_.isEmpty() ) {
    return cached_rect_;
  }

  if ( false == flow_->dnok() ) {
    return QRect();
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
  cached_rect_ = cached_rect_.united( dfrect().toRect() );
  cached_rect_ = cached_rect_.united( circle_.boundingRect().toRect() );

  return cached_rect_;
}

QRectF FlowPath::dfrect() const
{
  QRectF r;
  if ( true == dfrect_.isEmpty() ) {
    return r;
  }
  r = QFontMetrics( flow_->qfont() ).boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, dfrect_ );
  r.setTop( r.top()+1 );
  r.setBottom( r.bottom()-1 );
  r.moveCenter(finish_);

  int dn = flow_->dn();
  while ( 0 > dn ) {
    dn += 360;
  }
  while ( 360 < dn ) {
    dn -= 360;
  }
  if ( ( 337 < dn && 360 >= dn ) || ( 0 <= dn && 23 >= dn ) ) {
    r.moveBottom( r.center().y() );
  }
  else if ( ( 23 < dn && 67 >= dn ) ) {
    r.moveBottomLeft( r.center() );
  }
  else if ( ( 67 < dn && 112 >= dn ) ) {
    r.moveLeft( r.center().x() );
  }
  else if ( ( 112 < dn && 157 >= dn ) ) {
    r.moveTopLeft( r.center() );
  }
  else if ( ( 157 < dn && 202 >= dn ) ) {
    r.moveTop( r.center().y() );
  }
  else if ( ( 202 < dn && 247 >= dn ) ) {
    r.moveTopRight( r.center() );
  }
  else if ( ( 247 < dn && 292 >= dn ) ) {
    r.moveRight( r.center().x() );
  }
  else if ( ( 292 < dn && 337 >= dn ) ) {
    r.moveBottomRight( r.center() );
  }

  return r;
}

void FlowPath::clearPrimitivies() const
{
  poly_.clear();
  line_.clear();
  circle_ = QPainterPath();
  
}

void FlowPath::buildPrimitivies() const
{
  if ( false == line_.isEmpty() || false == poly_.isEmpty() || false == circle_.isEmpty() ) {
    return;
  }

  if ( false == flow_->dnok() && false == flow_->cnok() ) {
    return;
  }
  else if ( false == flow_->dnok() ) {
    return;
  }

  const float kWscale = 1;
  float px, py;
 
  const float LenFt = 40.0*kWscale;
 
  int dn = 180 - dn_;
  if ( ( 990 == dn_ ) || ( 0 == cn_ ) ) {
    return;
  }
 
  px = pos_.x();
  py = pos_.y();
  float angle = dn;

  if ( 0 == dn_ && 0 == cn_ ) {
    return;
  }

  float radangle = DEG2RAD*angle;
  float sinradangle = ::sin(radangle);
  float cosradangle = ::cos(radangle);

  finish_ = QPointF( px + (LenFt + radius_)*sinradangle, py + (LenFt + radius_)*cosradangle );

  px += radius_*sinradangle;
  py += radius_*cosradangle;

  addLine( angle, LenFt, px, py );
  QPolygonF poly;


  if ( false == flow_->cnok() ) {
    return;
  }
  px = pos_.x();
  py = pos_.y();
  px += (LenFt + radius_ - 10)*sinradangle;
  py += (LenFt + radius_ - 10)*cosradangle;
  addTriangle( angle, 6, 10, px, py );
  buildDF();
}

void FlowPath::buildDF() const
{
  if ( false == flow_->dnok() || false == flow_->cnok() ) {
    dfrect_ = QString();
  }
  else {
    dfrect_ = QString::number( flow_->cn() );
  }
}

void FlowPath::drawDF( QPainter* p ) const
{
  return;
  if ( true == dfrect_.isEmpty() ) {
    return;
  }
  QRectF r = dfrect();
  p->save();
  p->setRenderHint( QPainter::HighQualityAntialiasing, true );
  p->setRenderHint( QPainter::TextAntialiasing, true );

  QFont fnt = flow_->qfont();
  QBrush br = flow_->qbrush();
  QPen pn = flow_->qpen();

  p->setPen(pn);
  p->setFont(fnt);
  p->setBrush(br);

  p->save();
  p->drawText( r, Qt::AlignHCenter | Qt::AlignBottom, dfrect_ );
  br.setStyle( Qt::NoBrush );
  p->restore();
//  p->setBrush(br);
//  p->setPen(pn);
//  p->drawRect(r);
  p->restore();
}

void FlowPath::addLine( float angle, float len, float px, float py ) const
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

void FlowPath::addTriangle( float angle, float width, float len, float px, float py ) const
{
  float sinw = ::sin( DEG2RAD*angle );
  float cosw = ::cos( DEG2RAD*angle );
  QPointF p0( px + len*sinw, py + len*cosw );
  QPolygonF poly;
  poly.append(p0);
  angle -= 90;
  QLineF line;
  sinw = ::sin( DEG2RAD*angle );
  cosw = ::cos( DEG2RAD*angle );
  QPointF p1( px+width/2.0*sinw, py+width/2.0*cosw );
  QPointF p2( px-width/2.0*sinw, py-width/2.0*cosw );
  poly.append(p1);
  poly.append(p2);
  poly_.append(poly);
}

void FlowPath::addLine( QPolygonF* p, float angle, float len, float px, float py ) const
{
  QLineF line;
  line.setP1(QPointF(px,py));
  line.setLength(len);
  line.setAngle(angle-90.);
  p->append(line.p1().toPoint());
  p->append(line.p2().toPoint());
}
    
void FlowPath::addCircle( const QPoint& pnt, int rad ) const
{
  circle_.addEllipse( pnt, rad, rad );
}

void FlowPath::draw( QPainter* pntr ) const
{
  if ( true == cached_rect_.isEmpty() ) {
    return;
  }
  if ( false == flow_->dnok() ) {
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
  drawDF(pntr);
  pntr->restore();
}

QSize FlowPath::size() const
{
  return QSize( 40+40, 40+40 );
}

}
}
