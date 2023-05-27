#include "vprofile.h"

#include <qmap.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
//#include <commons/obanal/haos.h>
#include <commons/obanal/interpolorder.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/proto/field.pb.h>

namespace meteo{
namespace map {


VProfile::VProfile()
{
}

VProfile::~VProfile()
{
}

bool VProfile::getCoord(int x,int y,GeoPoint *gp){

  int nProfiles = profiles_.count();
  if (1 > nProfiles){
    return false;
  }

  if (1 == nProfiles){
    *gp = profiles_.at(0).coord();
    gp->setAlt(y);
    return true;
  }

  for ( int i=0; i<nProfiles-1 ; ++i ) {
    const zond::PlaceData& p1 = profiles_.at(i);
    const zond::PlaceData& p2 = profiles_.at(i+1);
    if( p1.xPos() <= x && p2.xPos() > x  ){
      GeoPoint gp1 = p1.coord();
      GeoPoint gp2 = p2.coord();
      float k = (x - p1.xPos())/(p2.xPos() - p1.xPos());
      float fi = (gp2.fi() - gp1.fi())*k;
      float la = (gp2.la() - gp1.la())*k;
      *gp = GeoPoint(fi,la,y,LA_GENERAL);
      return true;
    }
  }
  return false;
}

void VProfile::setData(QList<zond::PlaceData>* profiles, const QVector<float> &levels)
{
  clear();
  profiles_ = *profiles;
  int nProfiles = profiles->count();
  zond::Uroven c_ur;
  int nLevels = levels.count();

  for ( int i=0; i<nProfiles; ++i ) {
    float x = profiles->at(i).xPos();
   // var(profiles->at(i)->xPos()) << "------------------------------------------------";
  //  profiles->at(i)->zond().print();
    for(int type = zond::UR_P; type < zond::LAST_UR; ++type) {
      zond::ValueType c_ur_type = static_cast<zond::ValueType>(type);
      if(profiles->at(i).zond().countValid(c_ur_type) < 3){
        continue;
      }

      for ( int z = 0; z < nLevels; ++z ) {
        float p = levels.at(z);
        if(profiles->at(i).zond().getUrPoP(p, &c_ur)
           &&control::DOUBTFUL > c_ur.quality(c_ur_type)){
          float fun = c_ur.value(c_ur_type);
          anyData.insertMulti(c_ur_type,DecartData(x,p,fun));
        } else {
          miss_p.insertMulti(c_ur_type,p);
        }
      }
      //QMap <zond::ValueType,DecartData> anyData;
    }

    zond::PlaceData pd = profiles->at(i);
    if( pd.zond().getTropo(&c_ur)
       &&control::DOUBTFUL > c_ur.quality(zond::UR_P)
       &&control::DOUBTFUL > c_ur.quality(zond::UR_T)){
      tropoData_T_.append(DecartData(x,c_ur.value(zond::UR_P),c_ur.value(zond::UR_T)));
    }

    if( pd.zond().getUrMaxWind(&c_ur)
       &&control::DOUBTFUL > c_ur.quality(zond::UR_P)
       &&control::DOUBTFUL > c_ur.quality(zond::UR_ff)
       &&control::DOUBTFUL > c_ur.quality(zond::UR_dd)){
      maxwindData_dd_.append(DecartData(x,c_ur.value(zond::UR_P),c_ur.value(zond::UR_dd)));
      maxwindData_ff_.append(DecartData(x,c_ur.value(zond::UR_P),c_ur.value(zond::UR_ff)));
    }

  }

}


bool VProfile::getTropo(QVector<DecartData> * temperature)
{
  if ( 0 == temperature ) { return false; }
  if ( 0 == tropoData_T_.size() ) { return false; }

  *temperature = tropoData_T_;

  return true;
}

bool VProfile::getMaxWind(QVector<DecartData> *dd, QVector<DecartData> *ff)
{
  if ( 0 == dd ) { return false; }
  if ( 0 == ff ) { return false; }
  if ( 0 == maxwindData_dd_.size() ) { return false; }
  if ( 0 == maxwindData_ff_.size() ) { return false; }

  *dd = maxwindData_dd_;
  *ff = maxwindData_ff_;

  return true;
}

void VProfile::clear()
{
  profiles_.clear();
  tropoData_T_.clear();
  maxwindData_dd_.clear();
  maxwindData_ff_.clear();
  miss_p.clear();
  anyData.clear();
}


bool VProfile::initField(obanal::TField* field, int type)
{
  if ( nullptr == field ) { return false; }
  int kol_src = anyData.values(type).size();
  QList <DecartData> srcData;
  for(int i=0;i<kol_src;++i){
   DecartData vs = anyData.values(type).value(i);
    if(!miss_p.values(type).contains(vs.y)){
      srcData.append(vs);
    }
  }

  if ( 4 > srcData.size() ) { return false; }

  int n = srcData.size();
  float vs = srcData.at(0).x;
  for(int i=1;i<n;i++){
    if( false == MnMath::isEqual( srcData.at(i).x, vs)) {
      vs = srcData.at(i).x;
    }
  }
  float minP = 9999;
  float maxP = -9999;
  float minX = 9999;
  float maxX = -9999;
  findMin(&minX, &minP, srcData);
  findMax(&maxX, &maxP, srcData);
  float step = 10.;
  if(CUT_LAYER_D == type) {
    step = 1.;
  }
  field->clear();
  field->setDecartNet(minX, minP, maxX, maxP , step,step);

  field->setTypeNet( GENERAL_NET);
return true;
}

bool VProfile::calc(obanal::TField* field, int type)
{
  if ( 0 == field ) { return false; }
  int kol_src = anyData.values(type).size();
  QList <DecartData> srcData;
  for(int i=0;i<kol_src;++i){
   DecartData vs = anyData.values(type).value(i);
    if(!miss_p.values(type).contains(vs.y)){
      srcData.append(vs);
    }
  }

  if ( 4 > srcData.size() ) { return false; }

  int n = srcData.size();
  int nFiFact = 0;

  float vs = srcData.at(0).x;
  for(int i=1;i<n;i++){
    if( false == MnMath::isEqual( srcData.at(i).x, vs)) {
      vs = srcData.at(i).x;
      nFiFact++;
    }
  }
  nFiFact++;
  if(nFiFact<1) {
    //debug_log << "Ошибка анализа сетки!";
    return false;
  }
  float minP = 9999;
  float maxP = -9999;
  float minX = 9999;
  float maxX = -9999;
  findMin(&minX, &minP, srcData);
  findMax(&maxX, &maxP, srcData);
  float step = 10.;
  if(CUT_LAYER_D == type) {
    step = 1.;
  }
  field->clear();
  field->setDecartNet(minX, minP, maxX, maxP , step,step);

  field->setTypeNet( GENERAL_NET);
  QVector<float> netfi_fact(nFiFact,0.);
  QVector<int> kol_point_fact(nFiFact,0);
  QVector<float> rez_vs(nFiFact*field->kolLa());
  QVector<float> yy(n,0.);
  QVector<float> fun(n,0.);
  nFiFact = 0;
  vs = srcData.at(n-1).x;
  netfi_fact[0]=vs;
  for(int i=n-1;i>=0;i--){
    if( false == MnMath::isEqual(srcData.at(i).x, vs))
    {
      nFiFact++;
      vs = srcData.at(i).x;
      netfi_fact[nFiFact] = vs;
      kol_point_fact[nFiFact] ++;
    }
    else {
      kol_point_fact[nFiFact] ++;
    }
    yy[n-i-1] =srcData.at(i).y;
    fun[n-i-1] =  srcData.at(i).fun;
  }
  qSort(netfi_fact);
  nFiFact++;
  int ret_val = InterpolY(field->netLa(), &rez_vs, &yy, &fun, kol_point_fact,nFiFact,field->kolLa());
  if(ret_val!=0) {
   // var(ret_val);
    return ret_val;
  }
 ret_val = InterpolX(field->netFi(),field,netfi_fact,rez_vs,field->kolFi(),nFiFact,field->kolLa());
  if ( ret_val != 0 ) {
 //   var(ret_val);
    return ret_val;
  }

 // field->smootchField(1);
  return true;
}


void VProfile::findMin(float* x, float* y, const QList<DecartData>& vector) const
{
  if ( 0 == x ) { return; }
  if ( 0 == y ) { return; }
  if ( vector.size() == 0 ) { return; }

  float minX = vector[0].x;
  float minY = vector[0].y;
  for ( int i=1,isz=vector.size(); i<isz; ++i ) {

    minX = qMin<float>(minX, vector[i].x);
    minY = qMin<float>(minY, vector[i].y);
    //   debug_log << minY_vs<<minY;
  }

  *x = minX;
  *y = minY;
}

void VProfile::findMax(float* x, float* y, const QList<DecartData>& vector) const
{
  if ( 0 == x ) { return; }
  if ( 0 == y ) { return; }
  if ( vector.size() == 0 ) { return; }

  float maxX = vector[0].x;
  float maxY = vector[0].y;
  for ( int i=1,isz=vector.size(); i<isz; ++i ) {
    maxX = qMax<float>(maxX, vector[i].x);
    maxY = qMax<float>(maxY, vector[i].y);
  }

  *x = maxX;
  *y = maxY;
}

}
} // meteo
