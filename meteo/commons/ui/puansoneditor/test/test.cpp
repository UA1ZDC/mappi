#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qpixmap.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/plugins/puansoneditor/editor.h>

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");
  QApplication app( argc, argv );
  qDebug() << "превед медвед";
//  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  meteo::puanson::Editor* editor = new meteo::puanson::Editor;
  editor->show();
  app.exec();
  return EXIT_SUCCESS;
}
