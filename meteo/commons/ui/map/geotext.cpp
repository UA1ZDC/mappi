#include "geotext.h"

#include <qfontmetrics.h>
#include <qpainter.h>
#include <qtextlayout.h>
#include <qdebug.h>
#include <qline.h>
#include <qpainterpath.h>

#include <commons/geom/geom.h>
#include <meteo/commons/global/global.h>

#include "document.h"
#include "ramka.h"
#include "layergrid.h"


namespace meteo {
namespace map {

namespace {
  Object* createGeoText( Layer* l )
  {
    return new GeoText(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( GeoText::Type, createGeoText );
  Object* createGeoText2( Object* o )
  {
    return new GeoText(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( GeoText::Type, createGeoText2 );
}

GeoText::GeoText( Layer* l )
  : Label(l),
  hasvalue_(false)
{
}

GeoText::GeoText( Object* p )
  : Label(p),
  hasvalue_(false)
{
}

GeoText::GeoText( Projection* proj )
  : Label(proj),
  hasvalue_(false)
{
}

GeoText::GeoText( const meteo::Property& prop )
  : Label(prop),
  hasvalue_(false)
{
}

GeoText::~GeoText()
{
}

Object* GeoText::copy( Layer* l ) const
{
  if ( 0 == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  GeoText* gt = new GeoText(l);
  gt->setProperty(property_);
  gt->hasvalue_ = hasvalue_;
  gt->text_ = text_;
  gt->value_ = value_;
  gt->format_ = format_;
  gt->unit_ = unit_;
  gt->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gt);
  }
  return gt;
}

Object* GeoText::copy( Object* o ) const
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  GeoText* gt = new GeoText(o);
  gt->setProperty(property_);
  gt->hasvalue_ = hasvalue_;
  gt->text_ = text_;
  gt->value_ = value_;
  gt->format_ = format_;
  gt->unit_ = unit_;
  gt->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gt);
  }
  return gt;
}

Object* GeoText::copy( Projection* proj ) const
{
  if ( 0 == proj ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  GeoText* gt = new GeoText(proj);
  gt->setProperty(property_);
  gt->hasvalue_ = hasvalue_;
  gt->text_ = text_;
  gt->value_ = value_;
  gt->format_ = format_;
  gt->unit_ = unit_;
  gt->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(gt);
  }
  return gt;
}

QList< QPair < QPoint, float > > GeoText::screenPoints( const QTransform& transform ) const
{
  QPoint translatepoint = qtranslateXy();
  QList< QPair< QPoint, float > > points;
  if ( 0 != screenpos_ && 0 == parent_ ) {      //установлена экранная координата
    points.append( qMakePair(*screenpos_ + translatepoint, 0.0 ) );
  }
  else if ( 0 != cartesian_points_.size() ) {    //есть геогр. координаты. рисовать по геогр. координатам текста
    for ( int i = 0, sz = cartesianSkelet().size(); i < sz; ++i ) {
      QPoint pnt;
      if ( 0 == cartesianSkelet()[i].size() ) {
        continue;
      }
      else if ( 1 == cartesianSkelet()[i].size() ) {
        pnt = transform.map( cartesianSkelet()[i][0] );
        points.append( qMakePair( pnt + translatepoint, 0.0 ) );
//        debug_log << "normal =" << pnt << translatepoint;
      }
      else {
        QPolygon screen = transform.map( cartesianSkelet()[i] );
        for ( int j = 0, jsz = screen.size(); j < jsz; ++j ) {
          points.append( qMakePair( screen[j] + translatepoint, 0.0 ) );
        }
      }
    }
  }
  if ( nullptr == parent_ ) {
    return points;
  }

  QVector<QPolygon> cartesianparent = parent_->cartesianSkelet();
  QVector<QPolygon> screenlist;
  for ( auto p : cartesianparent ) {
    screenlist.append( transform.map(p) );
  }
  points.append( geolinePoints(screenlist) );
  points.append( isolinePoints(screenlist) );
  points.append( geotextPoints(screenlist) );
  return points;
}

QList<QRect> GeoText::boundingRect( const QTransform& transform ) const
{
  QList<QRect> list;
  QList< QPair< QPoint, float > > points = screenPoints(transform);

  Position orient = pos();

  for ( int i = 0, sz = points.size(); i < sz; ++i ) {
    const QPoint& pnt = points[i].first;
    float angle = points[i].second;
    QPen pen = qpen();
    TextRamka rmk = ramka();
    int dopotstup = 0;
    if ( (rmk & kBorder) == kBorder ) {
      dopotstup = pen.width() + 2;
    }
    QRect r = Label::boundingRect( pnt, orient, angle );
    if ( true == r.isNull() ) {
      continue;
    }
    r.setTopLeft( r.topLeft() - QPoint( dopotstup, dopotstup ) );
    r.setBottomRight( r.bottomRight() + QPoint( dopotstup, dopotstup ) );
    list.append(r);
  }
  return list;
}

QRect GeoText::boundingRect() const
{
  if ( false == cached_rect_.isNull() ) {
    return cached_rect_;
  }
  QImage testpix( QSize(1,1), QImage::Format_ARGB32 );
  QPainter testpntr(&testpix);
  testpntr.setFont( qfont() );
  testpntr.setPen( qpen() );
  QRect r = testpntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, text_ );
  return r;
}

QList<GeoVector> GeoText::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  QList<GeoVector> gvlist;
  Projection* proj = projection();
  if ( 0 == proj ) {
    return gvlist;
  }
  QList< QPair< QPoint, float > > list = screenPoints(transform);
  GeoVector gv;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QPoint& scr = list[i].first;
    if ( false == rect.contains(scr) ) {
      continue;
    }
    GeoPoint gp;
    QPoint cartpoint = transform.inverted().map(scr);
    proj->X2F_one( cartpoint, &gp );
    gv.append(gp);
  }
  gvlist.append(gv);
  return gvlist;
}

void GeoText::setText( const QString& t )
{
  hasvalue_ = false;
  text_ = t;
  delete pix_; pix_ = nullptr;
}

void GeoText::setValue( float v, const QString& f, const QString& u )
{
  value_ = v;
  format_ = f;
  unit_ = u;

  QChar ch_integer(' ');
  QChar ch_fraction(' ');
  int length_integer = -1;
  int length_fraction = -1;

  int pntindx = format_.indexOf('.');
  switch ( pntindx ) {
    case -1:
      if ( 4 > format_.length() ) {
        warning_log << QObject::tr(
            "Неверно задана форматная строка %1."
            "\n\tДолжна быть \"[d'c'][.][d'c']\"."
            "\n\tПример 1: Значение = 100.0 Форматная строка = 40.20. Результат = 0100.00 //четырые символа для целой части, два - для дробной"
            "\n\tПример 2: Значение = 100.1 Форматная строка = 20 Результат = 100         //только целая часть"
            "\n\tПример 3: Значение = 100.1 Форматная строка = .20 Результат = 10         //только дробная часть"
            ).arg(format_);
        length_integer = 4;
        length_fraction = 2;
      }
      else {
        length_fraction = 0;
        length_integer = format_.left( format_.length() - 3 ).toInt();
        ch_integer = format_[ format_.length() - 2 ];
      }
      break;
    case 0:
      if ( format_.length() < 5 ) {
        warning_log << QObject::tr(
            "Неверно задана форматная строка %1."
            "\n\tДолжна быть \"[d'c'][.][d'c']\"."
            "\n\tПример 1: Значение = 100.0 Форматная строка = 40.20. Результат = 0100.00 //четырые символа для целой части, два - для дробной"
            "\n\tПример 2: Значение = 100.1 Форматная строка = 20 Результат = 100         //только целая часть"
            "\n\tПример 3: Значение = 100.1 Форматная строка = .20 Результат = 10         //только дробная часть"
            );
        length_integer = 4;
        length_fraction = 2;
      }
      else {
        length_integer = 0;
        length_fraction = format_.mid( 1, format_.length() - 4 ).toInt();
        ch_fraction = format_[ format_.length() - 2 ];
      }
      break;
    default:
      if ( format_.length() < 9 ) {
        warning_log << QObject::tr(
            "Неверно задана форматная строка %1."
            "\n\tДолжна быть \"[d'c'][.][d'c']\"."
            "\n\tПример 1: Значение = 100.0 Форматная строка = 40.20. Результат = 0100.00 //четырые символа для целой части, два - для дробной"
            "\n\tПример 2: Значение = 100.1 Форматная строка = 20 Результат = 100         //только целая часть"
            "\n\tПример 3: Значение = 100.1 Форматная строка = .20 Результат = 10         //только дробная часть"
            );
        length_integer = 4;
        length_fraction = 2;
      }
      else {
        length_integer = format_.left( pntindx - 3 ).toInt();
        ch_integer = format_[ pntindx - 2 ];
        length_fraction = format_.mid( pntindx+1, format_.length() - pntindx - 4 ).toInt();
        ch_fraction = format_[ format_.length() - 2 ];
      }
      break;
  }
  QString left;
  if ( 0 < length_integer ) {
    left = QString("%1").arg( static_cast<int>(value_), length_integer, 10, QChar(ch_integer) );
  }
  QString right;
  if ( 0 < length_fraction ) {
    float fraqval = value_ - static_cast<int>(value_);
    int fraqint = 0;
    if ( false == MnMath::isZero(fraqval) ) {
      fraqint = fraqval*::pow(10,length_fraction);
    }
    QString str = QString::number(fraqint);
    int sch = str.size() - 1;
    while ( 0 <= sch && '0' == str[sch] ) {
      str[sch] = ch_fraction;
      --sch;
    }
    str.replace('.',"");
    if ( QChar('.') != ch_fraction ) {
      while ( length_fraction > str.size() ) {
        str.append(ch_fraction);
      }
    }
    if ( length_fraction < str.size() ) {
      str = str.left(length_fraction);
    }
    right = str;
  }
  QString t;
  if ( 0 < length_integer && 0 < length_fraction ) {
    if ( false == right.isEmpty() ) {
      t = QString("%1.%2")
        .arg(left)
        .arg(right);
    }
    else {
      t = left;
    }
  }
  else if ( 0 < length_integer ) {
    t = left;
  }
  else {
    t = right;
  }
  if ( false == unit_.isEmpty() ) {
    t += unit_;
  }
  setText(t);
  hasvalue_ = true;
}

int GeoText::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  int dist = 10000000;
  QTransform tr;
  if ( 0 != document() ) {
    tr = document()->transform();
  }
  QList<QRect> list  = boundingRect(tr);
  if ( 0 == list.size() ) {
    return dist;
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    dist = meteo::geom::distance( pos, list[i], cross );
  }
  return dist;
}

int32_t GeoText::dataSize() const
{
  int32_t objsz = Object::dataSize();
  objsz += sizeof(hasvalue_);

  QByteArray loc = text_.toUtf8();
  int32_t sz = loc.size();
  objsz += sizeof(sz);
  objsz += sz;

  objsz += sizeof(value_);

  loc = format_.toUtf8();
  sz = loc.size();
  objsz += sizeof(sz);
  objsz += sz;

  loc = unit_.toUtf8();
  sz = loc.size();
  objsz += sizeof(sz);
  objsz += sz;

  if ( 0 != screenpos_ ) {
    objsz += sizeof(bool); //есть экранная координата
    objsz += 2*sizeof(int32_t); //размер x и y
  }
  else {
    objsz += sizeof(bool);
  }


  return objsz;
}

int32_t GeoText::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  ::memcpy( arr + pos, &hasvalue_, sizeof(hasvalue_) );
  pos += sizeof(hasvalue_);

  QByteArray loc = text_.toUtf8();
  int32_t sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  ::memcpy( arr + pos, &value_, sizeof(value_) );
  pos += sizeof(value_);

  loc = format_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  loc = unit_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  bool hasscreen = false;
  if ( 0 != screenpos_ ) {
    hasscreen = true;
    ::memcpy( arr + pos, &hasscreen, sizeof(hasscreen) );
    pos += sizeof(hasscreen);
    int32_t xy = screenpos_->x();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    xy = screenpos_->y();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
  }
  else {
    ::memcpy( arr + pos, &hasscreen, sizeof(hasscreen) );
    pos += sizeof(hasscreen);
  }

  return pos;
}

int32_t GeoText::parseFromArray( const char* arr )
{
  int32_t pos = 0;
  int32_t sz = Object::setData( arr + pos );
  if ( -1 == sz ) {
    return sz;
  }
  pos += sz;
  global::fromByteArray( arr + pos, &hasvalue_ );
  pos += sizeof(hasvalue_);
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  text_ = QString::fromUtf8( arr + pos, sz );
  pos += sz;
  global::fromByteArray( arr + pos, &value_ );
  pos += sizeof(value_);
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  format_ = QString::fromUtf8( arr + pos, sz );
  pos += sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  unit_ = QString::fromUtf8( arr + pos, sz );
  pos += sz;
  bool hasscreen = false;
  global::fromByteArray( arr + pos, &hasscreen );
  pos += sizeof(hasscreen);
  if ( true == hasscreen ) {
    int32_t x; int32_t y;
    global::fromByteArray( arr + pos, &x );
    pos += sizeof(x);
    global::fromByteArray( arr + pos, &y );
    pos += sizeof(y);
    setScreenPos( QPoint(x,y) );
  }
  setSkelet(skelet_);
  return pos;
}

QList< QPair< QPoint, float > > GeoText::isolinePoints( const QVector<QPolygon>& parentscreen ) const
{
  QList< QPair<QPoint, float> > list;
  if ( nullptr == document() ) {
    return list;
  }
  if ( kIsoline != parent_->type() ) {
    return list;
  }

  if ( false == drawOnRamka() ) {
    return list;
  }
  if (false==document()->property().text_towards_iso() ) {
    list = pointsTextHorizont(parentscreen);
  }
  else {
    list=pointsTextOnLine(parentscreen);
  }
  return list;
}

QList< QPair< QPoint, float > > GeoText::pointsTextHorizont( const QVector<QPolygon>& parentscreen ) const
{
  QList< QPair<QPoint, float> > list;
  const QPolygon& ramka = document()->isolineRamka();
  const QVector<QPolygon>& polylist = parentscreen;
  for ( int i = 0, sz = polylist.size(); i < sz; ++i ) {
    const QPolygon& poly = polylist[i];
    QList<float> anglist;
    QList<QPoint> cross = geom::crossPoint( poly, ramka, &anglist );
    for ( int j = 0, jsz = cross.size(); j < jsz; ++j ) {
      list.append( qMakePair( cross[j], 0.0) );
    }
  }
  return list;
}

QList< QPair< QPoint, float > > GeoText::pointsTextOnLine( const QVector<QPolygon>& parentscreen ) const
{
  QList< QPair<QPoint, float> > list;
  const QPolygon& ramka = document()->isolineRamka();
  const QVector<QPolygon>& polylist = parentscreen;
  for ( int i = 0, sz = polylist.size(); i < sz; ++i ) {
    const QPolygon& poly = polylist[i];
    QPoint nowDot,nextDot,prevDot;
    QLineF line;
    float range=0.0;
    float angle=0.0;
    bool flSet=false;
    for (int i=1; i<poly.size()-1; i++){
      prevDot=poly[i-1];
      nowDot=poly[i];
      nextDot=poly[i+1];
      if ( false == ramka.containsPoint(nowDot, Qt::OddEvenFill)
           && false == ramka.containsPoint(nextDot, Qt::OddEvenFill) ) {
        continue;
        range = 0.0;
      }
      float delta = ::sqrt((nextDot.x()-nowDot.x())*(nextDot.x()-nowDot.x()) +
                            (nextDot.y()-nowDot.y())*(nextDot.y()-nowDot.y()));
      range+=delta;
      if (range>=400.0){
        line.setP1(prevDot);
        line.setP2(nextDot);
        angle=line.angle();
        angle=(360-angle)*(M_PI/180);
        if ( (M_PI_2 < angle) && (angle<=(M_PI+M_PI_2)) ) {
          angle += M_PI;
        }
        if (angle>2*M_PI){
          angle-=(2*M_PI);
        }
        list.append( qMakePair( nowDot, angle ) );
        flSet=true;
        range=0;
      }
      if (( false==flSet ) && ( (poly.size()-2)==i )) {
        list.append( qMakePair( nowDot, 0.0) );
      }
    }
  }
  return list;
}

QList< QPair< QPoint, float > > GeoText::geolinePoints( const QVector<QPolygon>& parentscreen ) const
{
  QList< QPair< QPoint, float > > list;
  if ( kPolygon != parent_->type() && kIsoline != parent_->type() ) {
    return list;
  }
  if ( kIsoline == parent_->type() && 0 != document() && true == document()->property().text_towards_iso() ) {
    return list;
  }
  const QVector<QPolygon>& cach = parentscreen;
  bool drawonends;
  if ( false == property_.has_draw_on_ends() ) {
    drawonends = parent_->drawOnOnEnds();
  }
  else {
    drawonends = drawOnOnEnds();
  }
  for ( int i = 0, sz = cach.size(); i < sz; ++i ) {
    const QPolygon& poly = cach[i];
    if ( 2 > poly.size() ) { //подписи к полигонам из одной точки не рисовать
      continue;
    }
    if ( poly.first() != poly.last() && false == parent_->closed() ) {
      if (  true == drawonends ) {
        list.append( qMakePair( poly.first(), 0.0 ) );
        list.append( qMakePair( poly.last(), 0.0 ) );
      }
    }
    else { //рисоовать в зависимости от property_.posOnParent
      QLine top( poly.first(), poly.first() );
      QLine left(top);
      QLine down(top);
      QLine right(top);
      for ( int j = 1, jsz = poly.size(); j < jsz; ++j ) {
        const QPoint& pnt = poly[j];
        if ( pnt.y() < top.y1() ) {
          top = QLine( pnt, pnt );
        }
        else if ( pnt.y() == top.y1() ) {
          top.setP2(pnt);
        }
        if ( pnt.y() > down.y1() ) {
          down = QLine( pnt, pnt );
        }
        else if ( pnt.y() == down.y1() ) {
          down.setP2(pnt);
        }
        if ( pnt.x() < left.x1() ) {
          left = QLine( pnt, pnt );
        }
        else if ( pnt.x() == left.x1() ) {
          left.setP2(pnt);
        }
        if ( pnt.x() > right.x1() ) {
          right = QLine( pnt, pnt );
        }
        else if ( pnt.x() == right.x1() ) {
          right.setP2(pnt);
        }
      }
      QPoint tpnt = QPoint( top.x1() + top.dx()/2, top.y1() );
      QPoint dpnt = QPoint( down.x1() + down.dx()/2, down.y1() );
      QPoint lpnt = QPoint( left.x1(), left.y1() + left.dy()/2 );
      QPoint rpnt = QPoint( left.x1(), right.y1() + right.dy()/2 );
      Position posonparent;
      if ( false == property_.has_pos_on_parent() ) {
        posonparent = parent_->posOnParent();
      }
      else {
        posonparent = posOnParent();
      }
      switch ( posonparent ) {
        case kTopCenter:
          list.append( qMakePair( tpnt, 0.0 ) );
          break;
        case kBottomCenter:
          list.append( qMakePair( dpnt, 0.0 ) );
          break;
        case kLeftCenter:
          list.append( qMakePair( lpnt, 0.0 ) );
          break;
        case kRightCenter:
          list.append( qMakePair( rpnt, 0.0 ) );
          break;
        case kCenter:
          {
            QPoint p1 = poly.first();
            QPoint p2 = poly.last();
            float angle = atan2( p2.y() - p1.y(), p2.x() - p1.x() );
            if ( M_PI_2 < angle ) {
              angle -= M_PI;
            }
            else if ( -M_PI_2 > angle ) {
              angle += M_PI;
            }
            list.append( qMakePair(poly.boundingRect().center(), angle) );
          }
          break;
        default:
 //         warning_log << QObject::tr("Задана необрабатываемая позиция относительно родителя");
          break;
      }
    }
  }
  return list;
}

QList< QPair< QPoint, float > > GeoText::geotextPoints( const QVector<QPolygon>& parentscreen ) const
{
  QList< QPair< QPoint, float > > list;
  if ( kText != parent_->type() && kPuanson != parent_->type() ) {
    return list;
  }
  for ( auto poly : parentscreen ) {
    if ( 0 == poly.size() ) {
      continue;
    }
    const QPoint pnt = poly[0] + *screenpos_;
    list.append( qMakePair( pnt, 0.0 ) );
  }
  return list;
}

bool GeoText::preparePixmap()
{
  cached_rect_ = QRect();

  QRect r = boundingRect();
  if ( false == r.isValid() ) {
    if ( nullptr != pix_ ) {
      delete pix_; pix_ = 0;
    }
    return false;
  }
  if ( nullptr != pix_ ) {
    delete pix_; pix_ = 0;
  }
  cached_rect_ = r;

  QFontMetrics fm = QFontMetrics( qfont() );
  pix_ = new QImage( r.size(), QImage::Format_ARGB32 );
  pix_->fill( QColor( 0,0,0, 0 ) );
  QPainter pntr(pix_);
  pntr.setRenderHint( QPainter::HighQualityAntialiasing, true );
  pntr.setRenderHint( QPainter::TextAntialiasing, true );
  pntr.translate( -r.topLeft() );

  QPen pn;
  QBrush bru;
  QFont fnt;
  if ( 0 != parent_ && ( kIsoline == parent_->type() || kPolygon == parent_->type() ) ) {
    pn = parent_->qpen();
    bru = parent_->qbrush();
    fnt = parent_->qfont();
  }
  else {
    pn = qpen();
    bru = qbrush();
    fnt = qfont();
  }

  if ( 0 != document() && true == document()->property().text_outline() ) {
    QPainterPath path;
    QPointF strokepos =  r.bottomLeft() + QPointF(-0.5,-fm.descent()-1);// - QPoint( 0, 2 );
    path.addText( strokepos, font2qfont( property_.font() ), text_ );
    QPainterPathStroker stroker;
    stroker.setDashPattern(Qt::SolidLine);
    stroker.setWidth(0.5);
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setCurveThreshold(0.99);
    QPainterPath strpath = stroker.createStroke(path);
    QColor clr = QColor::fromRgba( document()->property().outlinecolor() );;
    QPen outpen(clr);
    pntr.save();
    pntr.setPen(outpen);
    QBrush br(outpen.color());
    pntr.setBrush(br);
    pntr.drawPath(strpath);
    pntr.restore();
  }

  pntr.setPen(pn);
  pntr.setBrush(bru);
  pntr.setFont(fnt);
  pntr.drawText( r, Qt::AlignHCenter | Qt::AlignVCenter, text_ );
  return true;
}

void GeoText::prepareChange()
{
  delete pix_; pix_ = nullptr;
}

}
}
