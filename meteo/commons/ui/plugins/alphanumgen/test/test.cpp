#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <../kn01.h>
#include <../kn04.h>
#include <../kn03.h>

#include <meteo/novost/settings/tmeteosettings.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qpixmap.h>

#include <stdlib.h>
#include <unistd.h>

void help(const QString& progName)
{  
  info_log << QObject::tr("\nИспользование:") << progName << "[-123]\n"
    "-1 kn01\n"
    "-2 kn04\n"
    "-3 kn03\n";
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  meteo::settings::TMeteoSettings::instance()->load();

  int flag = 0, opt;
  while ((opt = getopt(argc, argv, "123h")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      break;
    case '2':
      flag = 2;
      break;
    case '3':
      flag = 3;
      break;
    case 'h':
      help(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      error_log << "Option" << opt << "not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
    error_log << QObject::tr("Необходимо задать опцию");
    help(argv[0]);
    exit(-1);
  }

  info_log << QObject::tr("\nТест собирается с исходниками, а не подключает библиотеку!\n");

  switch (flag) {
  case 1: {
    meteo::Kn01* editor = new meteo::Kn01;
    editor->show();
  }
    break;
  case 2: {
    meteo::Kn04* editor4 = new meteo::Kn04;
    editor4->show();
  }
    break;
  case 3: {
    meteo::Kn03* editor3 = new meteo::Kn03;
    editor3->show();
  }
    break;

  default: {}
  }

  app.exec();
  return EXIT_SUCCESS;
}
