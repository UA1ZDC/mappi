#include "basiscache.h"

#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qapplication.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/map.h>

static const int32_t kCacheSize{256};

inline bool operator<( const QPoint& p1, const QPoint& p2 )
{
  if ( p1.x() < p2.x() ) {
    return true;
  }
  else if ( p1.x() == p2.x() && p1.y() < p2.y() ) {
    return true;
  }
  return false;
}

namespace meteo {
namespace map {

inline QPoint rectAddress( const QPoint& pnt )
{
  int xaddr = pnt.x()/kCacheSize;
  int yaddr = pnt.y()/kCacheSize;
  QPoint cpnt( xaddr, yaddr );

  return cpnt;
}

inline QPoint rectAddress( const QRect& r )
{
  return rectAddress( r.topLeft() );
}

static const QString kCachePath( MnCommon::varPath("meteo") + "/mapcache" );

QString projTypeName( int projtype )
{
  switch ( projtype ) {
    case kGeneral:
      return "general";
      break;
    case kMercat:
      return "merkat";
      break;
    case kConical:
      return "conich";
      break;
    case kStereo:
      return "stereo";
      break;
    default:
      return "unknown";
      break;
  }
  return "unknown";
}

int projType( const QString& projname )
{
  if ( "general" == projname ) {
    return kGeneral;
  }
  else if ( "merkat" == projname ) {
    return kMercat;
  }
  else if ( "conich" == projname ) {
    return kConical;
  }
  else if ( "stereo" == projname ) {
    return kStereo;
  }
  else if ( "unknown" == projname ) {
    return -1;
  }
  return -1;
}

QString projCachePath( const QString& loader, const QString& hash, ProjectionType proj )
{
  return kCachePath + "/" + loader + "/" + hash + "/" + projTypeName(proj);
}

QString projCachePath( const QString& loader, const QString& hash, const QString& proj )
{
  return kCachePath + "/" + loader + "/" + hash + "/" + proj;
}

QString projCachePath( const std::string& loader, const QString& hash, ProjectionType proj )
{
  return projCachePath( QString::fromStdString(loader), hash, proj );
}

QString projCachePath( const std::string& loader, const QString& hash, const QString& proj )
{
  return projCachePath( loader, hash, proj );
}

QString scaleCachePath( const QString& loader, const QString& hash, ProjectionType proj, double scale )
{
  return projCachePath( loader, hash, proj) + "/" + QString::number(scale);
}

QString scaleCachePath( const std::string& loader, const QString& hash, ProjectionType proj, double scale )
{
  return projCachePath( loader, hash, proj) + "/" + QString::number(scale);
}

inline QString BasisCache::rectFileName( Document* d, const QRect& r ) const
{
  QString strproj = QString::number( d->projection()->type() );
  QString strscale = QString::number( d->scale() );
  QString hash = d->cacheHash();

  QPoint rp = rectAddress(r);

  GeoPoint gp = d->projection()->getMapCenter();

  QString coordstr =QObject::tr("%1__%2__%3")
    .arg( gp.latDeg(), 0, 'f', 4, 0 )
    .arg( gp.lonDeg(), 0, 'f', 4, 0 )
    .arg( d->property().rot_angle(), 0, 'f', 4, 0 );

  QString raddr = QObject::tr("%1_%2")
      .arg( rp.x() )
      .arg( rp.y() );

  QString path = scaleCachePath( d->property().geoloader(), hash, d->property().projection(), d->scale() )
    + "/" + coordstr
    + "/" + raddr
    + ".png";
  return path;
}

BasisCache::BasisCache()
{
}

BasisCache::~BasisCache()
{
}

bool BasisCache::flushBasis( Document* d )
{
  QRect docrect = d->documentRect();
  QPoint tl = d->absolutetransform().map( docrect.topLeft() );
  QPoint br = tl + QPoint( docrect.width(), docrect.height() );
  int tlx = tl.x() - tl.x()%kCacheSize;
  if ( 0 > tl.x() ) {
    tlx -= kCacheSize;
  }
  int tly = tl.y() - tl.y()%kCacheSize;
  if ( 0 > tl.y() ) {
    tly -= kCacheSize;
  }
  while ( tlx < br.x() ) {
    int ltly = tly;
    while  ( ltly < br.y() ) {
      QRect r( QPoint(tlx, ltly), QSize(kCacheSize, kCacheSize) );
      if ( false == containsRect( d, r) ) {
        flushRect( d, r );
      }
      ltly += kCacheSize;
    }
    tlx += kCacheSize;
  }

  return false;
}

void BasisCache::flushRect( Document* d, const QRect& r )
{
  QFile file( rectFileName( d, r ) );
  if ( true == file.exists() ) {
    return;
  }
  QFileInfo fi(file);
  QDir dir;
  if ( false == dir.mkpath( fi.absolutePath() ) ) {
    error_log << QObject::tr("Не удалось создать директорию = %1")
      .arg( fi.absolutePath() );
    return;
  }

  QImage img( kCacheSize, kCacheSize, QImage::Format_ARGB32 );
  img.fill( Qt::transparent );
  QPainter pntr(&img);
  QStringList uuids = d->cacheUuids();
  QRect newrect = QRect(d->absolutetransform().inverted().map(r.topLeft()), r.size() );
  d->drawDocTile( &pntr, uuids, newrect );
  if ( true == file.exists() ) {
    return;
  }
  if ( false == file.open( QIODevice::WriteOnly ) ) {
    error_log << QObject::tr("Не удалось открыть файл для записи = %1")
      .arg( fi.absoluteFilePath() );
    return;
  }
  img.save( &file, "PNG" );
  file.flush();
  file.close();
}

bool BasisCache::renderBasis( Document* d, QPainter* p )
{
  QRect docrect = d->documentRect();
  QPoint tl = d->absolutetransform().map( docrect.topLeft() );
  QPoint br = tl + QPoint( docrect.width(), docrect.height() );
  int tlx = tl.x() - tl.x()%kCacheSize;
  if ( 0 > tl.x() ) {
    tlx -= kCacheSize;
  }
  int tly = tl.y() - tl.y()%kCacheSize;
  if ( 0 > tl.y() ) {
    tly -= kCacheSize;
  }
  while ( tlx < br.x() ) {
    int ltly = tly;
    while  ( ltly < br.y() ) {
      QRect r( QPoint(tlx, ltly), QSize(kCacheSize, kCacheSize) );
      if ( false == containsRect( d, r) ) {
        return false;
      }
      ltly += kCacheSize;
    }
    tlx += kCacheSize;
  }
  tlx = tl.x() - tl.x()%kCacheSize;
  if ( 0 > tl.x() ) {
    tlx -= kCacheSize;
  }
  tly = tl.y() - tl.y()%kCacheSize;
  if ( 0 > tl.y() ) {
    tly -= kCacheSize;
  }
  while ( tlx < br.x() ) {
    int ltly = tly;
    while  ( ltly < br.y() ) {
      QRect r( QPoint(tlx, ltly), QSize(kCacheSize, kCacheSize) );
      if ( false == renderRect( r, d, p ) ) {
        return false;
      }
      ltly += kCacheSize;
    }
    tlx += kCacheSize;
  }
  return true;
}

void BasisCache::loadGeoBasis( Document* doc )
{
  loadGeoLayers(doc);
}

bool BasisCache::hasTile( const proto::Document& doc, int x, int y ) const
{
  QString filename = tilePath( doc, x, y );
  if ( false == QFile::exists(filename) ) {
    return false;
  }
  return true;
}

QByteArray BasisCache::getTile( const proto::Document& doc, int x, int y ) const
{
  QString filename = tilePath( doc, x, y );
  QFile file(filename);
  if ( false == file.exists() ) {
    return QByteArray();
  }
  if ( false == file.open(QIODevice::ReadOnly) ) {
    return QByteArray();
  }
  return file.readAll();
}

inline QString BasisCache::tilePath( const proto::Document& doc, int x, int y ) const
{
  QPoint rp(x, y);

  GeoPoint gp = pbgeopoint2geopoint( doc.map_center() );

  QString coordstr = QObject::tr("%1__%2__%3")
    .arg( gp.latDeg(), 0, 'f', 4, 0 )
    .arg( gp.lonDeg(), 0, 'f', 4, 0 )
    .arg( doc.rot_angle(), 0, 'f', 4, 0 );

  QString raddr = QObject::tr("%1_%2")
      .arg( rp.x() )
      .arg( rp.y() );

  QString path = scaleCachePath( doc.geoloader(), "", doc.projection(), doc.scale() )
    + "/" + coordstr
    + "/" + raddr
    + ".png";
  return path;
}

bool BasisCache::renderRect( const QRect& r, Document* d, QPainter* p )
{
  QPoint pnt = d->absolutetransform().inverted().map( r.topLeft() );
  QImage img( rectFileName(d,r), "PNG" );
  if ( true == img.isNull() ) {
    return false;
  }
  p->drawImage( pnt, img );
  return true;
}

bool BasisCache::containsRect( Document* d, const QRect& r ) const
{
  QFile file( rectFileName( d, r ) );
  if ( true == file.exists() ) {
    return true;
  }
  return false;
}

}
}
