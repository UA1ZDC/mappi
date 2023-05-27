#include "common.h"

#include <qstring.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qfont.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpoint.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psqlquery.h>
#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>

#include "weatherloader.h"

namespace meteo {

ProjectionType projection2proto( int type )
{
  return projection2proto( static_cast<meteo::ProjType>(type) );
}

ProjectionType projection2proto( meteo::ProjType type )
{
  switch (type) {
    case meteo::MERCAT:
      return kMercat;
      break;
    case meteo::CONICH:
      return kConical;
      break;
    case meteo::GENERAL:
      return kGeneral;
      break;
    case meteo::STEREO:
      return kStereo;
      break;
    case meteo::POLARORBSAT:
      return kSatellite;
      break;
  default:
      break;
  }
  return kStereo;
}

meteo::ProjType proto2projection( ProjectionType type )
{
  switch ( type ) {
    case kMercat:
      return meteo::MERCAT;
      break;
    case ::meteo::kConical:
      return meteo::CONICH;
      break;
    case ::meteo::kGeneral:
      return meteo::GENERAL;
      break;
    case ::meteo::kStereo:
      return meteo::STEREO;
      break;
    case ::meteo::kSatellite:
      return meteo::POLARORBSAT;
      break;
    default:
      break;
  }
  return meteo::STEREO;
}

QPen pen2qpen( const Pen& p )
{
  QPen qpen;
  QVector<qreal> dp;
  for ( int i = 0, sz = p.dash_pattern_size(); i < sz; ++i ) {
    dp << p.dash_pattern(i);
  }
  if ( 0 != dp.size() ) {
    qpen.setDashPattern(dp);
  }
  qpen.setStyle( static_cast<Qt::PenStyle>( p.style() ) );
  qpen.setWidth( p.width() );
  qpen.setColor( QColor::fromRgba( p.color() ) );
  return qpen;
}

Pen qpen2pen( const QPen& p )
{
  Pen pen;
  pen.set_style( static_cast<PenStyle>( p.style() ) );
  pen.set_width( p.width() );
  pen.set_color( p.color().rgba() );
  QVector<qreal> dp = p.dashPattern();
  for ( int i = 0, sz = dp.size(); i < sz; ++i ) {
    pen.add_dash_pattern( dp[i] );
  }
  return pen;
}

QBrush brush2qbrush( const Brush& b )
{
  QBrush qbrush;
  qbrush.setStyle( static_cast<Qt::BrushStyle>( b.style() ) );
  qbrush.setColor( QColor::fromRgba( b.color() ) );
  return qbrush;
}

Brush qbrush2brush( const QBrush& b )
{
  Brush brush;
  brush.set_style( static_cast<BrushStyle>( b.style() ) );
  brush.set_color( b.color().rgba() );
  return brush;
}

QFont font2qfont( const Font& f )
{
  QFont qfont;
  qfont.setFamily( QString::fromStdString( f.family() ) );
  qfont.setPointSize( f.pointsize() );
  qfont.setWeight( f.weight() );
  qfont.setBold( f.bold() );
  qfont.setItalic( f.italic() );
  qfont.setUnderline( f.underline() );
  qfont.setStrikeOut( f.strikeout() );
  return qfont;
}

Font qfont2font( const QFont& f )
{
  Font font;
  font.set_family( f.family().toStdString() );
  font.set_pointsize( f.pointSize() );
  font.set_weight( f.weight() );
  font.set_bold( f.bold() );
  font.set_italic( f.italic() );
  font.set_underline( f.underline() );
  font.set_strikeout( f.strikeOut() );
  return font;
}

QPoint point2qpoint( const Point& p )
{
  QPoint qpoint( p.x(), p.y() );
  return qpoint;
}

Point qpoint2point( const QPoint& p )
{
  Point point;
  point.set_x( p.x() );
  point.set_y( p.y() );
  return point;
}

QPointF pointf2qpointf( const PointF& p )
{
  QPointF qpoint( p.x(), p.y() );
  return qpoint;
}

PointF qpointf2pointf( const QPointF& p )
{
  PointF point;
  point.set_x( p.x() );
  point.set_y( p.y() );
  return point;
}

int position2qtalignmentflag( Position pos )
{
  int fl;
  switch (pos) {
    case kCenter:
      fl = Qt::AlignCenter;
      break;
    case kTopCenter:
      fl = Qt::AlignTop | Qt::AlignHCenter;
      break;
    case kLeftCenter:
      fl = Qt::AlignVCenter | Qt::AlignLeft;
      break;
    case kTopLeft:
      fl = Qt::AlignTop | Qt::AlignLeft;
      break;
    case kBottomCenter:
      fl = Qt::AlignBottom | Qt::AlignHCenter;
      break;
    case kBottomLeft:
      fl = Qt::AlignBottom | Qt::AlignLeft;
      break;
    case kRightCenter:
      fl = Qt::AlignVCenter | Qt::AlignRight;
      break;
    case kTopRight:
      fl = Qt::AlignTop | Qt::AlignRight;
      break;
    case kBottomRight:
      fl = Qt::AlignBottom | Qt::AlignRight;
      break;
    case kNoPosition:
    case kFloat:
      fl = pos;
      break;
  }
  return fl;
}

Position qtalignmentflag2position( int align )
{
  Position fl = kNoPosition;
  if ( Qt::AlignCenter == align ) {
    fl = kCenter;
  }
  else if ( ( Qt::AlignTop | Qt::AlignHCenter ) == align ) {
    fl = kTopCenter;
  }
  else if ( ( Qt::AlignVCenter | Qt::AlignLeft ) == align ) {
    fl = kLeftCenter;
  }
  else if ( ( Qt::AlignTop | Qt::AlignLeft ) == align ) {
    fl = kTopLeft;
  }
  else if ( ( Qt::AlignBottom | Qt::AlignHCenter ) == align ) {
    fl = kBottomCenter;
  }
  else if ( ( Qt::AlignBottom | Qt::AlignLeft ) == align ) {
    fl = kBottomLeft;
  }
  else if ( ( Qt::AlignVCenter | Qt::AlignRight ) == align ) {
    fl = kRightCenter;
  }
  else if ( ( Qt::AlignTop | Qt::AlignRight ) == align ) {
    fl = kTopRight;
  }
  else if ( ( Qt::AlignBottom | Qt::AlignRight ) == align ) {
    fl = kBottomRight;
  }
  else {
    fl = kNoPosition;
  }
  return fl;
}

QSize size2qsize( const Size& sz )
{
  QSize qsz( sz.width(), sz.height() );
  return qsz;
}

Size qsize2size( const QSize& qsz )
{
  Size sz;
  sz.set_width( qsz.width() );
  sz.set_height( qsz.height() );
  return sz;
}

QSizeF sizef2qsize( const SizeF& sz )
{
  QSizeF qsz( sz.width(), sz.height() );
  return qsz;
}

SizeF qsize2size( const QSizeF& qsz )
{
  SizeF sz;
  sz.set_width( qsz.width() );
  sz.set_height( qsz.height() );
  return sz;
}

GeoPoint surfGeopoint2geopoint( const meteo::surf::Point& sgp )
{
  GeoPoint gp;
  if ( true == sgp.has_fi() ) {
    gp = GeoPoint( sgp.fi(), sgp.la(), sgp.height() ) ;
  }
  return gp;
}

meteo::surf::Point geopoint2SurfGeopoint( const GeoPoint& gp )
{
  meteo::surf::Point sgp;
  sgp.set_fi( gp.lat() );
  sgp.set_la( gp.lon() );
  sgp.set_height( gp.alt() );
  return sgp;
}

GeoPoint pbgeopoint2geopoint( const meteo::GeoPointPb& pbgp )
{
  GeoPoint gp;
  if ( true == pbgp.has_lat_radian() ) {
    gp = GeoPoint( pbgp.lat_radian(), pbgp.lon_radian(), pbgp.height_meters(), pbgptype2gptype( pbgp.type() ) );
  }
  else if ( true == pbgp.has_lat_deg() ) {
    gp = GeoPoint::fromDegree( pbgp.lat_deg(), pbgp.lon_deg(), pbgp.height_meters(), pbgptype2gptype( pbgp.type() ) );
  }
  return gp;
}

meteo::GeoPointPb geopoint2pbgeopoint( const GeoPoint& gp )
{
  meteo::GeoPointPb pbgp;
  pbgp.set_lat_deg( gp.latDeg() );
  pbgp.set_lon_deg( gp.lonDeg() );
  pbgp.set_height_meters( gp.alt() );
  pbgp.set_type( gptype2pbgptype( gp.type() ) );
  return pbgp;
}

GeoVector pbgeovector2geovector( const GeoVectorPb& pbgv )
{
  GeoVector gv( pbgv.point_size() );
  for ( int i = 0, sz = pbgv.point_size(); i < sz; ++i ) {
    gv[i] = pbgeopoint2geopoint( pbgv.point(i) );
  }
  return gv;
}

GeoVectorPb geovector2pbgeovector( const GeoVector& gv )
{
  GeoVectorPb pbgv;
  for ( int i = 0, sz = gv.size(); i < sz; ++i ) {
    pbgv.add_point()->CopyFrom( geopoint2pbgeopoint( gv[i] ) );
  }
  return pbgv;
}

TypeGeoPointPb gptype2pbgptype( meteo::typeGeoCoord type )
{
  meteo::TypeGeoPointPb t;
  switch ( type ) {
    case meteo::LA180:
      t = meteo::kLA180;
      break;
    case meteo::LA360:
      t = meteo::kLA360;
      break;
    case meteo::LA_GENERAL:
      t = meteo::kLA_GENERAL;
      break;
  }
  return t;
}
meteo::typeGeoCoord pbgptype2gptype( meteo::TypeGeoPointPb type )
{
  meteo::typeGeoCoord t;
  switch ( type ) {
    case meteo::kLA180:
      t = meteo::LA180;
      break;
    case meteo::kLA360:
      t = meteo::LA360;
      break;
    case meteo::kLA_GENERAL:
      t = meteo::LA_GENERAL;
      break;
  }
  return t;
}

QRect rectByOrient( const QRect& source, Position orient )
{
  QRect r(source);
  QPoint pnt = source.topLeft();
  switch ( orient ) {
    case kCenter:
      break;
    case kTopCenter:
      r.moveBottom(pnt.y());
      break;
    case kLeftCenter:
      r.moveRight(pnt.x());
      break;
    case kTopLeft:
      r.moveBottomRight( r.center() );
      break;
    case kBottomCenter:
      r.moveTop(pnt.y());
      break;
    case kBottomLeft:
      r.moveTopRight( r.center() );
      break;
    case kRightCenter:
      r.moveLeft(pnt.x());
      break;
    case kTopRight:
      r.moveBottomLeft( r.center() );
      break;
    case kBottomRight:
      r.moveTopLeft( r.center() );
      break;
    case kNoPosition:
    case kFloat:
      r = QRect();
      break;
  }
  return r;
}

QMap< QString, Property > mapSettings()
{
  QMap<QString, Property> m;
  global::createMapSettingsDir();
  QFile file(global::kMapColorSettingsFilename);
  if ( false == file.exists() ) {
    createDefaultColorSettings(global::kMapColorSettingsFilename);
  }
  if ( false == file.exists() ) {
    error_log << QObject::tr("Файл с параметрами картографических документов не существует\n\t Имя файла - %1")
      .arg(global::kMapColorSettingsFilename);
    return m;
  }
  Properties protmsg;
  if ( false == TProtoText::fromFile( global::kMapColorSettingsFilename, &protmsg ) ) {
    error_log << QObject::tr("Не удалось прочитать из файла %1 набор свойств элементов карты.")
      .arg(global::kMapColorSettingsFilename);
    return m;
  }
  for ( int i = 0, sz = protmsg.property_size(); i < sz; ++i  ) {
    const Property& p = protmsg.property(i);
    m.insert( QString::fromStdString( p.name() ), p );
  }
  return m;
}

QMap< QString, Property > gridSettings()
{
  QMap<QString, Property> m;
  global::createMapSettingsDir();
  QFile file(global::kMapGridSettingsFilename);
  if ( false == file.exists() ) {
    error_log << QObject::tr("Файл с параметрами картографических документов не существует\n\t Имя файла - %1")
      .arg(global::kMapGridSettingsFilename);
    return m;
  }
  Properties protmsg;
  if ( false == TProtoText::fromFile( global::kMapGridSettingsFilename, &protmsg ) ) {
    error_log << QObject::tr("Не удалось прочитать из файла %1 набор свойств элементов карты.")
      .arg( global::kMapGridSettingsFilename );
    return m;
  }
  for ( int i = 0, sz = protmsg.property_size(); i < sz; ++i  ) {
    const Property& p = protmsg.property(i);
    m.insert( QString::fromStdString( p.name() ), p );
  }
  return m;
}

bool angleOnPolygon( const QRect r, const QPolygon& poly, int pnt_indx, QPointF* pnt, float* resangle )
{
  float width = r.width();
  if ( 0 == width ) {
    return pnt_indx;
  }
  float width2 = width*width;
//  float height = r.height();
//  float height_2 = height/2.0;

  int poly_size = poly.size();

  float shift = 0.0;
  QPointF p1(*pnt);
  for ( int i = pnt_indx; i < poly_size && 0.0 < width; ++i ) {
    QPointF p2 = poly[i];
    QPointF distpnt = p2 - p1;
    if ( true == MnMath::isZero( distpnt.x() ) && true == MnMath::isZero( distpnt.y() ) ) {
      ++pnt_indx;
      continue;
    }
    float angle = ::atan2( distpnt.y(), distpnt.x() );
//    float angledeg = angle*RAD2DEG;
    float dist2 = distpnt.x()*distpnt.x() + distpnt.y()*distpnt.y();
    if ( dist2 > width2 ) {
      pnt->setX( p1.x() + (width)*::cos(angle) );
      pnt->setY( p1.y() + (width)*::sin(angle) );
      *resangle = angle;
      return true;
    }
    if ( i+1 >= poly_size ) {
      return false;
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
    angle = ::atan2( distpnt.y(), distpnt.x() );// - M_PI_2;

 //   QPolygonF target;
 //   float x = height_2*::cos(angle);
 //   float y = height_2*::sin(angle);
 //   target.append( p1 + QPoint( x,y ) );
 //   target.append( p3 + QPoint( x,y ) );
 //   target.append( p3 - QPoint( x,y ) );
 //   target.append( p1 - QPoint( x,y ) );
 //   QRect source( QPoint( shift, 0 ), QSize( distnew + dist, height ) );
////    QRect source( img->rect() );
 //   QTransform tr;
 //   if ( true == QTransform::quadToQuad( QPolygon(source), target, tr ) ) {
 //     painter->save();
 //     painter->setTransform( tr, true );
 //     painter->drawImage( QPointF(shift,0), *img, source );
 //     painter->restore();
 //   }
    width -= (distnew + dist);
    if ( 0.0 > width ) {
      width = 0.0;
    }
    shift += distnew + dist;
    ++i;
    pnt_indx = i;
    p1 = p3;
    *pnt = p3;
    if ( width < 3 ) {
      *resangle = angle;
      return true;
    }
  }
  return false;
}

int paintBit( QPainter* painter, QImage* img, const QPolygon& poly, int pnt_indx, QPointF* pnt )
{
  float width = img->width();
  if ( 0 == width ) {
    return -pnt_indx;
  }
  float width2 = width*width;
  float height = img->height();
  float height_2 = height/2.0;

  int poly_size = poly.size();

  float shift = 0.0;
  QPointF p1(*pnt);
  for ( int i = pnt_indx; i < poly_size && 0.0 < width; ++i ) {
    QPointF p2 = poly[i];
    QPointF distpnt = p2 - p1;
    if ( true == MnMath::isZero( distpnt.x() ) && true == MnMath::isZero( distpnt.y() ) ) {
      ++pnt_indx;
      continue;
    }
    float angle = ::atan2( distpnt.y(), distpnt.x() );
    float angledeg = angle*RAD2DEG;
    float dist2 = distpnt.x()*distpnt.x() + distpnt.y()*distpnt.y();
    if ( dist2 > width2 ) {
      QTransform tr;
      tr.translate(p1.x(),p1.y());
      tr.rotate(angledeg);
      tr.translate( 0, -height/2.0 );
      painter->save();
      painter->setTransform( tr, true );
      painter->drawImage( QPointF(0,0), *img, QRectF( QPointF( shift, 0.0 ), QSizeF( width+1, height ) ) );
      painter->restore();
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

    QPolygonF target;
    float x = height_2*::cos(angle);
    float y = height_2*::sin(angle);
    target.append( p1 + QPoint( x,y ) );
    target.append( p3 + QPoint( x,y ) );
    target.append( p3 - QPoint( x,y ) );
    target.append( p1 - QPoint( x,y ) );
    QRect source( QPoint( shift, 0 ), QSize( distnew + dist, height ) );
//    QRect source( img->rect() );
    QTransform tr;
    if ( true == QTransform::quadToQuad( QPolygon(source), target, tr ) ) {
      painter->save();
      painter->setTransform( tr, true );
      painter->drawImage( QPointF(shift,0), *img, source );
      painter->restore();
    }
    width -= (distnew + dist);
    if ( 0.0 > width ) {
      width = 0.0;
    }
    shift += distnew + dist;
    ++i;
    pnt_indx = i;
    p1 = p3;
    *pnt = p3;
    if ( width < 3 ) {
      return pnt_indx;
    }
  }
  return pnt_indx;
}

enum GeoDataSpec{
  UNKNOWNDATASPEC = -1,
  COAST       =         31110000,
  COAST2      =         32220000,
  LAKE        =         31120000,
  DELIM       =         81110000,
  STRIKE      =         31110011,
  ISLAND31    =         31110001,
  ISLAND34    =         34000000,
  ISLAND,
  HYDRO       =         31410000,
  STATION     =         41110000,
  STAT_HYDRO  =         41110001,
  CITY        =         41100000,
  RIVER,
  BORDER_MAP  =         60000002,
  STRIKE_BORDER =       60000002,
  DATATYPES_COUNT
};

const int Generalization::LEVELSIZE = 30;

Generalization::Generalization()
  : lowlimit_(0),
  highlimit_(LEVELSIZE)
{
  for ( int i = 0; i <= LEVELSIZE; ++i ) {
    scales_.insert( i, true );
  }
}

Generalization::Generalization( const General& p )
{
  loadProto(p);
}

void Generalization::setLowLimit( int scale )
{
  if ( 0 > scale ) {
    error_log << QObject::tr("Неверный параметр scale = %1")
      .arg(scale);
    return;
  }
  lowlimit_ = scale;
  if ( LEVELSIZE < lowlimit_ ) {
    lowlimit_ = LEVELSIZE;
  }
  if ( lowlimit_ > highlimit_ ) {
    highlimit_ = lowlimit_;
  }
  for ( int i = 0; i <= highlimit_; ++i ) {
    bool visible = ( i < lowlimit_ ) ? false : true ;
    scales_.insert( i, visible );
  }
}

void Generalization::setHighLimit( int scale )
{
  if ( 0 > scale ) {
    error_log << QObject::tr("Неверный параметр scale = %1")
      .arg(scale);
    return;
  }
  highlimit_ = scale;
  if ( LEVELSIZE < highlimit_ ) {
    highlimit_ = LEVELSIZE;
  }
  if ( highlimit_ < lowlimit_ ) {
    lowlimit_ = highlimit_;
  }
  for ( int i = LEVELSIZE; i >= lowlimit_ ; --i ) {
    bool visible = ( i > highlimit_ ) ? false : true ;
    scales_.insert( i, visible );
  }
}

void Generalization::setLimits( int low, int high )
{
  if ( 0 > low || 0 > high ) {
    error_log << QObject::tr("Неверные Входные параметры low = %1 hight = %2")
      .arg(low)
      .arg(high);
    return;
  }
  setLowLimit(low);
  setHighLimit(high);
}

Generalization& Generalization::loadProto( const General& p )
{
  highlimit_ = -1;
  lowlimit_ = 100000;
  QMap<int,bool> m;
  for ( int i = 0, sz = p.level_size(); i < sz; ++i ) {
    const General::Level& l = p.level(i);
    m.insert( l.level(), l.visible() );
    if ( l.level() > highlimit_ ) {
      highlimit_ = l.level();
    }
    if ( l.level() < lowlimit_ ) {
      lowlimit_ = l.level();
    }
  }
  if ( lowlimit_ > highlimit_ ) {
    lowlimit_ = highlimit_;
  }
  scales_ = m;

  return *this;
}

General Generalization::proto() const
{
  General p;
  QMapIterator<int,bool> it(scales_);
  while ( true == it.hasNext() ) {
    it.next();
    General::Level* l = p.add_level();
    l->set_level( it.key() );
    l->set_visible( it.value() );
  }
  return p;
}

void createDefaultColorSettings( const QString& filename )
{
  QMap< QString,  meteo::Property > setts;

   meteo::Property prop;
  QPen pen;
  pen.setColor( meteo::kMAP_COAST_COLOR);
  pen.setWidth(1);
  QBrush brush;
  brush.setStyle( Qt::SolidPattern );
  brush.setColor( meteo::kMAP_SEA_COLOR);
  prop.set_closed(true);
  meteo::Generalization g;
  g.setLimits(0,30);
  prop.mutable_pen()->CopyFrom( meteo::qpen2pen(pen) );
  prop.mutable_brush()->CopyFrom( meteo::qbrush2brush(brush) );
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(COAST).toStdString() );
  setts.insert( QString::number(COAST), prop );

  g.setHighLimit(16);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(LAKE).toStdString() );
  setts.insert( QString::number(LAKE), prop );

  brush.setColor( meteo::kMAP_ISLAND_COLOR);
  g.setHighLimit(30);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.mutable_brush()->CopyFrom( meteo::qbrush2brush(brush) );
  prop.set_name( QString::number(ISLAND31).toStdString() );
  setts.insert( QString::number(ISLAND31), prop );

  prop.set_name( QString::number(ISLAND34).toStdString() );
  setts.insert( QString::number(ISLAND34), prop );

  prop.set_closed(false);
  brush.setStyle( Qt::NoBrush );
  prop.mutable_brush()->CopyFrom( meteo::qbrush2brush(brush) );
  pen.setColor( meteo::kMAP_HYDRO_COLOR);
  g.setHighLimit(14);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.mutable_pen()->CopyFrom( meteo::qpen2pen(pen) );
  prop.set_name( QString::number(HYDRO).toStdString() );
  setts.insert( QString::number(HYDRO), prop );

  pen.setColor( meteo::kMAP_COAST_COLOR);
  prop.mutable_pen()->CopyFrom( meteo::qpen2pen(pen) );
  prop.set_closed(true);
  g.setHighLimit(30);
  prop.mutable_general()->CopyFrom(g.proto());
  prop.set_name( QString::number(BORDER_MAP).toStdString() );
  setts.insert( QString::number(BORDER_MAP), prop );

  pen = QPen();
  pen.setWidth(1);
  pen.setColor( meteo::kMAP_DELIM_COLOR);
  prop.mutable_pen()->CopyFrom( meteo::qpen2pen(pen) );
  prop.set_closed(false);
  g.setHighLimit(14);
  prop.mutable_general()->CopyFrom( g.proto() );
  prop.set_name( QString::number(DELIM).toStdString() );
  setts.insert( QString::number(DELIM), prop );

  pen.setColor( meteo::kMAP_SEA_COLOR);
  pen.setWidth(2);
  prop.mutable_pen()->CopyFrom( meteo::qpen2pen(pen) );
  prop.set_closed(false);
  g.setHighLimit(30);
  prop.mutable_general()->CopyFrom(g.proto());
  prop.set_name( QString::number(STRIKE).toStdString() );
  setts.insert( QString::number(STRIKE), prop );
  prop.set_name( QString::number(STRIKE_BORDER).toStdString() );
  setts.insert( QString::number(STRIKE_BORDER), prop );

  prop =  meteo::Property();
  pen = QPen();
  pen.setWidth(1);
  prop.mutable_pen()->CopyFrom( meteo::qpen2pen(pen) );
  brush = QBrush();
  brush.setColor( meteo::kMAP_ISLAND_COLOR);
  prop.mutable_brush()->CopyFrom( meteo::qbrush2brush(brush) );
  prop.set_name( meteo::kMAPCOLORSETTINGS_IDENT.toStdString() );
  setts.insert(  meteo::kMAPCOLORSETTINGS_IDENT, prop );
  meteo::Properties prots;
  QMapIterator<QString, meteo::Property> it(setts);
  while ( true == it.hasNext() ) {
    it.next();
     meteo::Property* p = prots.add_property();
    p->CopyFrom( it.value() );
  }
  TProtoText::toFile( prots, filename );
}

void datadesc2weatherlayer( const field::DataDesc& datadesc, map::proto::WeatherLayer* layer )
{
  layer->set_mode( map::proto::kIsoline );
  layer->set_source( map::proto::kField );
  layer->set_center( datadesc.center() );
  layer->set_model( datadesc.model() );
  layer->set_level( datadesc.level() );
  layer->set_type_level( datadesc.level_type() );
  layer->set_template_name( QString::number( datadesc.meteodescr() ).toStdString() ); //слабое место
  layer->set_datetime( datadesc.date() );
  layer->set_hour( datadesc.hour() );
  layer->set_data_size( datadesc.count_point() );
  if ( true == datadesc.has_center_name() ) {
    layer->set_center_name( datadesc.center_name() );
  }
  layer->set_type(kLayerIso);
  layer->set_forecast_beg( datadesc.dt1() );
  layer->set_forecast_end( datadesc.dt2() );
}

void sigwx2weatherlayer( const surf::SigwxDesc& sigwxdesc, map::proto::WeatherLayer* layer )
{
  layer->set_mode( map::proto::kSigwx );
  layer->set_center( sigwxdesc.center() );
  layer->set_datetime( sigwxdesc.date() );
  layer->set_h1( sigwxdesc.level_lo() );
  layer->set_h2( sigwxdesc.level_hi() );
  QDateTime dt = QDateTime::fromString( QString::fromStdString( sigwxdesc.date() ), Qt::ISODate );
  QDateTime dtend = QDateTime::fromString( QString::fromStdString( sigwxdesc.forecast_end() ), Qt::ISODate );
  layer->set_hour( dt.secsTo(dtend)/3600 );
  layer->set_type(kLayerIso);
  layer->set_forecast_beg( sigwxdesc.forecast_beg() );
  layer->set_forecast_end( sigwxdesc.forecast_end() );
}

QString replacePatternByDate( const QString& pattern, const QDateTime& dt )
{
  int begindx = pattern.indexOf('$');
  if ( -1 == begindx ) {
    return pattern;
  }
  int endindx =  pattern.indexOf('$', begindx + 1 );
  if ( -1 == endindx ) {
    return pattern;
  }
  QString dtptrn = pattern.mid( begindx, endindx - begindx + 1);
  begindx += 1;
  int len = endindx - begindx;
  if ( 0 >= len ) {
    return pattern;
  }
  QString dtstr = pattern.mid( begindx, endindx - begindx );
  dtstr = dt.toString(dtstr);
  QString result = pattern;
  result.replace( dtptrn, dtstr );
  return result;
}

QString layernameFromInfo( const field::DataDesc& desc )
{
  map::proto::WeatherLayer info;
  datadesc2weatherlayer( desc, &info );
  return layernameFromInfo(info);
}

QString layernameFromInfo( const surf::SigwxDesc& desc )
{
  map::proto::WeatherLayer info;
  sigwx2weatherlayer( desc, &info );
  return layernameFromInfo(info);
}

QString layernameFromInfo( const map::proto::WeatherLayer& info )
{
  QString centername;
  if ( true == info.has_center() ) {
    QMap<int, QPair<QString, int> > centernames = global::kMeteoCenters();
    if ( true == centernames.contains( info.center() ) ) {
      centername = centernames[info.center()].first;
    }
  }
  switch ( info.mode() ) {
    case map::proto::kEmptyLayer:
      if ( true == info.has_layer_name_pattern() ) {
        return QString::fromStdString( info.layer_name_pattern() );
      }
      return QString();
      break;
    case map::proto::kPuanson: {
        //QString name = QObject::tr("Новый слой");
        QString result = QString::fromStdString( info.layer_name_pattern() );
        QString code = QString::fromStdString( info.template_name() );
        if ( true == map::WeatherLoader::instance()->punchlibrary().contains(code) ) {
          code = QString::fromStdString( map::WeatherLoader::instance()->punchlibrary()[code].name() );
        }
        QString templname = QObject::tr("Наноска: %1")
          .arg(code);
        result.replace("[template]", templname);
        QDateTime dt = PsqlQuery::datetimeFromString( QString::fromStdString( info.datetime() ) );
        if ( true == dt.isValid() ) {
          result = replacePatternByDate( result, dt );
        }
        QString lvlstr;
        if ( 1 == info.type_level() ) {
          lvlstr = QObject::tr("у земли");
        }
        else if ( 100 == info.type_level() ) {
          lvlstr = QObject::tr("%1 мбар.").arg( info.level() );
        }
        else if ( 102 == info.type_level() ) {
          lvlstr = QObject::tr("Уровень моря.");
        }
        else if ( 160 ==  info.type_level() ) {
          lvlstr = QObject::tr("Глубина %1 м.").arg( info.level() );
        }
        else if ( 9 ==  info.type_level() ) {
          lvlstr = QObject::tr("Дно");
        }
        else if ( 7 ==  info.type_level() ) {
          lvlstr = QObject::tr("Уровень тропопаузы");
        }
        else if ( 6 ==  info.type_level() ) {
          lvlstr = QObject::tr("Уровень максимального ветра");
        }
        result.replace( "[level]",  lvlstr );
        result.replace( "[center]", centername );
        QString hour;
        if ( /*proto::kField == info.source() && */0 != info.hour() ) {
          hour = QObject::tr("прогноз на %1 ч.").arg(info.hour());
        }
        result.replace( "[hour]", hour );

        QString count;
        if ( true == info.has_data_size() ) {
          count = QObject::tr("кол-во: %1").arg(info.data_size());
        }
        result.replace( "[count]", count );

        return result;
      }
      break;
    case map::proto::kIsoline:
    case map::proto::kGradient: {
       // QString name = QObject::tr("Новый слой");
        QString result = QString::fromStdString( info.layer_name_pattern() );
        QString code = QString::fromStdString( info.template_name() );
        if ( true == map::WeatherLoader::instance()->isolibrary().contains(code) ) {
          code = QString::fromStdString( map::WeatherLoader::instance()->isolibrary()[code].name() );
        }
        QString templname = QObject::tr("%1")
          .arg(code);
        result.replace("[template]",templname);
        QDateTime dt = PsqlQuery::datetimeFromString( QString::fromStdString( info.datetime() ) );
        if ( true == dt.isValid() ) {
          result = replacePatternByDate( result, dt );
        }
        QString lvlstr;
        if ( 1 == info.type_level() ) {
          lvlstr = QObject::tr("у земли");
          if ( "22021" == info.template_name() || "22043" == info.template_name() ) {
            lvlstr = "";
          }
        }
        else if ( 100 == info.type_level() ) {
          lvlstr = QObject::tr("%1 мбар.").arg( info.level() );
        }
        else if ( 102 == info.type_level() ) {
          lvlstr = QObject::tr("Уровень моря.");
        }
        else if ( 160 ==  info.type_level() ) {
          lvlstr = QObject::tr("Глубина %1 м.").arg( info.level() );
        }
        else if ( 9 ==  info.type_level() ) {
          lvlstr = QObject::tr("Дно");
        }
        result.replace("[level]",lvlstr);
        result.replace( "[center]", centername );
        QString hour;
        if ( map::proto::kField == info.source() ) {
          if (  0 != info.hour() ) {
            hour = QObject::tr("прогноз на %1 ч.").arg(info.hour()/3600);
          }
          else {
            hour = QObject::tr("анализ.");
          }
        }
        result.replace( "[hour]", hour );

        QString count;
        if ( true == info.has_data_size() ) {
          count = QObject::tr("кол-во: %1").arg(info.data_size());
        }
        result.replace( "[count]", count );

        return result;
      }
      break;
    case map::proto::kRadar: {
        //QString name = QObject::tr("Новый слой");
        QString result = QString::fromStdString( info.layer_name_pattern() );
        QString code = QString::fromStdString( info.template_name() );
        if ( true == map::WeatherLoader::instance()->radarlibrary().contains(code) ) {
          code = QString::fromStdString( map::WeatherLoader::instance()->radarlibrary()[code].name() );
        }
        QString templname = QObject::tr("%1")
          .arg(code);
        result.replace("[template]",templname);
        QDateTime dt = PsqlQuery::datetimeFromString( QString::fromStdString( info.datetime() ) );
        if ( true == dt.isValid() ) {
          result = replacePatternByDate( result, dt );
        }
        QString lvlstr;
        if(0 == info.h1() && 0 == info.h2() ){
          //FIXME lvlstr = QObject::tr("у земли");
        } else {
          if(0 != info.h1() && 0 == info.h2() ){
            lvlstr = QObject::tr("на высоте %1 м").arg(info.h1());
          } else {
            if( 0 != info.h2() ){
              lvlstr = QObject::tr("в слое %1 - %2 м").arg(info.h1()).arg(info.h2());
            }
          }
        }
        result.replace("[layer]",lvlstr);
        result.replace( "[center]", centername );
//        result.replace( "[level]", "" );
//        result.replace( "[hour]", "" );

        QString count;
        if ( true == info.has_data_size() ) {
          count = QObject::tr("кол-во: %1").arg(info.data_size());
        }
        result.replace( "[count]", count );

        return result;
      }
      break;
  case map::proto::kSigwx: {
      //QString name = QObject::tr("Новый слой");
      QString result = QString::fromStdString( info.layer_name_pattern() ).append(" [layer]");
      QString code = QObject::tr("Карта особых явлений");
      result.replace("[template]", code);
      QDateTime dt = PsqlQuery::datetimeFromString( QString::fromStdString( info.datetime() ) );
      if ( true == dt.isValid() ) {
        result = replacePatternByDate( result, dt );
      }
      QString lvlstr = QObject::tr("в слое %1 - %2 м").arg(info.h1()).arg(info.h2());
      result.replace( "[level]", lvlstr );
      result.replace( "[center]", centername );
      result.replace( "[layer]", "" );
      QString hour;
      if (  0 != info.hour() ) {
        hour = QObject::tr("прогноз на %1 ч.").arg(info.hour());
      }
      else {
        hour = QObject::tr("анализ.");
      }
      result.replace( "[hour]", hour );
     
      QString count;
      if ( true == info.has_data_size() ) {
        count = QObject::tr("кол-во: %1").arg(info.data_size());
      }
      result.replace( "[count]", count );
     
      return result;
    }
    break;
  default:
    break;
  }
  return QString();
}

}
