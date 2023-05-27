#include <qapplication.h>
#include <qtextcodec.h>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>


int main(int argc, char** argv)
{
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app(argc, argv, false);

   CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

   CppUnit::TextTestRunner runner;
   runner.addTest(suite);
   runner.setOutputter(new CppUnit::CompilerOutputter( &runner.result(), std::cerr));

   bool wasSuccessful = runner.run();

   return wasSuccessful ? 0 : 1;
}
