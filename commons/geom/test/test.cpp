#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qline.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/geom/geom.h>

int main( int argc, char* argv[] )
{
  QApplication app(argc, argv, false );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QLine l1( QPoint(2,10), QPoint(-10,10) );
  QLine l2( QPoint(10,5), QPoint(-10,10) );
  QPoint cross;
  bool res = meteo::geom::crossPoint(l1,l2,&cross);
  debug_log << "is crooss =" << res;
  debug_log << "pnt =" << cross;
  return EXIT_SUCCESS;
}
