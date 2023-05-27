#ifndef METEO_NOVOST_MSGCENTER_CORE_AUTOTEST_AUTOTEST_H
#define METEO_NOVOST_MSGCENTER_CORE_AUTOTEST_AUTOTEST_H

#include <qstringlist.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#define REGISTER_TEST( class_name, test_name )                                \
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

#endif // METEO_NOVOST_MSGCENTER_CORE_AUTOTEST_AUTOTEST_H
