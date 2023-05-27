#include "geopoint.h"

#include <string.h>

#include <new>

#include <math.h>

#include <qmath.h>
#include <qobject.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qvector.h>

#include <commons/geobasis/coords.h>
#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>

const meteo::GeoPoint meteo::GeoPoint::Invalid = meteo::GeoPoint(-999.999, -999.999);

namespace meteo {

const GeoPoint& GeoPoint::to360(){//!< широта от 0 до 180 долгота от 0 до 360
  if (LA360 == type_coord_) return *this;
  type_coord_ = LA360;
  lat_ = M_PI_2f - lat_ ;
    float lo = lonDeg() - int(lonDeg()/360.f)*360.f;
  while(lo < 0.f) {
    lo += 360.f;
  }
  setLonDeg(lo);
  return *this;
}

//!< широта от -90 до 90 долгота от -180 до 180
const GeoPoint& GeoPoint::to180()
{
  if (LA180 == type_coord_) {
    return *this;
  }
  type_coord_ = LA180;
  lon_ = MnMath::PiToPi(lon_);
  return *this;
}


GeoPoint::GeoPoint()
: lat_(Invalid.lat()),
  lon_(Invalid.lon()),
  type_coord_(LA180),
  alt_(0.0)
{
}

GeoPoint::GeoPoint( float lt, float ln, float a, typeGeoCoord atc )
: lat_(lt),
  lon_(ln),
  type_coord_(atc),
  alt_(a)
{
 // if ( LA180 == type_coord_ ) { lon_ = MnMath::PiToPi(lon_); }
}

void GeoPoint::setPoint( float lt, float ln, float a , typeGeoCoord type_coord  )
{ lat_ = lt;
  lon_ = ln;
  type_coord_ = type_coord;
  alt_ = a;
  if ( LA180 == type_coord_ )
    {
      lon_ = MnMath::PiToPi(lon_);
    }
}

void GeoPoint::setLat( float lt ) {
  lat_ = lt;
}
void GeoPoint::setLon( float ln ) {
  lon_ = ln;
  if ( LA180 == type_coord_ ) {
      lon_ = MnMath::PiToPi(lon_);
    }
}
void GeoPoint::setLatDeg( float lt ) {
  lat_ = lt*DEG2RAD;
}
void GeoPoint::setLonDeg( float ln ){
lon_ = ln*DEG2RAD;
if ( LA180 == type_coord_ ) {
    lon_ = MnMath::PiToPi(lon_);
  }
}

void GeoPoint::setFi( float lt ) {
  lat_ = lt;
}
void GeoPoint::setLa( float ln ) {
  lon_ = ln;
  if ( LA180 == type_coord_ ) {
      lon_ = MnMath::PiToPi(lon_);
    }
}
void GeoPoint::setFiDeg( float lt ) {
  lat_ = lt*DEG2RAD;
}
void GeoPoint::setLaDeg( float ln ) {
  lon_ = ln*DEG2RAD;
  if ( LA180 == type_coord_ ) {
      lon_ = MnMath::PiToPi(lon_);
    }
}



QString GeoPoint::toString(bool withAlt, const QString& templ) const
{
  QString s = templ;

  if ( s.isNull() ) {
    if ( LA_GENERAL == type() ) {
      s = QObject::tr("X: %1  Y: %2");
      if ( withAlt ) { s += QObject::tr("  Z: %3"); }
    }
    else {
      s = QObject::tr("Широта: %1  Долгота: %2");
      if ( withAlt ) { s += QObject::tr("  Высота: %3"); }
    }
  }

  s = s.arg(strLat(), strLon());
  if ( withAlt ) { s = s.arg(strAlt()); }

  return s;
}

QString GeoPoint::strLat() const {

  if(LA_GENERAL == type() ){
    return QString::number( lat());
  }

  MnMath::Angle deg;
  rad2deg60( lat(), deg );
  QString latmsg;
  QString apndx;
  if ( 0 > deg.degree || 0 > deg.minute || 0 > deg.second ) {
    deg.degree = -deg.degree;
    deg.minute = -deg.minute;
    deg.second = -deg.second;
    apndx = QObject::tr("ю.ш.");
  }
  else {
    apndx = QObject::tr("с.ш.");
  }
  QString latstr = QString::fromUtf8("%1°%2'%3'' %4")
    .arg(deg.degree, 3)
    .arg(deg.minute, 2)
    .arg(deg.second, 2)
    .arg(apndx);
  return latstr;
}

QString GeoPoint::strLon() const {
  if(LA_GENERAL == type() ){
    return QString::number( lon());
  }

  MnMath::Angle deg;
  rad2deg60( lon(), deg );
  QString lonmsg;
  QString apndx;
  if ( 0 > deg.degree  || 0 > deg.minute || 0 > deg.second ) {
    deg.degree = -deg.degree;
    deg.minute = -deg.minute;
    deg.second = -deg.second;
    apndx = QObject::tr("з.д.");
  }
  else {
    apndx = QObject::tr("в.д.");
  }
  QString latstr = QString::fromUtf8("%1°%2'%3'' %4")
    .arg(deg.degree, 3)
    .arg(deg.minute, 2)
    .arg(deg.second, 2)
    .arg(apndx);
  return latstr;
}

QString GeoPoint::strAlt() const {
  if(LA_GENERAL == type() ){
    return QString::number( alt_);
  }

  return QString::number(alt_) + " м.";
}

/*!
* \brief Определение географических координат второй точки по известным координатам первой точки,
*  расстоянию между ними и азимуту
* \param d Угловое расстояние между первой точкой geo1 и искомой точкой
* \param a азимут
* \return Искомая вторая точка
*/
GeoPoint GeoPoint::findSecondCoord( const float d, const float a ) const {
  GeoPoint geo2;
  float divider = cosf(lat())*cosf(d)-sinf(lat())*sinf(d)*cosf(a);
  geo2.setLat( MnMath::spAsin( sinf(lat())*cosf(d) + cosf(lat())*sinf(d)*cosf(a) ));
  geo2.setLon( MnMath::PiToPi(atan2f( sinf(d)*sinf(a), divider ) + lon()));
  return geo2;
}

/*!
 * \brief:  Определение географических координат точки пересечения двух дуг большого круга,
 *          где одна из дуг задана её отрезком (line.first--line.second),
 *          а вторая - перпендикуляр из точки this к первой.
 * \param:  line - отрезок дуги большого круга.
 * \return: Координаты точки пересечения.
 */
GeoPoint GeoPoint::findPerpendicularIntersection(const QPair<GeoPoint, GeoPoint>& line) const
{
  if (this->isValid() == false ||
      line.first.isValid() == false ||
      line.second.isValid() == false) {
    return Invalid;
  }

  const GeoPoint& start = line.first;
  const GeoPoint& end = line.second;
  const GeoPoint& alto = *this;

  float start2Alto = start.orthodromicDirection(alto);
  float start2End = start.orthodromicDirection(end);
  float start2EndSuplemento = start2End + M_PIf;

  if (start2EndSuplemento > 2*M_PIf) {
    start2EndSuplemento -= 2*M_PIf;
  }

  float start2Intersection = -1.0;
  if (qAbs(start2End - start2Alto) < M_PI_2f) {
    start2Intersection = start2End;
  }
  else if (qAbs(start2EndSuplemento - start2Alto) < M_PI_2f) {
    start2Intersection = start2EndSuplemento;
  }
  else {
    if (qAbs(start2EndSuplemento - start2Alto) > qAbs(start2End - start2Alto)) {
      start2Intersection = start2EndSuplemento;
    }
    else {
      start2Intersection = start2End;
    }
  }

  float angle = qAbs(start2Intersection - start2Alto);
  float distanceStart2Alto = start.radianDistance(alto);

  float tan_sup_s2i = 1.0f / (cosf(angle)*tanf(distanceStart2Alto));
  float sup_s2i = atanf(tan_sup_s2i);
  float distanceStartIntersect = M_PI_2f - sup_s2i;

  return start.findSecondCoord(distanceStartIntersect, start2Intersection);
}

//! Расчет расстояния по ортодромии(дуга большого круга )
//! конечная точка g
//! угол в радианах

float GeoPoint::calcUgolOrt(const GeoPoint &ag) const
{
  float a,b,c,d,ugol;
  float r;
  GeoPoint g = ag;
  a = (M_PI_2f-fi());
  b = (M_PI_2f-g.fi());
  c = MnMath::PiToPi(g.la() - la());
  d = acos(cosf(a)*cosf(b)+sinf(a)*sinf(b)*cosf(c));

 if(true != MnMath::isZero(d)){
 //if(d != 0){
    ugol = asinf(sinf(b)*sinf(fabs(c))/sinf(d));
    r = acosf(cosf(a)*cosf(d)+sinf(a)*sinf(d)*cosf(ugol));
    if(MnMath::isEqual(r,b)) ugol=M_PIf-ugol;
    if (c<0) ugol = MnMath::M_2PI-ugol;
  }
  else ugol=0.;
  ugol = MnMath::PiToPi(ugol)+M_PIf;
  return ugol;
}

//! Расчет расстояния по локсодромии(кривая, пересекающая все меридианы под постоянным углом)
//! конечная точка g
//! angle - угол в радианах
float GeoPoint::calcDistLoks(const GeoPoint &ag, float angle) const
{
  float a,c,d,angle1,dfi,fi1,fi0,dfi2;
  int kol_step,i;
  GeoPoint g = ag;
  GeoPoint vs=*this;

  angle1=MnMath::PiToPi(M_PI_2f - angle);

  if (MnMath::isEqual(angle1,M_PI_2f) || MnMath::isEqual(angle1,-M_PI_2f)){
  //if (angle1 == M_PI_2 || angle1 == -M_PI_2){
    d=fabs(vs.fi() - g.fi() );
  }  else  {
    a=tan(angle1);
    if(false == MnMath::isZero(a)) {
      a=1.f/a;
      a=a*a;
      kol_step=abs(int((g.fi()-vs.fi())*100));
      dfi=(g.fi()-vs.fi())/kol_step;
      dfi2=dfi*0.5f;
      fi0=vs.fi();
      d=0.;
      for(i=0;i<kol_step;i++) {
        fi1=fi0+dfi*i+dfi2;
        d+=sqrtf(1.f+cosf(fi1)*cosf(fi1)*a)*dfi;
      }
      d=fabsf(d);
    }
    else{
      if(MnMath::isEqual( g.fi(),vs.fi())){
        c=(g.la()-vs.la());
        d=fabsf(c)*cosf(g.fi());
      } else {
        d=100.f*M_PIf;
      }
    }
  }

  return d;
}

void GeoPoint::setIn(){
  while (fi() < -M_PI_2f) setFi(-M_PIf - fi());
  while (fi() > M_PI_2f) setFi(M_PIf- fi());
  while (la() < -M_PIf) setLa(la()+k2PI);
  while (la() > M_PIf) setLa(la()-k2PI);
}


void GeoPoint::calcFLOrt(GeoPoint *g, float rast, float ugol)
{
  float alfa, b, c, t;
  float dl, r;

  b=(M_PI_2f-fi());
  alfa=ugol;
  t=rast/6.371e3f;

  c=acos(cosf(t)*cosf(b)+sinf(t)*sinf(b)*cosf(alfa));
  g->setFi(M_PI_2f-c);
  dl=asinf(sinf(t)*sinf(alfa)/sinf(c));
  g->setLa(la() + dl);
  r=calcDistance(*g);
  if(MnMath::isEqual(r,rast)) {dl = MnMath::M_2PI-dl; g->setLa(la()+dl);}
  g->setIn();
}



float GeoPoint::calcUgolLoks(const GeoPoint &ag)
{
  float a,b,c,d,ugol;
  GeoPoint g = ag;

  a=(fi() + M_PI_2f);
  b=(g.fi() + M_PI_2f);

  //if ( a == M_PI || b == M_PI ) {
  if (MnMath::isEqual( a, M_PIf) || MnMath::isEqual(b,M_PIf) ) {
    return 0;
  }
  c =MnMath::PiToPi(g.la()-la());


  if(!MnMath::isEqual(a,b))
  //if(a!=b)
  {
    d=logf(tanf(b*0.5f)/tanf(a*0.5f))/c;
    ugol=atanf (d);
    if(c<0) ugol+=M_PIf;
    ugol=M_PI_2f-ugol;
  }
  else
  {
    if(g.la()>la()) ugol = M_PI_2f;
    else  ugol = 3*M_PI_2f;
  }
  ugol=MnMath::PiToPi(ugol)+M_PIf;
  return ugol;
}


float GeoPoint::radianDistanceFast(const GeoPoint& p2) const
{
  if(compare(p2) ) return 0.;
  float a,b,c,d;
  a = M_PI_2f- lat();
  b = M_PI_2f - p2.lat();
  c = lon() - p2.lon();
  float h = cosf(a)*cosf(b)+sinf(a)*sinf(b)*cosf(c);
  d = acosf(h);
  return d;
}

float GeoPoint::radianDistance(const GeoPoint& p2) const
{
  if(compare(p2) ) return 0.;
  float a,b,c,d;
  a=M_PI_2f - lat();
  b=M_PI_2f - p2.lat();
  c=lon() - p2.lon();
  float h = cosf(a)*cosf(b)+sinf(a)*sinf(b)*cosf(c);
  d=acosf(h);
  return d;
}

//угол между двумя сторонами сферич треугольника
float GeoPoint::ugolA(const GeoPoint& pB, const GeoPoint& pC) const{
  if(pB == pC) return 0.;
  float a = pB.radianDistance(pC);
  float b = radianDistance(pC);
  float c = radianDistance(pB);
  float h = acosf( (cosf(a) - cosf(b)*cosf(c))/(sinf(b)*sinf(c)));
  return h;
}

float GeoPoint::calcDistance(const GeoPoint& p2) const // NB: returns in kilometers
{
  float d = radianDistance(p2);
  return d*6.371e3f; //FIXME
}

//азимут на точку p2
float GeoPoint::simpAzimut(const GeoPoint& p2) const
{
  float ang = ::atan2f(p2.lon()-lon(),p2.lat()-lat());
  if (ang < 0.0f) {
      ang += 2.f*M_PIf;
    }
  return ang;
}


float GeoPoint::radianAzimuth(const GeoPoint& p2) const
{
  Coords::GeoCoord from, to;
  from.setGeoPoint(*this);
  to.setGeoPoint(p2);

  return geoAzimooth(from, to);
}

/*!
 * \brief:  Определение направления меньшего из отрезков дуги большого круга,
 *          начало отрезка - точка this, его конец - точка p2.
 * \return: Величина угла в радианах.
 */
float GeoPoint::orthodromicDirection(const GeoPoint& p2) const
{
  return (qAbs(this->lon() - p2.lon()) > M_PIf) ? (2*M_PIf - this->radianAzimuth(p2))
                                               : this->radianAzimuth(p2);
}

GeoPoint& GeoPoint::operator+(const GeoPoint& src){
  lat_+=src.lat_;
  lon_+=src.lon_;
  alt_+=src.alt_;
  return *this;
}

GeoPoint& GeoPoint::operator+=(const GeoPoint& src){
  lat_+=src.lat_;
  lon_+=src.lon_;
  alt_+=src.alt_;
  return *this;
}

bool  GeoPoint::operator==(const GeoPoint& src){
  if( qFuzzyCompare(src.lat_,this->lat_)
    && qFuzzyCompare(src.lon_,this->lon_)
    && qFuzzyCompare(src.alt_,this->alt_)){
    return true;
  }
  return false;
}

bool GeoPoint::operator< (const GeoPoint& rhs) const
{
  if (this == &rhs) {
    return false;
  }

  if (this->lat_ < rhs.lat_) {
    return true;
  }
  if (this->lon_ < rhs.lon_) {
    return true;
  }
  if (this->alt_ < rhs.alt_) {
    return true;
  }
  return false;
}

bool GeoPoint::compare(const GeoPoint& point) const
{
  return ( MnMath::isEqual(lat_,point.lat())
           && MnMath::isEqual(lon_, point.lon())
           && MnMath::isEqual(alt_, point.alt()));
}

bool GeoPoint::compareLatLon(const GeoPoint& point) const
{
  return ( MnMath::isEqual(lat_, point.lat())  && MnMath::isEqual(lon_, point.lon()));
}

int32_t GeoPoint::size()
{
  int32_t latsize = sizeof(double);
  int32_t lonsize = sizeof(double);
  int32_t type_coord_size = sizeof(GeoPoint::type_coord_);
  int32_t altsize = sizeof(double);

  return latsize + lonsize + type_coord_size + altsize;
}

QByteArray GeoPoint::data() const
{
  QByteArray arr;
  getData(&arr);
  return arr;
}

int32_t GeoPoint::getData( QByteArray* arr ) const
{
  arr->resize( size() );
  return getData(arr->data());
}

int32_t GeoPoint::getData( char* d ) const
{
  size_t latsize = sizeof(double);
  size_t lonsize = sizeof(double);
  size_t type_coord_size = sizeof(type_coord_);
  size_t altsize = sizeof(double);
  double tmp = lat_;
  ::memcpy( d, &tmp, latsize );
  tmp = lon_;
  ::memcpy( d += latsize, &tmp, lonsize );
  ::memcpy( d += lonsize, &type_coord_, type_coord_size );
  tmp = alt_;
  ::memcpy( d += type_coord_size, &tmp, altsize );
  return latsize + lonsize + type_coord_size + altsize;
}

GeoPoint GeoPoint::fromData( const QByteArray& arr )
{
  const char* d = arr.data();
  return fromData(d);
}

GeoPoint GeoPoint::fromData( const char* d )
{
  char* ld = const_cast<char*>(d);
  size_t latsize = sizeof(double);
  size_t lonsize = sizeof(double);
  size_t type_coord_size = sizeof(GeoPoint::type_coord_);
  size_t altsize = sizeof(double);
  GeoPoint gp;
  double tmp;
  ::memcpy( &tmp, ld, latsize );
  gp.lat_ = tmp;
  ::memcpy( &tmp, ld += latsize,lonsize );
  gp.lon_ = tmp;
  ::memcpy( &gp.type_coord_, ld += lonsize, type_coord_size );
  ::memcpy( &tmp, ld += type_coord_size, altsize );
  gp.alt_ = tmp;
  return gp;
}

GeoPoint& GeoPoint::operator-(const GeoPoint& src){
  lat_-=src.lat_;
  lon_-=src.lon_;
  alt_-=src.alt_;
  return *this;
}

GeoPoint& GeoPoint::operator-=(const GeoPoint& src){
  lat_-=src.lat_;
  lon_-=src.lon_;
  alt_-=src.alt_;
  return *this;
}

QDataStream& operator<<(QDataStream& in, const GeoPoint& gc){
 in<<gc.lat_;
 in<<gc.lon_;
 in<<gc.alt_;
 in << static_cast<int>(gc.type_coord_);
 return in;
}

QDataStream& operator>>(QDataStream& out, GeoPoint& gc){
 out>>gc.lat_;
 out>>gc.lon_;
 out>>gc.alt_;
 int val;
 out >> val;

 gc.type_coord_ = meteo::typeGeoCoord(val);
 return out;
}

QString& operator<<( QString& str, const GeoPoint& gc )
{
  if(LA_GENERAL == gc.type() ){
    return str += QString("GeoPoint(%1,%2,%3)")
    .arg( QString::number(gc.lat(),'f',3) )
    .arg( QString::number(gc.lon(),'f',3) )
    .arg( QString::number(gc.alt(),'f',1) ).toLocal8Bit();
  }

  return str += QString("GeoPoint(%1,%2,%3)")
  .arg( QString::number(gc.latDeg(),'f',3) )
  .arg( QString::number(gc.lonDeg(),'f',3) )
  .arg( QString::number(gc.alt(),'f',1) ).toLocal8Bit();


}

TLog& operator<<( TLog& log, const GeoPoint& gc )
{
  QString str;
  str << gc;
  return log << str;
}

/*!
 * \brief:  Определение набора точек, составляющих меньший из отрезков дуги большого круга,
 *          заданной точками from и to.
 * \param:  from - точка начала отрезка;
 * \param:  to - точка конца отрезка;
 * \param:  radianStep - шаг (в радианах), через который рассчитываются промежуточные точки.
 * \return: набор точек от from до to с шагом radianStep (может быть неявно преобразован в GeoVector).
 */
QVector<GeoPoint> orthodromicLine(const GeoPoint& from, const GeoPoint& to, float radianStep)
{
  return divideLineBySteps(from, to, radianStep);
}

QVector<GeoPoint> divideLineBySteps(const GeoPoint& from, const GeoPoint& to, float radianStep)
{
  if (from.isValid() == false ||
      to.isValid() == false ||
      (radianStep > 0.0f) == false) {
    return QVector<GeoPoint>();
  }

  float direction = from.orthodromicDirection(to);
  float distance = from.radianDistance(to);
  int stepCount = static_cast<int>(distance/radianStep);

  QVector<GeoPoint> result;
  result.reserve(stepCount + 2);
  result.append(from);

  for (int i = 0; i <= stepCount; ++i) {
    GeoPoint intermediatePoint = from.findSecondCoord((i * radianStep), direction);
    result.append(intermediatePoint);
  }

  result.append(to);
  return result;
}

}

