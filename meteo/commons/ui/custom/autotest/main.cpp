#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>

#include <qapplication.h>
#include <qtextcodec.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt = QStringList() << "?" << "help";
const QStringList kRunOpt  = QStringList() << "r" << "run";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка")
  << HELP(kRunOpt, "Запускает тесты зарегистрированные под указанным именем")
     ;

// Регистрация теста
// CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( <test-class>, "<run-name>" );
// #include <qstringlist.h>
// extern QStringList gRunTest;
// namespace  {
//   bool registerTest() { gRunTest.append("<run-name>"); return true; }
//     static bool r = registerTest();
// }

QStringList gRunTest;

int main(int argc, char** argv)
{
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app(argc, argv, false);
  TArg args(argc, argv);

  if ( args.contains(kHelpOpt) ) {
    kHelp.print();
    return 0;
  }

  if ( args.contains(kRunOpt) ) {
    gRunTest.clear();
    gRunTest = args.values(kRunOpt);
  }

  if ( gRunTest.isEmpty() ) {
    std::cout << "NO TEST TO RUN. ADD ANY TO runTest in main.cpp\n";
    return 1;
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
