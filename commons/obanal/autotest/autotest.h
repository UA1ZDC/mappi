#ifndef COMMONS_OBANAL_AUTOTEST_AUTOTEST_H
#define COMMONS_OBANAL_AUTOTEST_AUTOTEST_H

// Макрос для регистрации тестов.
//
// Пример:
// #include "autotest.h"
// class Test1 : public CppUnit::TestCase {};
// T_REGISTER_TEST( Test1, "mytest" );

#include <qstringlist.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#define T_REGISTER_TEST( class_name, test_name )                              \
  extern QStringList gRunTest;                                                \
                                                                              \
  namespace  {                                                                \
    bool registerTest() { gRunTest.append(test_name); return true; }          \
    static bool r = registerTest();                                           \
  }                                                                           \
  CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( class_name, test_name )

//
//
//

#include <qeventloop.h>
#include <qelapsedtimer.h>
#include <qtemporaryfile.h>

#include <cross-commons/debug/tlog.h>
#include <commons/testhelper/tcppunit.h>
#include <commons/testhelper/ttesthelper.h>

//!
inline void processEvents(int msec)
{
  QElapsedTimer t;
  t.start();
  QEventLoop loop;
  while ( t.elapsed() < msec ) {
    loop.processEvents();
  }
}

#endif // COMMONS_OBANAL_AUTOTEST_AUTOTEST_H
