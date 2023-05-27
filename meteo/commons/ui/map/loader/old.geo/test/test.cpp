#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qpixmap.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/loader/loader.h>

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  meteo::map::Map* map = new meteo::map::Map( meteo::STEREO );
//  meteo::map::Map* map = new meteo::map::Map( meteo::MERCAT );
  bool res = meteo::map::Loader::instance()->handleData( "geo.old", map );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось згрузить геогр. основу %1").arg("geo.old");
  }
  meteo::GeoPoint gp;
  gp.setLatDeg(60);
  gp.setLonDeg(30);
  //meteo::map::Document* doc = map->createDocument( QSize(600, 400), gp, QSize(600, 400) );
  meteo::map::Document* doc = map->createDocument( QSize(600, 400), gp );
  doc->setScale(15);
  QPixmap pikcha;
  if ( false == doc->drawDocument(&pikcha) ) {
    error_log << QObject::tr("Не удалось создать картинку");
    return EXIT_FAILURE;
  }
  pikcha.save("test.png");
  return EXIT_SUCCESS;
}
