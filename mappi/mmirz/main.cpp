#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <mmirz.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qpixmap.h>

#include <stdlib.h>
#include <unistd.h>


int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  mappi::MmirzWidget* w = new mappi::MmirzWidget;
  w->show();

  app.exec();
  return EXIT_SUCCESS;
}
