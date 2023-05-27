#include "layerprofile.h"

#include <quuid.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/generalproj.h>
#include <commons/geom/geom.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/obanal/tfield.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/geoaxis.h>
#include <meteo/commons/ui/map/layeriso.h>

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerProfile(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerProfile::Type, createLayer );
}

LayerProfile::LayerProfile(Document* d, const QString& n )
  : LayerAxis(d, n)
{
}

LayerProfile::~LayerProfile()
{}

QString LayerProfile::addProfile(float x, const QVector<float>& levels, float angle)
{
  if ( nullptr == document() ) { return QString(); }

  GeoGroup* group = new GeoGroup(this);

  float minP = levels.first();
  float maxP = levels.last();
  GeoPoint p1(x, minP, 0, LA_GENERAL);
  GeoPoint p2(x, maxP, 0, LA_GENERAL);
  GeoVector axisSkelet;
  axisSkelet << p1 << p2;
  map::GeoAxis* axis = new map::GeoAxis(group);

  QPen pen = axis->qpen();
  pen.setWidth(1);
  pen.setColor(Qt::black);
  axis->setPen(pen);
  axis->setSkelet(axisSkelet);
  axis->setTickValues(levels);
  axis->setRange(minP, maxP);

  QVector<GeoPoint> coords;
  coords.fill(GeoPoint(x,0,0,LA_GENERAL),levels.size());

  for ( int i = 0, isz = levels.size(); i < isz; ++i ) {
    coords[i].setLon(levels.at(i));
  }

  const QVector<TMeteoData> vec = getDataByCoords(coords);

  for ( int i = 0, isz = levels.size(); i < isz; ++i ) {
    float y = levels.at(i);
    const TMeteoData& md = vec.at(i);
    if ( md.count() == 0 ) { continue; }

    meteo::GeoVector gv;
    gv.append(GeoPoint(x,y,0,meteo::LA_GENERAL));

    meteo::map::Puanson* p = new meteo::map::Puanson(group);
    p->setSkelet(gv);
    p->setPunch(puanson_);
    p->setMeteodata(md);
    if ( angle != 0 ) {
      p->windCorrection(angle);
    }
  }

  return group->uuid();
}

QVector<TMeteoData> LayerProfile::getDataByCoords(const QVector<GeoPoint>& coords)
{
  if ( nullptr == document() ) { return QVector<TMeteoData>(); }
  if ( coords.isEmpty() ) { return QVector<TMeteoData>(); }

  QVector<TMeteoData> vec;
  vec.resize(coords.size());

  Document* doc = document();
  for ( Layer* layer : doc->layers() ) {
    LayerIso* l = maplayer_cast<LayerIso*>(layer);
    if ( nullptr == l ) { continue; }

    for ( int i = 0, isz = coords.size(); i < isz; ++i ) {
      const GeoPoint& gp = coords.at(i);

      bool ok;
      float value = l->field()->pointValue(gp, &ok);
     // debug_log<<gp.toString();
      if ( false == ok ) { continue; }

      TMeteoData& md = vec[i];
      md.set(l->descriptor(),TMeteoParam("",value,control::RIGHT));
    }
  }
  return vec;
}

//int32_t LayerProfile::dataSize() const
//{
//  int32_t size = Layer::dataSize();

//  int32_t sz = 0;
//  QByteArray ba;

//  sz = input_.size();
//  size += sizeof(sz);

//  for ( int i=0,isz=input_.size(); i<isz; ++i ) {
//    ba.clear();
//    input_[i] >> ba;
//    sz = ba.size();
//    size += sizeof(sz);
//    size += sz;
//  }

//  size += sizeof(sz);
//  size += sizeof(double_t) * xVals_.size();

//  size += sizeof(sz);
//  size += sizeof(double_t) * yVals_.size();

//  ba.clear();
//  ba = graphUuid_.toUtf8();
//  sz = ba.size();
//  size += sizeof(sz);
//  size += sz;

//  ba.clear();
//  ba = puansonUuids_.join(",").toUtf8();
//  sz = ba.size();
//  size += sizeof(sz);
//  size += sz;

//  std::string str;
//  prop_.SerializePartialToString(&str);
//  sz = str.size();
//  size += sizeof(sz);
//  size += sz;

//  puanson_.SerializePartialToString(&str);
//  sz = str.size();
//  size += sizeof(sz);
//  size += sz;

//  size += sizeof(int32_t) * 4; // mapRect_

//  size += x_.dataSize();
//  size += y_.dataSize();

//  return size;
//}

//int32_t LayerProfile::serializeToArray(char* data) const
//{
//  int32_t pos = Layer::serializeToArray(data);

//  int32_t sz = 0;
//  QByteArray ba;

//  sz = input_.size();
//  ::memcpy( data + pos, &sz, sizeof(sz) );
//  pos += sizeof(sz);

//  for ( int i=0,isz=input_.size(); i<isz; ++i ) {
//    ba.clear();
//    input_[i] >> ba;
//    sz = ba.size();
//    ::memcpy( data + pos, &sz, sizeof(sz) );
//    pos += sizeof(sz);
//    ::memcpy( data + pos, ba.data(), sz );
//    pos += sz;
//  }

//  sz = xVals_.size();
//  ::memcpy( data + pos, &sz, sizeof(sz) );
//  pos += sizeof(sz);

//  for ( int i=0,isz=xVals_.size(); i<isz; ++i ) {
//    double_t d = xVals_[i];
//    ::memcpy( data + pos, &d, sizeof(d) );
//    pos += sizeof(d);
//  }

//  sz = yVals_.size();
//  ::memcpy( data + pos, &sz, sizeof(sz) );
//  pos += sizeof(sz);

//  for ( int i=0,isz=yVals_.size(); i<isz; ++i ) {
//    double_t d = yVals_[i];
//    ::memcpy( data + pos, &d, sizeof(d) );
//    pos += sizeof(d);
//  }

//  ba.clear();
//  ba = graphUuid_.toUtf8();
//  sz = ba.size();
//  ::memcpy( data + pos, &sz, sizeof(sz) );
//  pos += sizeof(sz);
//  ::memcpy( data + pos, ba.data(), sz );
//  pos += sz;

//  ba.clear();
//  ba = puansonUuids_.join(",").toUtf8();
//  sz = ba.size();
//  ::memcpy( data + pos, &sz, sizeof(sz) );
//  pos += sizeof(sz);
//  ::memcpy( data + pos, ba.data(), sz );
//  pos += sz;

//  std::string str;
//  prop_.SerializePartialToString(&str);
//  sz = str.size();
//  ::memcpy( data + pos, &sz, sizeof(sz) );
//  pos += sizeof(sz);
//  ::memcpy( data + pos, str.data(), sz );
//  pos += sz;

//  puanson_.SerializePartialToString(&str);
//  sz = str.size();
//  ::memcpy( data + pos, &sz, sizeof(sz) );
//  pos += sizeof(sz);
//  ::memcpy( data + pos, str.data(), sz );
//  pos += sz;

//  int32_t r = 0;
//  r = mapRect_.left();
//  ::memcpy( data + pos, &r, sizeof(r) );
//  pos += sizeof(r);
//  r = mapRect_.top();
//  ::memcpy( data + pos, &r, sizeof(r) );
//  pos += sizeof(r);
//  r = mapRect_.right();
//  ::memcpy( data + pos, &r, sizeof(r) );
//  pos += sizeof(r);
//  r = mapRect_.bottom();
//  ::memcpy( data + pos, &r, sizeof(r) );
//  pos += sizeof(r);

//  pos += x_.serializeToArray(data + pos);
//  pos += y_.serializeToArray(data + pos);

//  return pos;
//}

//int32_t LayerProfile::parseFromArray(const char* data)
//{
//  int32_t pos = Layer::parseFromArray(data);

//  int32_t sz = 0;

//  input_.clear();
//  global::fromByteArray( data + pos, &sz );
//  pos += sizeof(sz);

//  for ( int i=0,isz=sz; i<isz; ++i ) {
//    global::fromByteArray( data + pos, &sz );
//    pos += sizeof(sz);

//    TMeteoData md;
//    md << QByteArray::fromRawData( data + pos, sz );
//    input_ << md;
//    pos += sz;
//  }

//  xVals_.clear();
//  global::fromByteArray( data + pos, &sz );
//  pos += sizeof(sz);

//  xVals_.resize(sz);
//  for ( int i=0,isz=xVals_.size(); i<isz; ++i ) {
//    double_t d = 0;
//    global::fromByteArray( data + pos, &d );
//    pos += sizeof(d);
//    xVals_[i] = d;
//  }

//  yVals_.clear();
//  global::fromByteArray( data + pos, &sz );
//  pos += sizeof(sz);

//  yVals_.resize(sz);
//  for ( int i=0,isz=yVals_.size(); i<isz; ++i ) {
//    double_t d = 0;
//    global::fromByteArray( data + pos, &d );
//    pos += sizeof(d);
//    yVals_[i] = d;
//  }

//  global::fromByteArray( data + pos, &sz );
//  pos += sizeof(sz);
//  graphUuid_ = QString::fromUtf8( data + pos, sz );
//  pos += sz;

//  global::fromByteArray( data + pos, &sz );
//  pos += sizeof(sz);
//  puansonUuids_ = QString::fromUtf8( data + pos, sz ).split(",");
//  pos += sz;

//  global::fromByteArray( data + pos, &sz );
//  pos += sizeof(sz);
//  prop_.ParsePartialFromArray( data + pos, sz );
//  pos += sz;

//  global::fromByteArray( data + pos, &sz );
//  pos += sizeof(sz);
//  puanson_.ParsePartialFromArray( data + pos, sz );
//  pos += sz;

//  int32_t r = 0;
//  global::fromByteArray( data + pos, &r );
//  pos += sizeof(r);
//  mapRect_.setLeft(r);
//  global::fromByteArray( data + pos, &r );
//  pos += sizeof(r);
//  mapRect_.setTop(r);
//  global::fromByteArray( data + pos, &r );
//  pos += sizeof(r);
//  mapRect_.setRight(r);
//  global::fromByteArray( data + pos, &r );
//  pos += sizeof(r);
//  mapRect_.setBottom(r);

//  pos += x_.parseFromArray(data + pos);
//  pos += y_.parseFromArray(data + pos);

//  return pos;
//}

} // map
} // meteo
