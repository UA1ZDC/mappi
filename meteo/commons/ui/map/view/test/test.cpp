#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qpixmap.h>
#include <qmessagebox.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/loader/loader.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/novost/settings/tmeteosettings.h>
#include <meteo/novost/settings/tadminsettings.h>

namespace {
QList<meteo::map::MapView*> views;
}

void createMapView( const meteo::GeoPoint& gp, int width, int height, int scale, meteo::map::Incut::Position pos, meteo::map::Map* map )
{
  meteo::map::Document* doc = map->createDocument( QSize(width, height), gp, meteo::map::proto::kGeoMap );
  doc->setIncutPosition(pos);
  doc->setScale(scale);
  meteo::map::MapView* view = new meteo::map::MapView(doc);
  views.append(view);
  view->show();
  view->turnSceneEvents();
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  meteo::map::Map* map = new meteo::map::Map( meteo::MERCAT );
//  meteo::map::Map* map = new meteo::map::Map( meteo::STEREO );
  map->projection()->setSouthern();
  map->projection()->setMapCenter( meteo::GeoPoint::fromDegree( -60, 120 ) );
  bool res = meteo::map::Loader::instance()->handleData( "geo.old", map );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось згрузить геогр. основу %1").arg("geo.old");
  }
  createMapView( meteo::GeoPoint::fromDegree( -60, 30 ), 1024, 768, 14, meteo::map::Incut::TOP_LEFT, map );
//  createMapView( meteo::GeoPoint::fromDegree( 56, 111 ), 1024, 768, 17, meteo::map::Incut::BOTTOM_RIGHT, map );
//  createMapView( meteo::GeoPoint::fromDegree( 23, -60 ), 200, 100, 18, meteo::map::Incut::BOTTOM_LEFT, map );
  app.exec();

  for ( int i = 0, sz = views.size(); i < sz; ++i ) {
    delete views[i];
  }
  views.clear();
  delete map; map = 0;
  return EXIT_SUCCESS;
}
