#include "satelliteline.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qmath.h>

#include <commons/obanal/interpolorder.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geom/geom.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/gradientparams.h>
#include <meteo/commons/global/common.h>

namespace meteo {
namespace map {



SatelliteLine::SatelliteLine( Layer* l)
  : GeoPolygon(l)
{
}

SatelliteLine::SatelliteLine( Object* p)
  : GeoPolygon(p)
{
}

SatelliteLine::SatelliteLine( Projection* p)
  : GeoPolygon(p)
{
}

SatelliteLine::SatelliteLine( )
  : GeoPolygon( proto::Property() )
{
}


SatelliteLine::~SatelliteLine()
{
}

bool SatelliteLine::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  cached_screen_points_.clear();
  QVector<QPolygon>::const_iterator it = cartesian_points_.constBegin();
  QVector<QPolygon>::const_iterator end = cartesian_points_.constEnd();
  for ( ; end != it; ++it ) {
    QPolygon screen = transform.map(*it);
    if ( 2 > screen.size() ) {
      continue;
    }
    if ( true == property_.closed() && screen.first() != screen.last() ) {
      screen.append( screen.first() );
    }
    bool need_draw = false;
    QRect screenrect = screen.boundingRect();
    bool screenrect_contains_target_whole = screenrect.contains( target, true );
    if ( false == screenrect_contains_target_whole && true == target.intersects(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw && false == screenrect_contains_target_whole && true == screenrect.intersects(target) ) {
      need_draw = true;
    }
    if ( false == need_draw && true == target.contains(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw ) {
      QPainterPath path;
      path.addPolygon(screen);
      var(screen.size());
      bool screenpathcontains_target = path.contains(target);
      QBrush br = qbrush();
      if ( false == screenpathcontains_target && true == path.intersects(target) ) {
        need_draw = true;
      }
      else if ( Qt::NoBrush != br.style() && true == closed() ) {
        if ( true == screenpathcontains_target ) {
          need_draw = true;
        }
      }
    }
    if ( false == need_draw ) {
      continue;
    }
    cached_screen_points_.append(screen);
    wasdrawed_ = true;
  }

  QList<GeoText*> geoTextList=Object::childsByType<GeoText*>();
  QPainterPath pntrpath = painter->clipPath();
  pntrpath.addRegion(QRegion(target));
  for (int i=0;i<geoTextList.size();i++) {
    GeoText* text=geoTextList[i];
    QList< QPair< QPoint,double > > list = text->screenPoints(transform);
    proto::Position orient = text->pos();
    for (int i = 0, sz = list.size(); i<sz;i++) {
      const QPoint& pnt = list[i].first;
      double angle = list[i].second*RAD2DEG;
      QPoint oldtopleft;
      QRect r = text->boundingRect( pnt, orient, angle, &oldtopleft );
      QPainterPath recpath;
      recpath.addRect(r);
      pntrpath -= recpath;
    }
  }
  painter->save();
  if( 0 != geoTextList.size()) {
    painter->setClipPath(pntrpath,Qt::ReplaceClip);;
  }
  bool result=GeoPolygon::render( painter, target, transform );
  painter->restore();
  return result;
}


}
}
