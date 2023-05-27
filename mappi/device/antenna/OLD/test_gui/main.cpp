#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <mappi/device/antenna/smallant.h>
#include <smallantwidget.h>

#include <qapplication.h>
#include <qtextcodec.h>

int main( int argc, char* argv[] )
{
 TAPPLICATION_NAME("meteo");
 QApplication app( argc, argv );
 //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

 mappi::receive::SmallAntWidget ant;
 ant.show();
  
 app.exec();

 return 0;
}
