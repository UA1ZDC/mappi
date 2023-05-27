#include "field.h"

#include <algorithm>

#include <qcolor.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>
#include <commons/geobasis/projection.h>
#include <commons/obanal/tfield.h>

#include "layer.h"
#include "map.h"
#include "document.h"

namespace meteo {
namespace map {

namespace internal {

QMap< int, QColor > colors = TMap< int, QColor >()
  << qMakePair( 1, QColor(Qt::red) );

}

Field::Field( Layer* l )
  : Object(l,kGradient),
  field_(0)
{
}

Field::~Field()
{
  delete field_; field_ = nullptr;
}

QList<GeoVector> Field::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  Q_UNUSED(rect);
  Q_UNUSED(transform);
  return QList<GeoVector>();
}

void Field::setData( obanal::TField* field )
{
  delete field_; field_ = 0;
  field_ = field;
  QColor beg( 0, 0, 255, 125 );
  QColor end( 255, 0, 0, 125 );
  grad_ = TColorGrad( -12, 20.0, beg, end );
//  debug_log << "datushka";
}
    
void Field::render( QPainter* painter, const QRect& target, Document* document )
{
  Q_UNUSED(painter);
  Q_UNUSED(target);
  if ( 0 == field_ ) {
    warning_log << QObject::tr("Нет данных для отбражения");
    return;
  }
  if ( true == field_->isEmpty() ) {
    warning_log << QObject::tr("Массив данных пуст. Нечего рисовать");
    return;
  }
  if ( false == target.isValid() ) {
    error_log << QObject::tr("Область для отрисовки указана неверно");
    return;
  }
  bool ok = false;
  QTransform inverted = document->transform().inverted(&ok);
  if ( false == ok ) {
    error_log << QObject::tr("Ошибка преобразования координат");
    return;
  }
  QPoint leftop = target.topLeft();
  int step = 5;
  double min = 5000000.0;
  double max = -5000000.0;
  for ( int i = 0, isz = target.width(); i < isz; i += step ) {
    for ( int j = 0, jsz = target.height(); j < jsz; j += step ) {
      QPoint srcpnt = leftop + QPoint( i, j );
      QPoint pnt = inverted.map(srcpnt);
      meteo::GeoPoint geo;
      bool ok = layer()->projection()->X2F_one( pnt, &geo );
      if ( false == ok ) {
        warning_log << QObject::tr("Не удалось вычислить географическую координату экранной точки QPoint(%1,%2)")
          .arg(srcpnt.x())
          .arg(srcpnt.y());
        continue;
      }
      double latdeg = geo.latDeg();
      double londeg = geo.lonDeg();
      if ( latdeg > 90.0 || latdeg < -90.0 || londeg < -180.0 || londeg > 180.0 ) {
        continue;
      }
//      double val = field_->pointValue( geo, &ok );
      double val = field_->radarValue( geo, &ok );
      if ( val < min ) {
        min = val;
      }
      if ( val > max ) {
        max = val;
      }
      if ( true == ok && 0.0 != val  ) {
//          debug_log << "coord =" << geo << "value =" << val;
        painter->fillRect( QRect(srcpnt, QSize(step, step)), grad_.color(val) );
      }
    }
  }
//  debug_log << "MIN =" << min;
//  debug_log << "MAX =" << max;
}

int Field::minimumScreenDistancePow2( const QPoint& pos, double scalepower ) const
{
  Q_UNUSED(pos);
  Q_UNUSED(scalepower);
  return 1000000;
}
    
void Field::setValue( double val, const QString& format, const QString& unit )
{
  Q_UNUSED(val);
  Q_UNUSED(format);
  Q_UNUSED(unit);
}

}
}
