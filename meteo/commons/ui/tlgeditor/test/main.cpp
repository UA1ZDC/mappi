#include <qtextcodec.h>
#include <qmessagebox.h>
#include <qapplication.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <meteo/product/ui/tlgeditor/tlgeditorwindow.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))


const QStringList kHelpOpt = QStringList() << "?" << "help";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
     ;

int main( int argc, char** argv )
{
  TAPPLICATION_NAME( "meteo" );
  ::meteo::gSettings(new ::meteo::Settings);


  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app(argc, argv);

  TArg args(argc, argv);

  try {
    if( args.contains(kHelpOpt) ){
      kHelp.print();
      return EXIT_SUCCESS;
    }

    if( !meteo::gSettings()->load() ){
      return EXIT_FAILURE;
    }

    meteo::TlgEditorWindow window;
    window.show();

    app.exec();
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
