
#ifndef CTEST_H
#define CTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class CTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( CTest );
  CPPUNIT_TEST( f1 );
  CPPUNIT_TEST( f2 );
  CPPUNIT_TEST( f3 );
  CPPUNIT_TEST( f4 );
  CPPUNIT_TEST_SUITE_END();

public:
  void f1();
  void f2();
  void f3();
  void f4();
};

#endif // CTEST_H

