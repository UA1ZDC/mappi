#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt = QStringList() << "?" << "help";
const QStringList kListOpt = QStringList() << "l" << "list";
const QStringList kRunOpt  = QStringList() << "r" << "run";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
  << HELP(kListOpt, "Список тестов.")
  << HELP(kRunOpt, "Запустить тесты для указанной группы.")
     ;

// Регистрация теста
// #include "autotest.h"
// T_REGISTER_TEST( <Test_Class>, <group> );

QStringList gRunTest;

int main(int argc, char** argv)
{
  TAPPLICATION_NAME("ukaz");

  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QCoreApplication app(argc, argv);
  TArg args(argc, argv);

  if ( args.contains(kHelpOpt) ) {
    kHelp.print();
    return 0;
  }

  if ( args.contains(kListOpt) ) {
    foreach ( const QString& t, gRunTest ) {
      std::cout << t.toLocal8Bit().constData() << "\n";
    }
    return 0;
  }

  if ( args.contains(kRunOpt) ) {
    gRunTest.clear();
    gRunTest = args.values(kRunOpt);
  }

  if ( gRunTest.isEmpty() ) {
    std::cout << QObject::tr("Внимание! Нет зарегистрированных тестов для запуска.\n").toLocal8Bit().constData();
    return 2;
  }

  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  foreach ( const QString& name, gRunTest ) {
    std::string testName(name.toLocal8Bit().constData());
    registry.registerFactory( &CppUnit::TestFactoryRegistry::getRegistry(testName) );
  }

  CppUnit::TextTestRunner runner;
  runner.addTest( registry.makeTest() );

  runner.setOutputter( new CppUnit::CompilerOutputter(&runner.result(), std::cerr) );

  bool wasSuccessful = runner.run();

  return wasSuccessful ? 0 : 1;
}
