#include "georastr.h"

#include <qdatastream.h>
#include <qelapsedtimer.h>
#include <qpainter.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/satelliteimage.pb.h>
#include <mappi/ui/satlayer/satlayer.h>


namespace meteo {
namespace map {

GeoRastr::GeoRastr(map::Layer *layer)
  : map::SatelliteImage(layer)
{
}

GeoRastr::GeoRastr(meteo::Projection *projection)
  : map::SatelliteImage(projection)
{
}

GeoRastr::GeoRastr(map::Object* parent)
  : map::SatelliteImage(parent)
{
}

GeoRastr::~GeoRastr()
{
}

map::Object *GeoRastr::copy(map::Layer *l) const
{
  if ( 0 == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  GeoRastr* si = new GeoRastr(l);
  foreach(Object* obj, objects_) {
    obj->copy(si);
  }
  return si;
}

map::Object *GeoRastr::copy(map::Object *o) const
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  GeoRastr* si = new GeoRastr(o);
  foreach(Object* obj, objects_) {
    obj->copy(si);
  }
  return si;
}

map::Object *GeoRastr::copy(meteo::Projection *proj) const
{
  if ( 0 == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return 0;
  }
  GeoRastr* si = new GeoRastr(proj);
  foreach(Object* obj, objects_) {
    obj->copy(si);
  }
  return si;
}

QString GeoRastr::getPretName()const{
  return header_.short_name;
}

QString GeoRastr::getPretUnit()const{
  return header_.unit_name;
}


double GeoRastr::coordsValue(const meteo::GeoPoint &gp, bool* ok) const
{
  if ( nullptr != ok ) { *ok = false; }
  if ( -1 == headerSize_ ){
    error_log << QObject::tr("Отсуствует headerSize");
    return -9999;
  }

  PosGrid* proj = projection();

  QPoint p;
  if ( !proj->F2X_one(gp, &p) ){
    error_log << QObject::tr("Невозможно спроецировать gp: ") << gp;
    return -9999;
  }
  p.setX(p.x() + orig_.width()/2);
  p.setY(p.y() + orig_.height()/2);

  if (!orig_.valid(p) ){
    error_log << QObject::tr("Точка отсутствует на изображении: ") << p;
    return -9999;
  }

  p.setX(p.x() * static_cast<float>(header_.samples)/orig_.width());
  p.setY(p.y() * static_cast<float>(header_.lines)/orig_.height());

  QFile file(toFileName_);
  if ( !file.open(QFile::ReadOnly) ) {
    error_log << QObject::tr("Не удалось открыть файл %1.").arg(file.fileName()) << file.errorString();
    return -9999;
  }
  QDataStream ds(&file);
  ds.skipRawData(headerSize_);

  int idx = p.y() * header_.samples + p.x();
  int offset = idx * sizeof(float);
  ds.skipRawData(offset);

  float res;
  ds >> res;
  if ( nullptr != ok ) { *ok = true; }
  return res;
}


bool GeoRastr::load(const QString& fileName)
{
  if(!fileName.endsWith(".to")) return loadImg(fileName);

  toFileName_ = fileName;
  SatLayer* l = maplayer_cast<SatLayer*>(layer());

  if ( nullptr == l ) {
    error_log << QObject::tr("Слой не создан");
    return false;
  }

  QFile file(toFileName_);
  if ( !file.open(QFile::ReadOnly) ) {
    error_log << QObject::tr("Не удалось открыть файл %1.").arg(file.fileName())<< file.errorString();
    return false;
  }
  QDataStream ds(&file);
  ds >> header_;
  headerSize_ = ds.device()->pos();
  cnt_ = QPoint(header_.samples*0.5, header_.lines*0.5);
  file.close();
  return loadImg(fileName);
}

void GeoRastr::buildCache()
{
  if ( orig_.isNull() ) {
    load(fileName_);
  }

  SatelliteImage::buildCache();
}

void GeoRastr::resetCache()
{
  orig_ = QImage();
  SatelliteImage::resetCache();
}

double GeoRastr::readValue(QDataStream* ds, int type) const
{
  switch ( type ) {
    default:
      double v8;
      (*ds) >> v8;
      return v8;
  }

  return -9999;
}

} // map
} // meteo
