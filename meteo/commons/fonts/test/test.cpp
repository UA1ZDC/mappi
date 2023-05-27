#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qpixmap.h>

#include <cross-commons/app/paths.h>

#include <meteo/commons/fonts/weatherfont.h>

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  meteo::WeatherFont::instance();
  return EXIT_SUCCESS;
}
