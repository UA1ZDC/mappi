#include "placedata.h"

namespace zond {

PlaceData::PlaceData()
{
  clear();
}

void PlaceData::clear()
{
  zond_.clear();
  ruName_="";
  enName_="";
  index_ = "";
  coord_ = meteo::GeoPoint();
  dt_ = QDateTime();
  name_ = "";
  xPos_ = -9999.;
  helpPoint_ = false;
}

QString PlaceData::docName()const {
  QString name("");
  if (!ruName().isEmpty()) {
    name += ruName();
  } else if (!enName().isEmpty()) {
    name += enName();
  } else if (!index().isEmpty()) {
    name +=  index();
  }
  if(zond().centersName().size()>0){
    name += " по анализу (" + zond().centersName().join(";")+") ";
 }
  name += ( !dt().isValid() ) ? "" : QString("_") + dt().toString("MM-dd-yyyy hh:mm");
  name += "";
  name.replace('/', "");
  return name;
}


bool PlaceData::setData(const meteo::surf::OneZondValueOnStation &one_zond){
  bool hasFi  = one_zond.point().has_fi();
  bool hasLa = one_zond.point().has_la();
  bool hasDt = one_zond.has_date();

  if ( !hasFi || !hasLa || !hasDt ) {
    return false;
  }
  zond_.setDataNoPreobr(one_zond);
  //  debug_log<< one_zond.DebugString();
  QString dtStr = QString::fromUtf8(one_zond.date().c_str());
  QDateTime adt = QDateTime::fromString(dtStr, Qt::ISODate);
  adt.setTimeSpec(Qt::UTC);
  adt = adt.addSecs(one_zond.hour());

  bool validDt = adt.isValid();

  if ( !validDt ) {
    return false;
  }

  dt_ = adt;
  // debug_log<<one_zond.point().DebugString();
  coord_.setFi(one_zond.point().fi());
  coord_.setLa(one_zond.point().la());
  // zond.print();
  return true;
}



bool PlaceData::getMeteoData(double pur, TMeteoData *md)const {
  zond::Uroven ur;
  if(!zond_.getUrPoP(pur,&ur)) return false;
  md->clear();
  for(int type = zond::UR_P; type < zond::LAST_UR; ++type) {
    zond::ValueType ut = static_cast <zond::ValueType> (type);
    md->add(zond::descrPoUrType(ut),
                    TMeteoParam( "", ur.value(ut),control::RIGHT));
  }
  return true;
}

int32_t PlaceData::dataSize() const
{
  int32_t sz = sizeof(int32_t);
  sz += index_.toUtf8().size();
  sz += sizeof(int32_t);
  sz += ruName_.toUtf8().size();
  sz += sizeof(int32_t);
  sz += enName_.toUtf8().size();
  sz += sizeof(int32_t);
  sz += coord_.size();
  sz += sizeof(int32_t);
  sz += dt_.toString(Qt::ISODate).toUtf8().size();
  sz += sizeof(int32_t);
  sz += name_.toUtf8().size();
  sz += sizeof(xPos_);
  sz += sizeof(echelon_);
  sz += sizeof(helpPoint_);
  sz += sizeof(int32_t);
  QByteArray loc;
  zond_ >> loc;
  sz += loc.size();
  return sz;
}

int32_t PlaceData::serializeToArray( char* arr ) const
{
  int32_t pos = 0;
  QByteArray loc = index_.toUtf8();
  int32_t sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;
  loc = ruName_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;
  loc = enName_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;
  sz = coord_.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  loc = coord_.data();
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;
  loc = dt_.toString(Qt::ISODate).toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;
  loc = name_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;
  ::memcpy( arr + pos, &xPos_, sizeof(xPos_) );
  pos += sizeof(xPos_);
  ::memcpy( arr + pos, &echelon_, sizeof(echelon_) );
  pos += sizeof(echelon_);
  ::memcpy( arr + pos, &helpPoint_, sizeof(helpPoint_) );
  pos += sizeof(helpPoint_);
  loc.clear();
  zond_ >> loc;
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  return pos;
}

int32_t PlaceData::parseFromArray( const char* data )
{
  int32_t pos = 0;

  int32_t sz = 0;
  ::memcpy( &sz, data + pos, sizeof(sz) );
  pos += sizeof(sz);
  index_ = QString::fromUtf8( data + pos, sz );
  pos += sz;

  ::memcpy( &sz, data + pos, sizeof(sz) );
  pos += sizeof(sz);
  ruName_ = QString::fromUtf8( data + pos, sz );
  pos += sz;
  ::memcpy( &sz, data + pos, sizeof(sz) );
  pos += sizeof(sz);
  enName_ = QString::fromUtf8( data + pos, sz );
  pos += sz;
  ::memcpy( &sz, data + pos, sizeof(sz) );
  pos += sizeof(sz);
  coord_ = meteo::GeoPoint::fromData( data + pos );
  pos += sz;
  ::memcpy( &sz, data + pos, sizeof(sz) );
  pos += sizeof(sz);
  dt_ = QDateTime::fromString( QString::fromUtf8(data + pos, sz ), Qt::ISODate );
  pos += sz;
  ::memcpy( &sz, data + pos, sizeof(sz) );
  pos += sizeof(sz);
  name_ = QString::fromUtf8( data + pos, sz );
  pos += sz;
  ::memcpy( &xPos_, data + pos, sizeof(xPos_) );
  pos += sizeof(xPos_);
  ::memcpy( &echelon_, data + pos, sizeof(echelon_) );
  pos += sizeof(echelon_);
  ::memcpy( &helpPoint_, data + pos, sizeof(helpPoint_) );
  pos += sizeof(helpPoint_);
  ::memcpy( &sz, data + pos, sizeof(sz) );
  pos += sizeof(sz);
  QByteArray loc = QByteArray::fromRawData( data + pos, sz );
  pos += sz;
  zond_ << loc;
  return pos;
}

const PlaceData& operator>>(const PlaceData& data, QByteArray& out)
{
  out.resize( data.dataSize() );
  data.serializeToArray( out.data() );
  return data;
}

PlaceData& operator<<(PlaceData& data, const QByteArray& ba)
{
  data.parseFromArray(ba.data());
  return data;
}


}
