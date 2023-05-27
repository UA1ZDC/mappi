#include <meteo/novost/ui/settings/settingsform.h>

#include <cross-commons/app/paths.h>

#include <qapplication.h>
#include <qtextcodec.h>

int main( int argc, char** argv )
{
  TAPPLICATION_NAME( "meteo" );

//  QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "KOI8-R" ) );
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  QApplication app( argc, argv );

  TSettingsForm* dlg = new TSettingsForm;
  app.setActiveWindow(dlg);
  dlg->showMaximized();

//  TMainWindow mwindow = new TMainWindow;
//  app.setActiveWindow(mwindow);

  return app.exec();;
}
