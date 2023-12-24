#include "pos.h"

#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/satellitebase.h>
#include <sat-commons/satellite/satviewpoint.h>

namespace meteo {

static const double kDayLengthSecs = 86400.0;
static const double kDayLengthSecs_2 = 86400.0*0.5;
static const double kSiderialDaySecs = 86164.09;
static const double kSiderialDaySecs_2 = kSiderialDaySecs*0.5;

double earthDelta;

static const int delta = 0;

SatelliteBase* sat = nullptr;

POSproj::POSproj()
  : Projection()
{
  start_.setPoint(M_PI_2, -M_PI);
  end_.setPoint(-M_PI_2, M_PI);
  type_ = POLARORBSAT;
}

POSproj::~POSproj()
{
   delete sat_;
}

bool POSproj::countGridCorners(meteo::GeoPoint* gp ) const {
  if(nullptr == sat_){
    error_log << QObject::tr("Не указаны данные спутника");
    return false;
  }
  return sat_->countGridCorners( scan_angle_, gp, gp+1, gp+2, gp+3 );
}

void POSproj::setSatellite( const QDateTime& aos, const QDateTime& los, const MnSat::TLEParams& tle, double sa, int w, double lps )
{
  sat_ = new SatViewPoint (aos, los);
  sat_->setTLEParams(tle);
  Coords::EciPoint eci;
  if ( false == sat_->getPosition( aos, &eci ) ) {
    error_log << QObject::tr("Не удалось рассчиать координаты спутника %1 на время %2")
      .arg( sat_->name() )
      .arg( aos.toString(Qt::ISODate) );
    return;
}
  setSatellite(
      sat_->inclination(),
      sat_->meanMotion(),
      aos,
      los,
      eci,
      sa,
      w,
      lps );
}


void POSproj::setSatellite( const QDateTime& ts, const QDateTime& te, SatelliteBase* params, double sa, int w, double lps )
{
  sat = params;
  Coords::EciPoint eci;
  if ( false == params->getPosition( ts, &eci ) ) {
    error_log << QObject::tr("Не удалось рассчиать координаты спутника %1 на время %2")
      .arg( params->name() )
      .arg( ts.toString(Qt::ISODate) );
    return;
  }

//  debug_log << "INCL ORIG =" << params->inclination() << "TEMPO =" << incl;

  setSatellite( 
      params->inclination(),
      params->meanMotion(),
      ts,
      te,
      eci,
      sa,
      w,
      lps );
}

void POSproj::setSatellite( double i, double rn, const QDateTime& ts, const QDateTime& te, const Coords::EciPoint& satcoord, double sa, int w, double lps )
{
  i_ = i;
  rn_ = rn;
  ts_ = ts;
  te_ = te;


  dt_ = ts_.msecsTo(te_)*0.001;
  tka_ = kSiderialDaySecs/rn_;
  satcoord_ = satcoord;

  startpos_.setPoint( satcoord_.posToGeo().lat, satcoord_.posToGeo().lon );

  scan_angle_ = sa;
  width_ = w;
  lps_ = lps;

  double arc0_length =::sin( startpos_.lat() )/::sin(i_);
  if ( 1.0 < arc0_length ) {
    error_log << QObject::tr("sin(fi)>sin(i)! Необходимо поправить способ вычисления! ");
    arc0_length = 1.0;
  }
  arc0_length = ::asin( arc0_length ); // длина дуги от экватора до координаты начала приема
  double lamt = 0.0;
  if ( false == MnMath::isEqual(M_PI_2, ::fabs(i_) ) ) {
    //дельта по долготе при движении спутника от экватора до координаты начала приема (без учета вращения земли)
    lamt = ::fabs( ::tan( startpos_.lat() )/::tan( i_ ) );
    if ( 1.0 < lamt ) {
      error_log << QObject::tr("tan(fi)>tan(i)! Необходимо поправить способ вычисления! ");
      lamt = 1.0;
    }
    lamt = ::asin(lamt);
  }
  if ( 0.0 > satcoord.vel.z ) {
    arc0_length = M_PI - arc0_length;
    lamt = M_PI - lamt;
  }
  U_ = arc0_length;
  if ( M_PI_2 < i_ ) {
    lamt = -lamt;
  }
  t0_ = ts_.addSecs(-arc0_length/(M_PI*2.0)*tka_/**1000*/);
  int64_t delta_t_pi_2 = tka_/4.0*1000; // 
  t0_PI_2_ = t0_.addMSecs(delta_t_pi_2);
  t0_PI_ = t0_PI_2_.addMSecs(delta_t_pi_2);
  lam0_ = startpos_.lon() - lamt + M_PI*( t0_.msecsTo(ts_)*0.001 )/kSiderialDaySecs_2;

 // sat->lastParams( &l0, &u, &dt, &ii );
 // t0_ = ts_.addSecs(-dt);
 // Coords::GeoCoord crd;
 // sat->getPosition( t0_, &crd );
 // debug_log << "NEwDT0 =" << t0_.toString(Qt::ISODate);
 // U_ = u;
 // i_ = ii;
 // lam0_ = crd.lon;
 // debug_log << "NEWLAM0 =" << lam0_*RAD2DEG;
 // tka_ = dt*M_PI*2.0/U_;
 // debug_log << "u =" << U_*RAD2DEG << U_<< "dt =" << dt << "tka_ =" << tka_;

//  U_ = M_PI*2*t0_.msecsTo(ts_)*0.001/tka_; //длина дуги от момента прохождения приема до начала приема
  duseans_ = M_PI*2*ts_.msecsTo(te_)*0.001/tka_; //длина дуги от момента прохождения приема до окончания приема

  arc0_length = M_PI*2*t0_.msecsTo(te_)*0.001/tka_; //длина дуги от момента прохождения приема до окончания приема
  endpos_.setLat( ::asin( ::sin(arc0_length)*::sin(i_) ) );

  lamt = 0;
  if ( false == MnMath::isEqual(M_PI_2, ::fabs(i_) ) ) {
    //дельта по долготе при движении спутника от экватора до координаты конца приема (без учета вращения земли)
    lamt = ::fabs( ::tan( endpos_.lat() )/::tan(i_) );
    if ( 1.0 < lamt ) {
      error_log << QObject::tr("tan(fi)>tan(i)! Необходимо поправить способ вычисления! ");
      lamt = 1.0;
    }
    lamt = ::asin(lamt);
    if ( te_ > t0_PI_2_ ) {
      lamt = M_PI - lamt;
    }
  }
  if ( M_PI_2 < i_ ) {
    lamt = -lamt;
  }
  endpos_.setLon( lam0_ + lamt - M_PI*( t0_.msecsTo(te_)*0.001 )/kSiderialDaySecs_2 );
  QDateTime tmiddle = ts_.addMSecs( ts_.msecsTo(te_)*0.5 );
  if ( tmiddle < t0_PI_2_ ) {
    up_ = true;
  }
  else {
    up_ = false;
  }
  arc0_length = M_PI*2*(t0_.msecsTo(tmiddle)*0.001/tka_ );
  centerMap_.setLat( ::asin( ::sin(arc0_length)*::sin(i_) ) );

  lamt = 0;
  if ( false == MnMath::isEqual(M_PI_2, ::fabs(i_) ) ) {
    //дельта по долготе при движении спутника от экватора до координаты середины приема (без учета вращения земли)
    lamt = ::fabs( ::tan( centerMap_.lat() )/::tan(i_) );
    if ( 1.0 < lamt ) {
      error_log << QObject::tr("tan(fi)>tan(i)! Необходимо поправить способ вычисления! ");
      lamt = 1.0;
    }
    lamt = ::asin(lamt);
    if ( false == up_ ) {
      lamt = M_PI - lamt;
    }
  }
  if ( M_PI_2 < i_ ) {
    lamt = -lamt;
  }
  centerMap_.setLon( lam0_ + lamt - M_PI*( t0_.msecsTo(tmiddle)*0.001 )/kSiderialDaySecs_2 );
  cos_fi_s_ = ::cos( startpos_.lat() );
  cos_fi_e_ = ::cos( endpos_.lat() );
  sin_fi_s_ = ::sin( startpos_.lat() );
  sin_fi_e_ = ::sin( endpos_.lat() );
  sin_fi_c_ = ::sin( centerMap_.lat() );
  cos_fi_c_ = ::cos( centerMap_.lat() );
  sin_la_c_ = ::sin( centerMap_.lon() );
  cos_la_c_ = ::cos( centerMap_.lon() );
  CC_ = sin_fi_s_*sin_fi_e_ + cos_fi_s_*cos_fi_e_*::cos( startpos_.lon() - endpos_.lon() );
  DD_ = ::sqrt(1.0 - CC_*CC_);
  alpham_ = ::asin( satcoord_.pos.r/kEarthRadiusKM*::sin(scan_angle_) ) - scan_angle_;
  height_ = ts_.msecsTo(te_)*0.001*lps_;

  initRamka();
}

bool POSproj::F2X(const GeoVector& dPt, QVector<QPolygon>* apoint, bool isClosed ) const
{
  Q_UNUSED(isClosed);
  if ( dPt.isEmpty() ) {
    return true;
  }

  QPolygon scrPt;
  QPoint pnt;
  QPoint oldpnt;
  GeoPoint gp;

  for(const auto& gp : dPt)
  {
    if(false == F2X_one(gp, &pnt)) {
      continue;
    }
    if(pnt != oldpnt) {
      scrPt.append(pnt);
      oldpnt = pnt;
    }
  }

  if(scrPt.count() > 1)
  {
    if(false == F2X_one(dPt.last(), &pnt))
    {
      if(pnt != oldpnt) {
        scrPt.append(pnt);
      }
    }
  }

  apoint->append(scrPt);
  return scrPt.empty();
}

bool POSproj::F2X_one(const GeoPoint& geoCoord, QPoint* meterCoord ) const
{
  QPointF pnt(*meterCoord);
  if ( false == F2X_one( geoCoord, &pnt ) ) {
    return false;
  }
  *meterCoord = pnt.toPoint();
  return true;
}

bool POSproj::X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const
{
  return X2F_one( QPointF(meterCoord), geoCoord );
}

bool POSproj::F2X_one(const GeoPoint& geoCoord, QPointF* meterCoord ) const
{
  double arg = geoCoord.radianDistance(centerMap_);
  if ( 0.5 < ::fabs(arg) ) {
    return false;
  }

  double du = 0.0;
  if ( false == calcDuKA( geoCoord, &du ) ) {
    return false;
  }
  GeoPoint kapoint;
  if ( false == calcPointKA( du, &kapoint ) ) {
    return false;
  }

  double sa = 0.0;
  if ( false == calcScanAngleKA( kapoint, geoCoord, du, &sa ) ) {
    return false;
  }

  double dt = du*tka_/(M_PI*2);
  double x = width_*0.5 - width_*0.5*(sa/scan_angle_);
  double y = dt*lps_;
  if ( width_+1000 > x && -1000 < x && height_ + 1000 > y && -1000 < y ) {
    meterCoord->setX(x - width_*0.5);
    meterCoord->setY(y - height_*0.5);
  }
  else {
    return false;
  }

  return true;
}

bool POSproj::X2F_one(int num, GeoPoint* geoCoord ) const
{
  int i = num % width_;
  int j = num / width_;
 /* if(j >= height_){
    return false;
  }*/
  return X2F_one(QPointF(i-width_*0.5, j-height_*0.5), geoCoord);
}

double POSproj::bt(int num) const//угол обзора (сканирования) радиометра
{
  int i = num % width_;
  return -scan_angle_*(i-width_*0.5 )/(width_*0.5);
}

bool POSproj::zenithCos(int num, double *retval) const//угол обзора (сканирования) радиометра AVHRR
{
  int i = num % width_;
  double bt = -scan_angle_*(i-width_*0.5 )/(width_*0.5);
  if ( -0.2 > scan_angle_ - ::fabs(bt) ) {
    return false;
  }
  double h = ::sin(bt);
  double h1 = satcoord_.pos.r/kEarthRadiusKM;
  if(fabs(h*h1) >1.)
  {
    return false;
  }
  double arg = ::asin( h*h1 );
  *retval = -sin(arg);
  return true;
}

QDateTime POSproj::date(int num) const //дата время в точке картинки
{
  int j = num / width_;

  return ts_.addSecs(j*lps_);

}


bool POSproj::X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const
{
  double x = meterCoord.x() + width_*0.5;
  double y = meterCoord.y() + height_*0.5;

//  debug_log << "Y =" << y;

  double dt = y/lps_;
  double arc0_length = M_PI*2.0/tka_*( dt + t0_.msecsTo(ts_)*0.001 );
  double fi = ::asin( ::sin(arc0_length)*::sin(i_) );
  double lamt = ::fabs( ::asin( ::tan(fi)/::tan(i_) ) );
  if(qFuzzyIsNull(fi)){
    geoCoord->setLat( 0 );

    geoCoord->setLon( 0);

    return true;
  }

  if ( arc0_length > M_PI_2 ) {
    lamt = M_PI - lamt;
  }
  if ( M_PI_2 < i_ ) {
    lamt = -lamt;
  }
  double c_tkaz = tka_/kSiderialDaySecs;
  double la = lam0_ + lamt - arc0_length*c_tkaz; //Долгота подспутниковой точки в момент приема строки с точкой geoCoord
  double arg = ::sin(lamt)*::sin(i_)/::sin(fi);
  double azka = asin(arg);
  double bt = -scan_angle_*(x - width_*0.5 )/(width_*0.5);
  if ( -0.2 > scan_angle_ - ::fabs(bt) ) {
    return false;
  }
  arg = ::asin( ::sin(bt)*satcoord_.pos.r/kEarthRadiusKM );
  double lsc = ::fabs(arg) - ::fabs(bt);
  if ( 0.0 < bt ) {
    lsc = -lsc;
  }
  geoCoord->setLat( ::asin( ::cos(lsc)*::sin(fi) + ::sin(lsc)*::cos(fi)*::sin(azka) ) );
  arg = ( ::cos(lsc) - ::sin(fi)*::sin( geoCoord->lat() ) )/(::cos(fi)*::cos( geoCoord->lat() ) );
  if ( -1.0 > arg ) {
    arg = -1.0;
  }
  else if ( 1.0 < arg ) {
    arg = 1.0;
  }
  double lon = ::acos(arg);
  if ( arc0_length > M_PI_2 ) {
    if ( 0.0 > bt) {
      lon = -lon;
    }
  }
  else if ( 0.0 < bt ) {
    lon = -lon;
  }
  geoCoord->setLon( la - lon );
  return true;
}

Projection* POSproj::copy() const
{
  POSproj* p = new POSproj;
  p->setSatellite( i_, rn_, ts_, te_, satcoord_, scan_angle_, width_, lps_ );
  return p;
}

void POSproj::initRamka()
{
  QPolygonF poly;
  ramka_.clear();
  GeoPoint pnt;
  QPointF scr( (-width_+delta)*0.5, (-height_+delta)*0.5 );
  if ( true == X2F_one( scr, &pnt ) ) {
    ramka_.append(pnt);
  }
//  debug_log << "LEFT GeoPoint =" << pnt;
  poly << scr;
  scr = QPointF( (-width_+delta)*0.5, (height_-delta)*0.5 );
  if ( true == X2F_one( scr, &pnt ) ) {
    ramka_.append(pnt);
  }
  poly << scr;
  scr = QPointF( 0, (height_-delta)*0.5 );
  if ( true == X2F_one( scr, &pnt ) ) {
    ramka_.append(pnt);
  }
  poly << scr;
  scr = QPointF((width_-delta)*0.5, (height_-delta)*0.5 );
  if ( true == X2F_one( scr, &pnt ) ) {
    ramka_.append(pnt);
  }
  poly << scr;
  scr = QPointF( (width_-delta)*0.5, (-height_+delta)*0.5 );
  if ( true == X2F_one( scr, &pnt ) ) {
    ramka_.append(pnt);
  }
  poly << scr;
  scr = QPointF( 0, (-height_+delta)*0.5 );
  if ( true == X2F_one( scr, &pnt ) ) {
    ramka_.append(pnt);
  }
  poly << scr;
  QPolygon respoly;
  for ( int i = 0; i < 6; ++i ) {
    QPoint respnt;
    if ( true == F2X_one( ramka_[i], &respnt ) ) {
      respoly << respnt;
    }
  }
  ramka_.append( ramka_.first() );
}

bool POSproj::calcPointKA( double du, GeoPoint* kapoint ) const
{
  if ( -0.2 > du ) {
    return false;
  }
  double c_tkaz = tka_/kSiderialDaySecs;
  double fi = ::asin( ::sin(U_ + du)*::sin(i_) ); //широта подспутниковой точки в момент приема строки с точкой geoCoord
  double lamt = 0.0;
  if ( false == MnMath::isEqual( M_PI_2, i_ ) ) {
    lamt = ::fabs(::tan(fi)/::tan(i_));
    if ( 1.0 < lamt ) {
      error_log << QObject::tr("tan(fi)>tan(i)! Необходимо поправить способ вычисления! ");
      lamt = 1.0;
    }
    lamt = ::asin(lamt);
    if ( M_PI_2 < U_ + du ) {
      lamt = M_PI - lamt;
    }
    if ( i_ > M_PI_2 ) {
      lamt = -lamt;
    }
  }
  double la = lam0_ + lamt - (U_+du)*c_tkaz; //Долгота подспутниковой точки в момент приема строки с точкой geoCoord
  kapoint->setLat(fi);
  kapoint->setLon(la);

  return true;
}

bool POSproj::calcDuKA( const GeoPoint& gp, double* du ) const
{
  double sinfi = ::sin( gp.lat() );
  double cosfi = ::cos( gp.lat() );
//  double arg1 = geoCoord.lon() - startpos_.lon();
//  double arg2 = geoCoord.lon() - endpos_.lon();
//  double cosarg1 = ::cos(arg1);
//  double cosarg2 = ::cos(arg2);
//  double sinarg1 = ::sin(arg1);
//  double sinarg2 = ::sin(arg2);
  double a1 = sinfi*sin_fi_s_;
  double a2 = cosfi*cos_fi_s_;
  double b1 = sinfi*sin_fi_e_;
  double b2 = cosfi*cos_fi_e_;
//  double a3 = b1 - a1*CC_;
//  double a4 = b2*cosarg2 - a2*cosarg1*CC_;
//  double a5 = b2*sinarg2 - a2*sinarg1*CC_;
//  double a6 = a1*DD_;
//  double a7 = a2*cosarg1*DD_;
//  double a8 = a2*sinarg1*DD_;
//  double du = 0.0;
//  double c_tkaz = tka_/kSiderialDaySecs;
//  for ( int i = 0; i < 7; ++i ) {
//    double lvs = du*c_tkaz;
//    double xvs = ::cos(lvs);
//    double yvs = ::sin(lvs);
//    du = ::atan( (a3 + a4*xvs - a5*yvs)/(a6+a7*xvs-a8*yvs ) );
//  }
  *du = 0.0;
  double c_tkaz = tka_/kSiderialDaySecs;
  for ( int i = 0; i < 7; ++i ) {
    double lam1 = startpos_.lon() - c_tkaz*(*du);
    double lam2 = endpos_.lon() + c_tkaz*(duseans_ - *du);
//    double lam2 = endpos_.lon() + c_tkaz*(*du);
    double A = a1 + a2*::cos(gp.lon() - lam1);
    double B = b1 + b2*::cos(gp.lon() - lam2);
    double C = sin_fi_s_*sin_fi_e_ + cos_fi_s_*cos_fi_e_*::cos(lam1 - lam2);
    *du = ::atan( (B - A*C )/( A*::sqrt( 1 - C*C ) ) );
//    *du = ::atan2( (B - A*C ),( A*::sqrt( 1.0 - C*C ) ) );
  }
  return true;
}

bool POSproj::calcScanAngleKA( const GeoPoint& kapoint, const GeoPoint& gp, double du, double* sa ) const
{
  double a = ::acos( ::cos( kapoint.lat() )*::cos( ::fabs( kapoint.lon() - gp.lon() ) ) );
  double sina = ::sin(a);
  double A = M_PI_2;
  if ( false == MnMath::isZero(sina) ) {
    A = ::asin( ::sin( kapoint.lat() )/sina );
  }
  double cosd = ::cos( gp.lat() )*::cos(a) + ::sin( gp.lat() )*sina*::cos(M_PI_2-A);
  double lsc = ::fabs(::acos(cosd));
  if ( lsc > alpham_ + alpham_*0.1 ) {
    return false;
  }
  double bt = ::sqrt( kEarthRadiusKM*kEarthRadiusKM + satcoord_.pos.r*satcoord_.pos.r - 2.0*kEarthRadiusKM*satcoord_.pos.r*::cos(lsc) );
  bt = -fabs( ::asin(kEarthRadiusKM/bt*::sin(lsc) ) );
  
  double lamt = gp.lon() - kapoint.lon();
  if ( U_ + du < M_PI_2 ) {
    if ( 0.0 < lamt ) {
      bt = -bt;
    }
  }
  else if ( 0.0 > lamt ) {
    bt = -bt;
  }
  *sa = bt;
  return true;
}

}
