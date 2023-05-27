#include "ornament.h"

#include <qimage.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/global/common.h>

#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace map {

Ornament::Ornament( const QString& pattern, float linewidth, QRgb color )
  : factor_(1.0)
{
  if ( 1.0 > linewidth ) {
    linewidth = 1.0;
  }
  const QMap< QString, proto::Ornament >& lib = WeatherLoader::instance()->ornamentlibrary();
  if ( false == lib.contains(pattern) ) {
    error_log << QObject::tr("Не найден шаблон орнамента %1").arg(pattern);
  }
  else {
    ornament_.CopyFrom( lib[pattern] );
//    int minheight = 100000;
    for ( int i = 0, sz = ornament_.bit_size(); i < sz; ++i ) {
      const proto::Ornament::Bit& bit = ornament_.bit(i);
      QString filename = QString(":/meteo/icons/ornament/%1").arg( QString::fromStdString( bit.pixname() ) );
      proto::Mirror m = proto::kNoMirror;
      if ( proto::kNoMirror != bit.mirror() ) {
        m = bit.mirror();
      }
      else if ( proto::kNoMirror != ornament_.mirror() ) {
        m = ornament_.mirror();
      }
      if ( proto::kNoMirror != m ) {
        bool hfl = false;
        bool vfl = false;
        switch (m) {
          case proto::kMirrorH:
            hfl = true;
            break;
          case proto::kMirrorV:
            vfl = true;
            break;
          case proto::kMirrorBoth:
            hfl = true;
            vfl = true;
            break;
          case proto::kNoMirror:
            break;
        }
        patterns_[i] = QImage(filename).mirrored( hfl, vfl );
      }
      else {
        patterns_[i] = QImage(filename);
      }
      if ( true == bit.colorized() ) {
        if ( false == bit.has_color() ) {
          setBitColor( &(patterns_[i]), color );
        }
        else {
          setBitColor( &(patterns_[i]), bit.color() );
        }
      }

    }
    if ( true != MnMath::isEqual( 1.0f, linewidth ) ) {
      factor_ = linewidth;
    }
    for ( int i = 0, sz = ornament_.bit_size(); i < sz; ++i ) {
      const proto::Ornament::Bit& bit = ornament_.bit(i);
      QSize size = size2qsize( bit.size() );
      if ( true == size.isEmpty() ) {
        size = patterns_[i].size();
      }
      if ( false == MnMath::isEqual( 1.0f, factor_ ) ) {
        size *= factor_;
      }
      patterns_[i] = patterns_[i].scaled( size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }
  }
}

Ornament::Ornament( const QString& pattern, float linewidth, QRgb color, bool mirror )
  : factor_(1.0)
{
  if ( 1.0 > linewidth ) {
    linewidth = 1.0;
  }
  const QMap< QString, proto::Ornament >& lib = WeatherLoader::instance()->ornamentlibrary();
  if ( false == lib.contains(pattern) ) {
    error_log << QObject::tr("Не найден шаблон орнамента %1").arg(pattern);
  }
  else {
    ornament_.CopyFrom( lib[pattern] );
    if ( false == mirror ) {
      ornament_.set_mirror(proto::kNoMirror);
    }
    else {
      ornament_.set_mirror(proto::kMirrorV);
    }
    for ( int i = 0, sz = ornament_.bit_size(); i < sz; ++i ) {
      const proto::Ornament::Bit& bit = ornament_.bit(i);
      QString filename = QString(":/meteo/icons/ornament/%1").arg( QString::fromStdString( bit.pixname() ) );
      proto::Mirror m = proto::kNoMirror;
      if ( proto::kNoMirror != bit.mirror() ) {
        m = bit.mirror();
      }
      else if ( proto::kNoMirror != ornament_.mirror() ) {
        m = ornament_.mirror();
      }
      if ( proto::kNoMirror != m ) {
        bool hfl = false;
        bool vfl = false;
        switch (m) {
          case proto::kMirrorH:
            hfl = true;
            break;
          case proto::kMirrorV:
            vfl = true;
            break;
          case proto::kMirrorBoth:
            hfl = true;
            vfl = true;
            break;
          case proto::kNoMirror:
            break;
        }
        patterns_[i] = QImage(filename).mirrored( hfl, vfl );
      }
      else {
        patterns_[i] = QImage(filename);
      }
      if ( true == bit.colorized() ) {
        if ( false == bit.has_color() ) {
          setBitColor( &(patterns_[i]), color );
        }
        else {
          setBitColor( &(patterns_[i]), bit.color() );
        }
      }

    }
    if ( true != MnMath::isEqual( 1.0f, linewidth ) ) {
      factor_ = linewidth;
    }
    for ( int i = 0, sz = ornament_.bit_size(); i < sz; ++i ) {
      const proto::Ornament::Bit& bit = ornament_.bit(i);
      QSize size = size2qsize( bit.size() );
      if ( true == size.isEmpty() ) {
        size = patterns_[i].size();
      }
      if ( false == MnMath::isEqual( 1.0f, factor_ ) ) {
        size *= factor_;
      }
      patterns_[i] = patterns_[i].scaled( size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }
  }
}

Ornament::~Ornament()
{
}

int Ornament::width( float linewidth ) const
{
  if ( 1.0 > linewidth ) {
    linewidth = 1.0;
  }
  float w = 0.0;
  for ( int i = 0, sz = ornament_.bit_size(); i < sz; ++i ) {
    w += patterns_[i].width();
    w += ornament_.bit(i).step()*linewidth;
  }
  return w;
}
    
proto::Ornament Ornament::ornament( const QString& name )
{
  const QMap< QString, proto::Ornament >& lib = WeatherLoader::instance()->ornamentlibrary();
  if ( false == lib.contains(name) ) {
    error_log << QObject::tr("Не найден шаблон орнамента %1").arg(name);
  }
  else {
    return lib[name];
  }
  return proto::Ornament();
}

void Ornament::paintOnDocument( QPainter* painter, const QPolygon& polygon )
{
  if ( 0 == ornament_.bit_size() ) {
    return;
  }
  painter->save();
  painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
  QPointF pnt = polygon[0];
  for ( int i = 1, sz = polygon.size(); i < sz; /*++i*/ ) {
    for ( int j = 0, jsz = ornament_.bit_size(); j < jsz; ++j ) {
      const proto::Ornament::Bit& bit = ornament_.bit(j);
      i = paintBit( painter, &(patterns_[j]), polygon, i, &pnt );
      if ( i < 0 ) {
        painter->restore();
        error_log << QObject::tr("Случилось внезапное");
        return;
      }
      if ( i >= sz ) {
        painter->restore();
        return;
      }
      if ( 0 != bit.step() ) {
        float st = bit.step()*factor_;
        float littlestep = 5.0;
        while ( st > littlestep ) {
          i = step( littlestep, polygon, i, &pnt );
          st -= littlestep;
        }
        if ( false == MnMath::isZero(st) && st > 0.0 ) {
          i = step( st, polygon, i, &pnt );
        }
        if ( i < 0 ) {
          painter->restore();
          error_log << QObject::tr("Случилось внезапное");
          return;
        }
        if ( i >= sz ) {
          painter->restore();
          return;
        }
      }
    }
  }
  painter->restore();
}
    
int Ornament::step( float step, const QPolygon& poly, int pnt_indx, QPointF* pnt )
{
  float width = step;
  float width2 = width*width;

  int poly_size = poly.size();

  float shift = 0.0;
  QPointF p1(*pnt);
//  for ( int i = pnt_indx; i < poly_size && false == MnMath::isZero(width); ++i ) {
  for ( int i = pnt_indx; i < poly_size && 0.0 < width; ++i ) {
    QPointF p2 = poly[i];
    QPointF distpnt = p2 - p1;
    if ( true == MnMath::isZero( distpnt.x() ) && true == MnMath::isZero( distpnt.y() ) ) {
      ++pnt_indx;
      continue;
    }
    float angle = ::atan2( distpnt.y(), distpnt.x() );
    float dist2 = distpnt.x()*distpnt.x() + distpnt.y()*distpnt.y();
    if ( dist2 > width2 ) {
      pnt->setX( p1.x() + (width)*::cos(angle) );
      pnt->setY( p1.y() + (width)*::sin(angle) );
      return pnt_indx;
    }
    if ( i+1 >= poly_size ) {
      return i+1;
    }
    QPointF p3 = poly[i+1];
    QPointF perelom = p2;
    QPointF distpntnew = p3 - perelom;
    if ( true == MnMath::isZero( distpntnew.x() ) && true == MnMath::isZero( distpntnew.y() ) ) {
      continue;
    }
    float dist = ::sqrt(dist2);
    float ostatokwidth = width - dist;
    float ostatokwidth2 = ostatokwidth*ostatokwidth;
    float distnew2 = distpntnew.x()*distpntnew.x() + distpntnew.y()*distpntnew.y();
    while ( distnew2 < ostatokwidth2 && ++i + 1 < poly_size ) {
      if ( i < 0 ) {
//        return;
      }
      p3 = poly[i+1];
      distpntnew = p3 - perelom;
      distnew2 = distpntnew.x()*distpntnew.x() + distpntnew.y()*distpntnew.y();
    }
    if ( distnew2 > ostatokwidth2 ) {
      float angle2 = ::atan2( distpntnew.y(), distpntnew.x() );
      p3.setX( perelom.x() + (ostatokwidth)*::cos(angle2) );
      p3.setY( perelom.y() + (ostatokwidth)*::sin(angle2) );
      distpntnew = p3 - perelom;
      distnew2 = distpntnew.x()*distpntnew.x() + distpntnew.y()*distpntnew.y();
    }
    distpnt = p3 - p1;
    float distnew = ::sqrt(distnew2);
    angle = ::atan2( distpnt.y(), distpnt.x() ) - M_PI_2;

    width -= (distnew + dist);
    if ( 0.0 > width ) {
      width = 0.0;
    }
    shift += distnew + dist;
    ++i;
    pnt_indx = i;
    p1 = p3;
    *pnt = p3;
  }
  return pnt_indx;
}
//    
//int Ornament::step( float step, const QPolygon& poly, int pnt_indx, QPointF* pnt )
//{
//  float width = step;
//  float width2 = width*width;
//
//  QPointF p1 = *pnt;
//  if ( pnt_indx >= poly.size() ) {
//    return pnt_indx;
//  }
//  QPointF p2 = poly[pnt_indx];
//  while ( true == MnMath::isEqual( p1.x(),p2.x() ) && true == MnMath::isEqual( p1.y(), p2.y() ) && ++pnt_indx < poly.size() ) {
//    p2 = poly[pnt_indx];
//  }
//  if ( pnt_indx >= poly.size() ) {
//    return pnt_indx;
//  }
//  QPointF distpnt = p2 - p1;
//  float angle = ::atan2( distpnt.y(), distpnt.x() );
//  float dist2 = distpnt.x()*distpnt.x() + distpnt.y()*distpnt.y();
//  if ( dist2 < width2 ) {
//    float dist = ::sqrt(dist2);
//    float ostatokwidth = width - dist;
//    float ostatokwidth2 = ostatokwidth*ostatokwidth;
//    QPointF perelom = p2;
//    for ( int j = ++pnt_indx, sz = poly.size(); j < sz; ++j  ) {
//      QPointF p3 = poly[j];
//      if ( true == MnMath::isEqual( perelom.x(), p3.x() ) && true == MnMath::isEqual( perelom.y(), p3.y() ) ) {
//        continue;
//      }
//      QPointF distpntnew = p3 - perelom;
//      float distnew2 = distpntnew.x()*distpntnew.x() + distpntnew.y()*distpntnew.y();
//      if ( distnew2 < ostatokwidth2 ) {
//        continue;
//      }
//      float angle2 = ::atan2( distpntnew.y(), distpntnew.x() );
//      p3.setX( perelom.x() + (ostatokwidth)*::cos(angle2) );
//      p3.setY( perelom.y() + (ostatokwidth)*::sin(angle2) );
//      pnt_indx = j;
//      *pnt = p3;
//      break;
//    }
//    return pnt_indx;
//  }
//  pnt->setX( p1.x() + (width)*::cos(angle) );
//  pnt->setY( p1.y() + (width)*::sin(angle) );
//  return pnt_indx;
//}

void Ornament::setBitColor( QImage* img, QRgb rgba )
{
  for ( int i = 0, isz = img->width(); i < isz; ++i ) {
    for ( int j = 0, jsz = img->height(); j < jsz; ++j ) {
      QRgb src = img->pixel( i, j );
      if ( 0 != qAlpha(src) ) {
        img->setPixel( i, j, rgba );
      }
    }
  }
}

}
}
