#include "sateliteimagebuilder.h"
#include "faxbuilder.h"

#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>

#include <qcoreapplication.h>
#include <meteo/commons/settings/settings.h>

static const QStringList kOptHelp = (
      QStringList() << QObject::tr("h") << QObject::tr("help")      
      );

static const QStringList kOptFax = (
      QStringList() << QObject::tr("f") << QObject::tr("fax")
      );

static const QStringList kOptSatelite = (
      QStringList() << QObject::tr("s") << QObject::tr("satelite")
      );

static const HelpFormatter* createHelpFormatter(){
  HelpFormatter *helpFormatter = new HelpFormatter();
  *helpFormatter << ( QStringList() << kOptHelp << QObject::tr("Эта справка."));
  *helpFormatter << ( QStringList() << kOptFax << QObject::tr("Запустить сбор факсимильных карт"));
  *helpFormatter << ( QStringList() << kOptSatelite << QObject::tr("Запустить сбор спутниковых снимков"));
  return helpFormatter;
}

enum Mode {
  kFax
};

static const auto kHelp = createHelpFormatter();

int appMain( int argc, char** argv )
{
  TAPPLICATION_NAME("meteo");
  gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }
  meteo::global::setLogHandler();
  QCoreApplication app(argc, argv );

  TArg args( argc, argv );

  if ( true == args.contains(kOptHelp) ) {
    kHelp->print();
    return EXIT_SUCCESS;
  }

  bool isFaxLoader = args.contains(kOptFax);
  bool isSateliteLoader = args.contains(kOptSatelite);
  if ( (true == isFaxLoader && true == isSateliteLoader) ||
       (false == isFaxLoader && false == isSateliteLoader) ){
    error_log << QObject::tr("Недопустимые параметры");
    kHelp->print();
    return EXIT_FAILURE;
  }

  info_log << QObject::tr("Сервис сборки документов успешно запущен.");

  meteo::GenericChainBuilder* builder = nullptr;
  if ( true == isFaxLoader ) {
    builder = new FaxBuilder();
  }
  if ( true == isSateliteLoader ){
    builder = new SateliteImageBuilder();
  }

  builder->init();

  int exitStatus = app.exec();

  delete builder;

  return exitStatus;
}
