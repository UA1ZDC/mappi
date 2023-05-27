#include "layergraph.h"

#include <quuid.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/generalproj.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/puanson.h>

#include <commons/geom/geom.h>

#include "axisgraph.h"

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerGraph(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerGraph::Type, createLayer );
}

LayerGraph::LayerGraph(Document* d, const QString& n )
  : Layer(d)
{
  setName(n);

  if ( 0 != document() ) {
    Projection* proj = document_->projection();
    const GeoVector& rmk = proj->ramka();

    QPolygonF poly;
    for ( int i=0,isz=rmk.size(); i<isz; ++i ) {
      poly << QPointF(rmk[i].lat(),rmk[i].lon());
    }
    mapRect_ = poly.boundingRect().toRect();
    x_.setMapSize(mapRect_.size());
    x_.setOffset(mapRect_.left());
    y_.setMapSize(mapRect_.size());
    y_.setOffset(mapRect_.top());
  }
}

LayerGraph::~LayerGraph()
{
}

void LayerGraph::setData(const QList<TMeteoData>& data, const QString& xDescr, const QString& yDescr, bool recalcAxisRange)
{
  input_ = data;
  xVals_ = getValues(data, xDescr);
  yVals_ = getValues(data, yDescr);

  update(recalcAxisRange);
}

void LayerGraph::setGraphProperty(const meteo::Property& prop)
{
  prop_ = prop;

  Object* obj = objectByUuid(graphUuid_);
  if ( 0 != obj ) {
    obj->setProperty(prop);
  }

  meteo::Property xProp = x_.property();
  xProp.mutable_pen()->set_color(prop_.pen().color());
  x_.setProperty(xProp);

  meteo::Property yProp = y_.property();
  yProp.mutable_pen()->set_color(prop_.pen().color());
  y_.setProperty(yProp);
}

GeoVector LayerGraph::graphCoords() const
{
  Object* obj = objectByUuid(graphUuid_);
  if ( 0 != obj ) {
    return obj->skelet();
  }

  return GeoVector();
}

int32_t LayerGraph::dataSize() const
{
  int32_t size = Layer::dataSize();

  int32_t sz = 0;
  QByteArray ba;

  sz = input_.size();
  size += sizeof(sz);

  for ( int i=0,isz=input_.size(); i<isz; ++i ) {
    ba.clear();
    input_[i] >> ba;
    sz = ba.size();
    size += sizeof(sz);
    size += sz;
  }

  size += sizeof(sz);
  size += sizeof(double_t) * xVals_.size();

  size += sizeof(sz);
  size += sizeof(double_t) * yVals_.size();

  ba.clear();
  ba = graphUuid_.toUtf8();
  sz = ba.size();
  size += sizeof(sz);
  size += sz;

  ba.clear();
  ba = puansonUuids_.join(",").toUtf8();
  sz = ba.size();
  size += sizeof(sz);
  size += sz;

  std::string str;
  prop_.SerializePartialToString(&str);
  sz = str.size();
  size += sizeof(sz);
  size += sz;

  puanson_.SerializePartialToString(&str);
  sz = str.size();
  size += sizeof(sz);
  size += sz;

  size += sizeof(int32_t) * 4; // mapRect_

  size += x_.dataSize();
  size += y_.dataSize();

  return size;
}

int32_t LayerGraph::serializeToArray(char* data) const
{
  int32_t pos = Layer::serializeToArray(data);

  int32_t sz = 0;
  QByteArray ba;

  sz = input_.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);

  for ( int i=0,isz=input_.size(); i<isz; ++i ) {
    ba.clear();
    input_[i] >> ba;
    sz = ba.size();
    ::memcpy( data + pos, &sz, sizeof(sz) );
    pos += sizeof(sz);
    ::memcpy( data + pos, ba.data(), sz );
    pos += sz;
  }

  sz = xVals_.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);

  for ( int i=0,isz=xVals_.size(); i<isz; ++i ) {
    double_t d = xVals_[i];
    ::memcpy( data + pos, &d, sizeof(d) );
    pos += sizeof(d);
  }

  sz = yVals_.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);

  for ( int i=0,isz=yVals_.size(); i<isz; ++i ) {
    double_t d = yVals_[i];
    ::memcpy( data + pos, &d, sizeof(d) );
    pos += sizeof(d);
  }

  ba.clear();
  ba = graphUuid_.toUtf8();
  sz = ba.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( data + pos, ba.data(), sz );
  pos += sz;

  ba.clear();
  ba = puansonUuids_.join(",").toUtf8();
  sz = ba.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( data + pos, ba.data(), sz );
  pos += sz;

  std::string str;
  prop_.SerializePartialToString(&str);
  sz = str.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( data + pos, str.data(), sz );
  pos += sz;

  puanson_.SerializePartialToString(&str);
  sz = str.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( data + pos, str.data(), sz );
  pos += sz;

  int32_t r = 0;
  r = mapRect_.left();
  ::memcpy( data + pos, &r, sizeof(r) );
  pos += sizeof(r);
  r = mapRect_.top();
  ::memcpy( data + pos, &r, sizeof(r) );
  pos += sizeof(r);
  r = mapRect_.right();
  ::memcpy( data + pos, &r, sizeof(r) );
  pos += sizeof(r);
  r = mapRect_.bottom();
  ::memcpy( data + pos, &r, sizeof(r) );
  pos += sizeof(r);

  pos += x_.serializeToArray(data + pos);
  pos += y_.serializeToArray(data + pos);

  return pos;
}

int32_t LayerGraph::parseFromArray(const char* data)
{
  int32_t pos = Layer::parseFromArray(data);

  int32_t sz = 0;

  input_.clear();
  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);

  for ( int i=0,isz=sz; i<isz; ++i ) {
    global::fromByteArray( data + pos, &sz );
    pos += sizeof(sz);

    TMeteoData md;
    md << QByteArray::fromRawData( data + pos, sz );
    input_ << md;
    pos += sz;
  }

  xVals_.clear();
  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);

  xVals_.resize(sz);
  for ( int i=0,isz=xVals_.size(); i<isz; ++i ) {
    double_t d = 0;
    global::fromByteArray( data + pos, &d );
    pos += sizeof(d);
    xVals_[i] = d;
  }

  yVals_.clear();
  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);

  yVals_.resize(sz);
  for ( int i=0,isz=yVals_.size(); i<isz; ++i ) {
    double_t d = 0;
    global::fromByteArray( data + pos, &d );
    pos += sizeof(d);
    yVals_[i] = d;
  }

  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);
  graphUuid_ = QString::fromUtf8( data + pos, sz );
  pos += sz;

  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);
  puansonUuids_ = QString::fromUtf8( data + pos, sz ).split(",");
  pos += sz;

  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);
  prop_.ParsePartialFromArray( data + pos, sz );
  pos += sz;

  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);
  puanson_.ParsePartialFromArray( data + pos, sz );
  pos += sz;

  int32_t r = 0;
  global::fromByteArray( data + pos, &r );
  pos += sizeof(r);
  mapRect_.setLeft(r);
  global::fromByteArray( data + pos, &r );
  pos += sizeof(r);
  mapRect_.setTop(r);
  global::fromByteArray( data + pos, &r );
  pos += sizeof(r);
  mapRect_.setRight(r);
  global::fromByteArray( data + pos, &r );
  pos += sizeof(r);
  mapRect_.setBottom(r);

  pos += x_.parseFromArray(data + pos);
  pos += y_.parseFromArray(data + pos);

  return pos;
}

GeoPoint LayerGraph::nearestByX(float x, bool* ok) const
{
  QPointF p = nearest(x_.coord2scale(x), xVals_, ok);
  return GeoPoint(x_.scale2coord(p.x()), y_.scale2coord(p.y()), 0, LA_GENERAL);
}

GeoPoint LayerGraph::nearestByY(float y, bool* ok) const
{
  QPointF p = nearest(y_.coord2scale(y), yVals_, ok);
  return GeoPoint(x_.scale2coord(p.x()), y_.scale2coord(p.y()), 0, LA_GENERAL);
}

QList<GeoPoint> LayerGraph::crossByX(double x) const
{
  Object* obj = objectByUuid(graphUuid_);
  if ( 0 == obj ) { return QList<GeoPoint>(); }


  QTransform tr;
  tr.scale(100000,100000);

  QLineF line(QPointF(x,mapRect_.top()),QPointF(x,mapRect_.bottom()));
  line = tr.map(line);

  QList<QPoint> crosses;

  QPointF p1;
  QPointF p2;
  GeoVector skelet = obj->skelet();
  for ( int i=1,isz=skelet.size(); i<isz; ++i ) {
    p1.rx() = skelet.at(i).lat();
    p1.ry() = skelet.at(i).lon();
    p2.rx() = skelet.at(i-1).lat();
    p2.ry() = skelet.at(i-1).lon();

    QLine l = tr.map(QLineF(p1,p2)).toLine();

    QPoint p;
    if ( meteo::geom::crossPoint(line.toLine(), l, &p) ) {
      // проверяем на повтор, чтобы исключить дубликаты когда точка пересечения располагается на границе двух секторов
      if ( crosses.isEmpty() || crosses.last() != p ) {
        crosses << p;
      }
    }
  }

  QList<GeoPoint> list;
  foreach ( const QPoint& p, crosses ) {
    p1.rx() = p.x();
    p1.ry() = p.y();

    p1 = tr.inverted().map(p1);
    list << GeoPoint(p1.x(), p1.y(), 0, LA_GENERAL);
  }
  return list;
}

QList<GeoPoint> LayerGraph::crossByY(double y) const
{
  Object* obj = objectByUuid(graphUuid_);
  if ( 0 == obj ) { return QList<GeoPoint>(); }


  QTransform tr;
  tr.scale(100000,100000);

  QLineF line(QPointF(mapRect_.left(),y),QPointF(mapRect_.right(),y));
  line = tr.map(line);

  QList<QPoint> crosses;

  QPointF p1;
  QPointF p2;
  GeoVector skelet = obj->skelet();
  for ( int i=1,isz=skelet.size(); i<isz; ++i ) {
    p1.rx() = skelet.at(i).lat();
    p1.ry() = skelet.at(i).lon();
    p2.rx() = skelet.at(i-1).lat();
    p2.ry() = skelet.at(i-1).lon();

    QLine l = tr.map(QLineF(p1,p2)).toLine();

    QPoint p;
    if ( meteo::geom::crossPoint(line.toLine(), l, &p) ) {
      // проверяем на повтор, чтобы исключить дубликаты когда точка пересечения располагается на границе двух секторов
      if ( crosses.isEmpty() || crosses.last() != p ) {
        crosses << p;
      }
    }
  }

  QList<GeoPoint> list;
  foreach ( const QPoint& p, crosses ) {
    p1.rx() = p.x();
    p1.ry() = p.y();

    p1 = tr.inverted().map(p1);
    list << GeoPoint(p1.x(), p1.y(), 0, LA_GENERAL);
  }
  return list;
}

void LayerGraph::update(bool recalcAxisRange)
{
  delete objectByUuid(graphUuid_);

  foreach ( const QString& uuid, puansonUuids_ ) {
    delete objectByUuid(uuid);
  }

  if ( xVals_.isEmpty() ) { return; }

  if ( recalcAxisRange ) {
    calcAxisRange();
  }

  bool addPuanson = puanson_.IsInitialized();

  GeoVector skelet;
  for ( int i=0,isz=xVals_.size(); i<isz; ++i ) {
    float x = xVals_.at(i);
    float y = yVals_.at(i);

    if ( std::isnan(x) || std::isnan(y) ) {
      continue;
    }

    GeoPoint gp(x_.scale2coord(x), y_.scale2coord(y), 0, LA_GENERAL);

    skelet << gp;

    if ( addPuanson ) {
      Puanson* p = new Puanson(this);
      p->setPunch(puanson_);
      p->setMeteodata(input_.value(i));
      p->setDrawAlways(true);
      p->setSkelet(gp);
      puansonUuids_ << p->uuid();
    }
  }

  map::GeoPolygon* line = new map::GeoPolygon(this);
  line->setProperty(prop_);
  line->setSkelet(skelet);

  graphUuid_ = line->uuid();
}

QStringList LayerGraph::puansonLabels(float x, float y) const
{
  QStringList list;

  if ( std::isnan(x) && std::isnan(y) ) { return list; }

  float coordX = x_.scale2coord(x);
  float coordY = y_.scale2coord(y);

  QList<Puanson*> punchList;
  foreach ( const QString& uuid, puansonUuids_ ) {
    Puanson* p = mapobject_cast<Puanson*>(objectByUuid(uuid));

    if ( nullptr == p || p->skelet().size() == 0 ) { continue; }

    if ( !std::isnan(coordX) && MnMath::isEqual(coordX, static_cast<float>(p->skelet().at(0).lat())) ) {
      punchList << p;
      continue;
    }

    if ( !std::isnan(coordY) && MnMath::isEqual(coordY,static_cast<float>( p->skelet().at(0).lon())) ) {
      punchList << p;
      continue;
    }
  }

  foreach ( const Puanson* p, punchList ) {
    bool ok = false;
    TMeteoParam mp = p->dd(&ok);
    if ( ok ) {
      list << QObject::tr("dd,°: %1").arg(mp.value(), 3, 'f', 0);
    }
    mp = p->ff(&ok);
    if ( ok ) {
      list << QObject::tr("ff,м/с: %1").arg(mp.value(), 3, 'f', 0);
    }
  }

  return list;
}

QVector<float> LayerGraph::getDtValues(const QList<TMeteoData>& mdList) const
{
  QVector<float> data(mdList.size());

  for ( int i=0,isz=data.size(); i<isz; ++i ) {
    data[i] = TMeteoDescriptor::instance()->dateTime(mdList.at(i)).toTime_t();
  }

  return data;
}

QVector<float> LayerGraph::getValues(const QList<TMeteoData>& mdList, const QString& descr) const
{
  if ( descr.isEmpty() ) {
    return getDtValues(mdList);
  }

  QVector<float> data(mdList.size());

  descr_t desc = TMeteoDescriptor::instance()->descriptor(descr);

  control::QualityControl control;
  float val = 0;

  for ( int i=0,isz=data.size(); i<isz; ++i ) {
    bool ok = mdList.at(i).getValue(desc, &val, &control);
    if ( !ok || control > control::DOUBTFUL ) {
      data[i] = NAN;
    }
    else {
      data[i] = val;
    }
  }

  return data;
}

QPointF LayerGraph::nearest(float value, const QVector<float>& values, bool* ok) const
{

  if ( values.size() == 0 ) {
    if ( 0 != ok ) { *ok = false; }
    return QPointF();
  }

  if ( 0 != ok ) { *ok = true; }

  int left  = values.size() - 1;
  int right = 0;

  float v;
  for ( int i=0,isz=values.size(); i<isz; ++i ) {
    v = values.at(i);

    if ( std::isnan(v) ) { continue; }

    if ( v > value ) {
      left = i - 1;
      break;
    }

    if ( MnMath::isEqual(value, v) ) {
      return QPointF(xVals_[i], yVals_[i]);
    }
  }

  for ( int i=values.size()-1; i>=0; --i ) {
    v = values.at(i);

    if ( std::isnan(v) ) { continue; }

    if ( v < value ) {
      right = i + 1;
      break;
    }

    if ( MnMath::isEqual(value, v) ) {
      return QPointF(xVals_[i], yVals_[i]);
    }
  }

  int l = qBound(0, left, values.size() - 1);
  int r = qBound(0, right, values.size() - 1);

  float lDist = qAbs(values[l] - value);
  float rDist = qAbs(values[r] - value);

  int idx = lDist < rDist ? l : r;

  return QPointF(xVals_[idx], yVals_[idx]);
}

void LayerGraph::calcAxisRange()
{
  float minX = minElement(xVals_);
  float maxX = maxElement(xVals_);
  if ( !std::isnan(minX) && !std::isnan(maxX) ) {
    x_.setRange(minX, maxX);
  }

  float minY = minElement(yVals_);
  float maxY = maxElement(yVals_);
  if ( !std::isnan(minY) && !std::isnan(maxY) ) {
    y_.setRange(minY, maxY);
  }
}

void LayerGraph::printLayerInfo() const
{
  debug_log << "name:" << name();
  debug_log << "graph UUID:" << graphUuid_;
  debug_log << "punch UUIDs:" << puansonUuids_.join(",");
  debug_log << "prop:" << prop_.Utf8DebugString();
  debug_log << "puan:" << puanson_.Utf8DebugString();
  debug_log << "axis X:\n"
            << var(x_.lower()) << var(x_.upper()) << "\n";
  debug_log << "axis Y:\n"
            << var(y_.lower()) << var(y_.upper()) << "\n";

}

float LayerGraph::minElement(const QVector<float>& v) const
{
  float min = std::numeric_limits<float>::max();
  int idx = -1;

  for ( int i=0,isz=v.size(); i<isz; ++i ) {
    if ( !std::isnan(v[i]) && min > v[i] ) {
      idx = i;
      min = v[i];
    }
  }

  return -1 == idx ? NAN : min;
}

float LayerGraph::maxElement(const QVector<float>& v) const
{
  float max = -std::numeric_limits<float>::max();
  int idx = -1;

  for ( int i=0,isz=v.size(); i<isz; ++i ) {
    if ( !std::isnan(v[i]) && max < v[i] ) {
      idx = i;
      max = v[i];
    }
  }

  return -1 == idx ? NAN : max;
}

} // map
} // meteo
