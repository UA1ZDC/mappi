#include "posdocument.h"

#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/layergrid.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>

#include <cross-commons/debug/tlog.h>

#include <mappi/projection/pos.h>

namespace meteo {
namespace map {

PosDocument::PosDocument( const proto::Document& proto )
  : Document(proto)
{
}

bool loadCoordLayer( PosDocument* doc )
{
  LayerGrid* l = doc->gridLayer();
  delete l;
  l = new LayerGrid( doc, QObject::tr("Координатная сетка") );

  Projection* proj = doc->projection();

  int startFi = proj->startFi()*RAD2DEG;
  int endFi = proj->endFi()*RAD2DEG;
  int step = ( startFi < endFi ) ? 1 : -1;

  // longitude lines
  for ( int lon = -179; lon < 181; lon += 1 ) {
    if(0 == lon % 5) {
      GeoVector gv;
      for ( int lat = startFi; lat != endFi + step; lat += step ) {
        GeoPoint gp = GeoPoint::fromDegree( lat, lon );
        gv.append(gp);
      }
      GeoPolygon* geopol = new GeoPolygon(l);
      geopol->setPenColor(4286545791);
      geopol->setPenWidth(0);
      l->addLongitude(geopol);
      GeoText* geotxt = new GeoText(geopol);
      geotxt->setValue( lon, "1'0'", QObject::tr("\xc2\xb0") );
      geotxt->setDrawOnOnEnds(false);
      geotxt->setPos(meteo::kCenter);
      geotxt->setPosOnParent(meteo::kNoPosition);
      geopol->setSkelet(gv);
      meteo::Font fnt = geopol->font();
      fnt.set_pointsize(9);
      geopol->setFont(fnt);
    }
  }

  // latitude lines
  for ( int lat = startFi; lat != endFi+step; lat += step ) {
    if(0 == lat % 2) {
      GeoVector gv;
      for ( int lon = -180; lon <= 180; lon += 1  ) {
        GeoPoint gp = GeoPoint::fromDegree( lat, lon );
        gv.append(gp);
      }
      GeoPolygon* geopol = new GeoPolygon(l);
      geopol->setPenColor(4286545791);
      geopol->setPenWidth(0);
      l->addLatitude(geopol);
      GeoText* geotxt = new GeoText(geopol);
      geotxt->setValue( lat, "1'0'", QObject::tr("\xc2\xb0") );
      if ( MERCAT != l->projection()->type() ) {
        geotxt->setPosOnParent(meteo::kTopCenter);
      }
      else {
        geotxt->setPosOnParent(meteo::kNoPosition);
      }
      geotxt->setPos(meteo::kCenter);
      geotxt->setDrawOnOnEnds(false);
      geopol->setSkelet(gv);
      meteo::Font fnt = geopol->font();
      fnt.set_pointsize(9);
      geopol->setFont(fnt);
    }
  }

  return true;
}


bool PosDocument::init( const QDateTime& ts, const QDateTime& te, SatelliteBase* params, double sa, double w, double lps )
{
  meteo::POSproj* pos = new meteo::POSproj;
  pos->setSatellite( ts, te, params, sa, w, lps );
  setProjection(pos);

  GeoPoint sc = pos->getMapCenter();
  setScreenCenter(sc);
  if ( 0 == projection() ) {
    error_log << QObject::tr("Невозможно загрузить слои географической основы. Параметры проекции не установлены.");
    return false;
  }
  if ( false == property().has_geoloader() ) {
    return false;
  }
  bool res = Loader::instance()->handleData( QString::fromStdString( property().geoloader() ), this );
  if ( false == res ) {
    return res;
  }
  loadCoordLayer(this);
//  loadCitiesLayer(this);
  return true;
//  return loadCoordLayer(this);
}

}
}
