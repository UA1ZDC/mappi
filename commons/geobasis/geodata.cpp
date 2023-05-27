#include "geodata.h"

#include "projection.h"
#include <cross-commons/debug/tlog.h>

namespace meteo {

  bool foSortByFi(const MData &p1, const MData &p2)
 {
    return p1.point.fi() < p2.point.fi();
 }
  
  bool foSortByLa(const MData &p1, const MData &p2)
 {
    return p1.point.la() < p2.point.la();
 }
  
  
GeoData::GeoData()
  : QVector<MData>()
{
}

GeoData::GeoData( int sz )
: QVector<MData>(sz)
{
}



GeoData::~GeoData()
{
}

void GeoData::sortByFi(){
  qSort(this->begin(),this->end(),foSortByFi);
}


void GeoData::sortByLa(){
  qSort(this->begin(),this->end(),foSortByLa);
}


const GeoPoint& GeoData::maxAlt() const {
  float amax = data()->point.alt();
  int max_i = 0;
  for ( int i = 0, s = size(); i < s; ++i ) {
    if((data() + i)->point.alt() > amax){
      amax = (data() + i)->point.alt();
      max_i = i;
    }
  }
  return at(max_i).point;
}

void GeoData::move(const GeoPoint& dp){
  QVector<MData>::iterator it = begin();
  QVector<MData>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it).point += dp;
  }
}

void GeoData::addDeltaData(float delta){
  QVector<MData>::iterator it = begin();
  QVector<MData>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it).data += delta;
  }
  
}

void GeoData::mnozData(float mnoz){
  QVector<MData>::iterator it = begin();
  QVector<MData>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it).data *= mnoz;
  }
  
}




void GeoData::to360(){
  QVector<MData>::iterator it = begin();
  QVector<MData>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it).point.to360();
  }
}

void GeoData::to180(){
  QVector<MData>::iterator it = begin();
  QVector<MData>::iterator itEnd = end();
  for(;it != itEnd;++it){
    (*it).point.to180();
  }
}


TLog& operator<<( TLog& log, const GeoData& gv )
{
  log << QString("GeoData[%1]=(")
    .arg(gv.size() )
    .toLocal8Bit().data();
  for ( int i =0, sz = gv.size(); i < sz; ++i ) {
    log << '\n' << '\t' << gv[i].point<< '\t' << gv[i].data<< '\t' << gv[i].mask<< '\t' << gv[i].type_data;
  }
  return log << ");";
}


QDataStream& operator<<(QDataStream& in, const MData& gc){
  in<<gc.point;
  in<<gc.data;
  in<<gc.mask;
  int g = gc.type_data;
  in<< g;
  in<<gc.id;
  return in;
}

QDataStream& operator>>(QDataStream& out, MData& gc){
  out>>gc.point;
  out>>gc.data;
  out>>gc.mask;
  int g;
  out>> g;
  gc.type_data = static_cast<typeGeoData> (g);
  out>>gc.id;
  return out;
}


}
