#include "geovector.h"

#include "projection.h"
#include <cross-commons/debug/tlog.h>

namespace meteo {
  
GeoVector::GeoVector()
  : QVector<GeoPoint>()
{
}

GeoVector::GeoVector(const QVector<GeoPoint>& other)
  : QVector<GeoPoint>(other)
{
}

GeoVector::GeoVector( int sz )
  : QVector<GeoPoint>(sz)
{
}

GeoVector::~GeoVector()
{
}

void GeoVector::closeVector()
{
  if( first() != last() ) {
    append(first());
  }
}

const GeoPoint& GeoVector::maxAlt() const {
  float amax=data()->alt();
  int max_i=0;
  for ( int i = 0, s = size(); i < s; ++i ) {
    if((data() + i)->alt() > amax){
      amax = (data() + i)->alt();
      max_i = i;
    }
  }
  return at(max_i);
}

void GeoVector::move(const GeoPoint& dp){
  QVector<GeoPoint>::iterator it = begin();
  QVector<GeoPoint>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it)+=dp;
  }
}

void GeoVector::to360(){
  QVector<GeoPoint>::iterator it = begin();
  QVector<GeoPoint>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it).to360();
  }
}

void GeoVector::to180(){
  QVector<GeoPoint>::iterator it = begin();
  QVector<GeoPoint>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it).to180();
  }
}



int GeoVector::contains(const GeoPoint & ap) const
{
  if ( 3 > count() ) { return OUTSIDE; }

  float ugol = 0.;
  for ( int i = 0;  i < size()-1; ++i ) {
    ugol += ap.ugolA(at(i),at(i+1));
  }
  if ( first() != last() ) {
    ugol += ap.ugolA(last(),first());
  }
  
//  if (fabs (ugol*RAD2DEG) > 300.)
 // debug_log<<"ugol"<<ugol*RAD2DEG;
  
  return ((ugol > M_PI*2. || MnMath::isEqual(ugol, (float)M_PI*2)) ? INSIDE : OUTSIDE);
// ugol *= RAD2DEG;
// if (fabs (ugol) > 300.) debug_log<<"ugol"<<ugol<<(ugol - 360. < 1.e-6);
// return ((ugol >= 360. && ugol < 361.) ? INSIDE : OUTSIDE);
}

int32_t GeoVector::dataSize() const
{
  return QVector::size()*GeoPoint::size() + sizeof(int32_t);
}

QByteArray GeoVector::byteData() const
{
  QByteArray arr( dataSize(), '\0' );
  getData(&arr);
  return arr;
}

int32_t GeoVector::getData( QByteArray* arr ) const
{
  char* data = arr->data();
  return getData(data);
}

int32_t GeoVector::getData( char* data ) const
{
  int32_t gpsz = GeoPoint::size();
  int32_t sz = QVector::size();
  ::memcpy( data, &sz, sizeof(sz) );
  data += sizeof(sz);
  for ( int i = 0; i < sz; ++i ) {
    const GeoPoint& gp = QVector::operator[](i);
    gp.getData(data);
    data += gpsz;
  }
  return gpsz*sz + sizeof(sz);
}

GeoVector GeoVector::fromData( const QByteArray& arr )
{
  return GeoVector::fromData( arr.data() );
}

GeoVector GeoVector::fromData( const char* data )
{
  GeoVector gv;
  int32_t gpsz = GeoPoint::size();
  int32_t vecsize;
  ::memcpy( &vecsize, data, sizeof(vecsize) );
  data += sizeof(vecsize);
  gv.resize(vecsize);
  for ( int i = 0; i < vecsize; ++i ) {
    gv[i] = GeoPoint::fromData(data);
    data += gpsz;
  }
  return gv;
}
 
GeoVector& GeoVector::operator<<( const GeoPoint& gp )
{
  this->append(gp);
  return *this;
}
 
GeoVector& GeoVector::operator<<( const GeoVector& gv )
{
  (*this)+= gv;
  return *this;
}

GeoVector& GeoVector::operator<<( const QVector<GeoPoint>& gv )
{
  (*this)+=gv;
  return *this;
}


QString& operator<<( QString& str, const GeoVector& gv )
{
  str += QString("GeoVector[%1]=(")
    .arg(gv.size() )
    .toLocal8Bit();
  for ( int i =0, sz = gv.size(); i < sz; ++i ) {
    str += "\n\t";
    str << gv[i];
  }
  return str += ");";
}

TLog& operator<<( TLog& log, const GeoVector& gv )
{
  log << QString("GeoVector[%1]=(")
    .arg(gv.size() )
    .toLocal8Bit().data();
  for ( int i =0, sz = gv.size(); i < sz; ++i ) {
    log << '\n' << '\t' << gv[i];
  }
  return log << ");";
}



/*//!двигает вектор на dx dy экранных координат (аналог GeoArrayToGeoArray)
bool GeoVector::moveByXy(int dx, int dy, Projection *prj){
//  int GeoArrayToGeoArray(GeoPoint *geo_ish, GeoPoint *geo, PrjPar *aprj, int kol,
 //                      int dx, int dy)

  int  par1;
  QPolygon ar;
  int ret_val=0, param;

  par1=prj->getScale();
  prj->scale(2);
  param=int(pow(2.,float(par1-2)));
  dx*=param;
  dy*=param;
  prj->F2X(*this,&ar);
  ar.translate(dx,dy);
  ret_val=prj->X2F(ar,this);
  prj->scale(par1);
  return ret_val;
}
*/
/*bool GeoVector::rechangePointsEasy(const TGis::Projection &prj)
{
 if(size() <3 ) return false;
 int x,y,i;
 float ugol, ugol_n, ugol_vs;
 int n, n1;
 TGis::GeoPoint vs;
 TGis::Projection aprj;
 aprj = prj;
//TGis::GeoVector *mas,
 QPolygon scrPt;
 bool ret_val=false;

 if(1.e-5 < fabs((first().fi()-last().fi())*(first().fi()-last().fi())+
         (first().la()-last().la())*(first().la()-last().la()))){
          return false;
 }
 aprj.scale(2);
 aprj.F2X(*this,&scrPt);
 int kol =size();
 y = scrPt.at(1).x()-scrPt.at(0).x();
 x = scrPt.at(1).y()-scrPt.at(0).y();
 ugol = atan2(float(y),float(x));
 ugol_n = ugol;
 for(i=2;i<kol;i++)
   {
    y = scrPt.at(i).x()-scrPt.at(0).x();
    x = scrPt.at(i).y()-scrPt.at(0).y();
    if(x==0 && y==0) continue;
    ugol_vs = atan2(float(y),float(x));
    ugol_vs -= ugol;
    if(ugol_vs<-M_PI) ugol_vs+= MnMath::M_2PI;
    if(ugol_vs>M_PI) ugol_vs-= MnMath::M_2PI;
    ugol+=ugol_vs;
   }
 ugol-=ugol_n;
 if(ugol<0)
  {
   ret_val=true;
   n=kol/2;
   n1=kol-1;
   for(i=0;i<n;i++){
      vs=at(i);
      replace(i, at(n1-i));
      replace(n1-i,vs);
     }
  }
return ret_val;
}*/




}
