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

  if ( -1 == headerSize_ ) { return -9999; }

  PosGrid* proj = projection();

  QPoint p;
  if ( !proj->F2X_one(gp, &p) ) {
    return -9999;
  }
  p += cnt_;

  if ( !orig_.valid(p) ) {
    return -9999;
  }

  QFile file(fileName_);
  if ( !file.open(QFile::ReadOnly) ) {
    error_log << QObject::tr("Не удалось открыть файл %1.").arg(file.fileName())
              << file.errorString();
    return -9999;
  }
  QDataStream ds(&file);
  ds.skipRawData(headerSize_);

  int idx = p.y()*header_.samples + p.x();
  int offset = idx*8; //не знаю как победить с ходу

  ds.skipRawData(offset);

  if ( nullptr != ok ) { *ok = true; }
  double res = readValue(&ds,protoData().type());
  if(qFuzzyCompare(-9999., res)|| qIsInf(res) ||qIsNaN(res) ){
    if ( nullptr != ok ) { *ok = false; }
  }

  return res;
}


bool GeoRastr::load(const QString& fileName)
{
  if(0 != protoData_.format().compare("bin")){
    return loadImg(fileName);
  }
  SatLayer* l = maplayer_cast<SatLayer*>(layer());

  if ( nullptr == l ) {
    error_log << QObject::tr("Слой не создан");
    return false;
  }

  QFile file(fileName);
  if ( !file.open(QFile::ReadOnly) ) {
    error_log << QObject::tr("Не удалось открыть файл %1.").arg(file.fileName())
              << file.errorString();
    return false;
  }
  QDataStream ds(&file);

  fileName_ = fileName;

  ds >> header_;

  headerSize_ = ds.device()->pos();
  cnt_ = QPoint(header_.samples*0.5, header_.lines*0.5);
  return loadImg(fileName);


  const TColorGradList palette = l->palette();
  if(0 == palette.size()){
    file.close();
    return true;
  }
  QImage img(QSize(header_.samples,header_.lines), QImage::Format_ARGB32);

  QRgb* pData = reinterpret_cast<QRgb*>(img.bits());

  auto type = protoData().type();

  switch (type) {
//    case ::mappi::conf::kIndex8:

//      QRgb colors[256];
//      for ( int i = 0; i < 256; ++i ) {
//        colors[i] = palette.color(i/100.).rgba();
//      }

//      for ( int i = 0, isz = header_.lines * header_.samples; i < isz; ++i ) {
//        pData[i] = colors[int(readValue(&ds,type))];
//      }
//    break;
    case ::mappi::conf::kGrayScale:
    {
      img = img.convertToFormat(QImage::Format_Grayscale8);
      uint8_t *bData = img.bits();
      float min = 0;
      float nMin = header_.min;
      float max = 255;
      float nMax = header_.max;
      float k =  float(max - min)/float(nMax - nMin);
      for ( int i = 0, isz = img.size().height() *img.size().width(); i < isz; ++i ) {
        //pData[i] = colors[int(readValue(&ds,type))];
        bData[i] = uint8_t(k * float(readValue(&ds,type) - min) + float(nMin));
      }
    }
      orig_ = img.convertToFormat(QImage::Format_ARGB32);
    break;
    default:
      for ( int i = 0, isz = header_.lines * header_.samples; i < isz; ++i ) {
        pData[i] = palette.color(readValue(&ds,type)).rgba();
      }
      orig_ = img;
    break;
  }


  file.close();

  return true;
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
  //orig_ = QImage();

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
