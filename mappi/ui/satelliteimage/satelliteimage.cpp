#include "satelliteimage.h"

#include <qelapsedtimer.h>
#include <qpainter.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/ui/satlayer/satlayer.h>

#include <unordered_set>

namespace meteo {
namespace map {

SatelliteImage::SatelliteImage(meteo::map::Layer *layer):
  GeoPixmap(layer)
{
}

SatelliteImage::SatelliteImage(meteo::Projection *projection):
  GeoPixmap(projection)
{
}

SatelliteImage::SatelliteImage(meteo::map::Object* parent):
  GeoPixmap(parent)
{
}

SatelliteImage::~SatelliteImage()
{
}

meteo::map::Object *SatelliteImage::copy(meteo::map::Layer *l) const
{
  if ( 0 == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  SatelliteImage* si = new SatelliteImage(l);
  foreach(Object* o, objects_) {
    o->copy(si);
  }
  return si;
}

meteo::map::Object *SatelliteImage::copy(meteo::map::Object *o) const
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  SatelliteImage* si = new SatelliteImage(o);
  foreach(Object* obj, objects_){
    obj->copy(si);
  }
  return si;
}

meteo::map::Object *SatelliteImage::copy(meteo::Projection *proj) const
{
  if ( 0 == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return 0;
  }
  SatelliteImage* si = new SatelliteImage(proj);
  foreach(Object* obj, objects_){
    obj->copy(si);
  }
  return si;
}



//QList<meteo::GeoVector> SatelliteImage::skeletInRect(const QRect &rect, const QTransform &transform) const
//{
//  Q_UNUSED(rect);
//  Q_UNUSED(transform);
//  QList<meteo::GeoVector> list;
//  return list;
//}


//bool SatelliteImage::render(QPainter *painter, const QRect &docRect, const QTransform &transform)
//{
//  Q_UNUSED(docRect);
//  Q_UNUSED(transform);

//  if( false == visible() ) {
//    return false;
//  }
//  if ( cache_.isNull() ) {
//    buildCache();
//  }

//  debug_log << transform.m11()<< transform.m12()<< transform.m13();
//  debug_log << transform.m21()<< transform.m22()<< transform.m23();
//  debug_log << transform.m31()<< transform.m32()<< transform.m33();

//  QList<QRect> list = boundingRect(transform);
//  QRect boundRect;
//  if ( list.isEmpty() ) {
//    error_log << "Не удалось получить ограничивающий прямоугольник";
//    return false;
//  }

//  boundRect = list.at(0);

//  QRect cacheRect = cache_.rect();
//  cacheRect.moveTopLeft(QPoint(0,0));

//  QPolygon source = QPolygon(cacheRect);
//  QPolygon target = QPolygon(boundRect);

//  QTransform matrix;
//  QTransform::quadToQuad(source, target, matrix);
//  QRect visBoundRect = docRect.intersected(boundRect);

//  QTransform matrixInv = matrix.inverted();
//  // получаем rect той части кэш-изображения, которая видна на экране
//  QRect visCacheRect = matrixInv.mapRect(visBoundRect);

//  if ( !visCacheRect.isValid() ) {
//    return false;
//  }

//  const QImage &img2 = cache_.copy(visCacheRect);

//  QImage img(img2.transformed(matrix));

//  uchar* pixels = img.bits();
//   for ( int i = 0, isz = 4 * (img.width() * img.height()); i < isz; i += 4 ) {
//   //прозрачность
//     if ( 0 != pixels[i+3 ] ) {
//       pixels[i+3] = transparency_;
//     }
//   //контраст
//     int redContrast = ((pixels[i] - 127) * (contrast_)/100) + 127;
//     pixels[i] = qBound(0, redContrast, 255);
//     int greenContrast = ((pixels[i+1] - 127) * (contrast_)/100) + 127;
//     pixels[i+1] = qBound(0, greenContrast, 255);
//     int blueContrast = ((pixels[i+2] - 127) * (contrast_)/100) + 127;
//     pixels[i+2] = qBound(0, blueContrast, 255);
//   //яркость
//     int redBright = pixels[i] + ((brightness_)*255/100);
//     pixels[i] = qBound(0, redBright, 255);
//     int greenBright = pixels[i+1] + ((brightness_)*255/100);
//     pixels[i+1] = qBound(0, greenBright, 255);
//     int blueBright = pixels[i+2] + ((brightness_)*255/100);
//     pixels[i+2] = qBound(0, blueBright, 255);
//   //маскирование
//     if ( blackCoeff_ >= 0 || whiteCoeff_ <= 255 ) {
//       int gray = qGray(*(pixels+i), *(pixels+i+1), *(pixels+i+2));
//       if ( gray < blackCoeff_ || gray > whiteCoeff_) {
//         pixels[i+3] = 0;
//       }
//     }
//   }

//  painter->drawImage(visBoundRect.topLeft(),img);
//  return true;
//}

int SatelliteImage::minimumScreenDistance(const QPoint &pos, QPoint *cross) const
{
  Q_UNUSED(pos);
  Q_UNUSED(cross);
  int dist = 10000000;
  return dist;
}

QList<QRect> SatelliteImage::boundingRect(const QTransform &transform) const
{
  Q_UNUSED(transform);

  meteo::map::Document* doc = document();

  QPoint min = doc->coord2screen(topLeft_);
  QPoint max = doc->coord2screen(bottomRight_);

  return { QRect(min,max) };
}

double SatelliteImage::coordsValue(const meteo::GeoPoint &gp, bool* ok) const
{
  Q_UNUSED( gp );

  if ( 0 != ok ) { *ok = false; }

  return -9999;
}

bool SatelliteImage::loadImg(const QString& filename)
{
  QString afileName = filename;
  afileName.replace(".to", ".png"); //TODO пока так
  if ( !orig_.load(afileName) ) {
    error_msg << QObject::tr("Не удалось загрузить изображение %1").arg(filename);
    return false;
  }
  orig_ = orig_.convertToFormat(QImage::Format_ARGB32);
  buildCache();
  return true;
}

bool SatelliteImage::load(const QString& filename)
{
  return loadImg(filename);
}


void SatelliteImage::fixImagePosition( )
{


  //setMarkersPinned(true);
}


void SatelliteImage::buildCache()
{
  QElapsedTimer t;
  t.start();

  PosGrid* grid = projection();
  if ( nullptr == grid ) {
    debug_log << QObject::tr("Отсутствует проекция");
    return;
  }

  map::proto::Document docParam = grid->document()->property();
  *docParam.mutable_doc_center() = meteo::geopoint2pbgeopoint( document()->center() );
  *docParam.mutable_map_center() = meteo::geopoint2pbgeopoint( document()->projection()->getMapCenter() );

  map::Document tmpdoc;
  if ( !tmpdoc.init(docParam) ) {
    return;
  }

  //QRect screenBoundRect = calcBoundingRect(document());
  QRect screenBoundRect = calcBoundingRect(document());
  GeoVector gv = document()->screen2coord(QPolygon(screenBoundRect));

  topLeft_ = gv.value(0, GeoPoint());
  bottomRight_ = gv.value(2, GeoPoint());

  const QRect cacheBoundRect = calcBoundingRect(&tmpdoc);
  if (cacheBoundRect.width() > 6000 || cacheBoundRect.height() > 6000) { //TODO если сеанс не на видимой части документа, будет ошибка выделения памяти
    error_log << QObject::tr("Ошибка размера рамки");
    return;
  }

  const int width  = orig_.width();
  const int height = orig_.height();

  const int width_2  = width*0.5;
  const int height_2 = height*0.5;

  const int offsetX = cacheBoundRect.left();
  const int offsetY = cacheBoundRect.top();

  cache_ = QImage(cacheBoundRect.size(), QImage::Format_ARGB32);
  cache_.fill(QColor(0,0,0,0));

  const int kNoData = std::numeric_limits<int>::max();
  const int cachePixelCount = cache_.width() * cache_.height();

  QVector<int> xVec;
  QVector<int> yVec;
  QVector<QPoint> coords;

  xVec.fill(kNoData, cachePixelCount);
  yVec.fill(kNoData, cachePixelCount);
  coords.resize(cachePixelCount);

  int* pX = xVec.data();
  int* pY = yVec.data();
  QPoint* pCoords = coords.data();

  for ( int j = 0, jsz = cacheBoundRect.height(), isz = cacheBoundRect.width(); j < jsz; ++j ) {
    for ( int i = 0; i < isz; ++i ) {
      *(pCoords + j*isz + i) = QPoint(i+offsetX,j+offsetY);
    }
  }

  coords = tmpdoc.screenToCartesian(coords);
  grid->cartesian2X(coords, tmpdoc.projection()->getMapCenter(), pX, pY);

  QRgb* orig = reinterpret_cast<QRgb*>(orig_.bits());
  QRgb* pixel = reinterpret_cast<QRgb*>(cache_.bits());

  for ( int j = 0, hg = cache_.height(), wd = cache_.width(); j < hg; ++j ) {
    int row = j*wd;
    for ( int i = 0; i < wd; ++i ) {
      int idx = row + i;
      int x = *(pX + idx);
      int y = *(pY + idx);
      if ( kNoData == x ) {
        continue;
      }
      x += width_2;
      y += height_2;
      if ( x < 0 || x >= width || y < 0 || y >= height ) {
        continue;
      }
      *(pixel+idx) = *(orig + y*width + x);
    }
  }

  QPair< QPolygon, meteo::GeoVector > bunch;
//  bunch.first << QPoint(0,0)<<QPoint(0,width)<<QPoint(height,width);
  bunch.first <<QPoint(height,width)<<QPoint(0,width)<< QPoint(0,0);
  //bool SatViewPoint::countGridCorners(float maxAngle, meteo::GeoPoint* gp ) const {
  bunch.second.resize(4);
  if(grid->countGridCorners(bunch.second.data())) {

  }

  bunch.second.resize(3);

 // bunch.second << topLeft_ << gv.at(1) << bottomRight_;
  debug_log<< bunch.first << bunch.second ;
  setImage(cache_);
  bindScreenToCoord( bunch.first, bunch.second );
//orig_.save("/home/gotur/1.png","PNG");
}

void SatelliteImage::resetCache()
{
  cache_ = QImage();
}

void SatelliteImage::setProtoData(const ::mappi::proto::SatelliteImage& data)
{
  protoData_.CopyFrom(data);
}

void SatelliteImage::setColorToHide(int black, int white)
{
  if ( false == locked_ ) {
    blackCoeff_ = black; whiteCoeff_ = white;
  }
}

void SatelliteImage::setBrightness(int brightness)
{
  if ( false == locked_ ) {
    brightness_ = brightness;
  }
}

void SatelliteImage::setContrast(int contrast)
{
  if ( false == locked_) {
    contrast_ = contrast;
  }
}

void SatelliteImage::setTransparency(int transparency)
{
  if ( false == locked_ ) {
    transparency_ = transparency;
  }
}

void SatelliteImage::setLocked(bool locked)
{
  locked_ = locked;
  document()->eventHandler()->notifyLayerChanges(layer_, LayerEvent::ObjectChanged);
}

PosGrid* SatelliteImage::projection() const
{
  SatLayer* l = maplayer_cast<SatLayer*>(layer());

  if ( nullptr == l ) { return nullptr; }

  return l->projection();
}

QRect SatelliteImage::calcBoundingRect(Document* doc) const
{
  PosGrid* grid = projection();
  if ( nullptr == grid ) {
    debug_log << QObject::tr("Отсутствует проекция");
    return QRect();
  }

  int hw = orig_.width()*0.5;
  int hh = orig_.height()*0.5;

  int maxX = -99999;
  int maxY = -99999;
  int minX = 99999;
  int minY = 99999;

  for ( auto j : {-hh, hh} ) {
    for ( int i = -hw; i < hw; ++i ) {
      GeoPoint gp;
      bool res = grid->X2F_one(QPoint(i,j), &gp);
      if ( false == res ) {
        continue;
      }
      QPoint mapPoint = doc->coord2screen(gp, &res);
      if ( false == res ) {
        continue;
      }
      minX = qMin(minX, mapPoint.x());
      maxX = qMax(maxX, mapPoint.x());
      minY = qMin(minY, mapPoint.y());
      maxY = qMax(maxY, mapPoint.y());
    }
  }

  for ( auto i : {-hw, hw} ) {
    for ( int j = -hh; j < hh; ++j ) {
      GeoPoint gp;
      bool res = grid->X2F_one(QPoint(i,j), &gp);
      if ( false == res ) {
        continue;
      }
      QPoint mapPoint = doc->coord2screen(gp, &res);
      if ( false == res ) {
        continue;
      }
      minX = qMin(minX, mapPoint.x());
      maxX = qMax(maxX, mapPoint.x());
      minY = qMin(minY, mapPoint.y());
      maxY = qMax(maxY, mapPoint.y());
    }
  }

  return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

} // map
} // meteo
