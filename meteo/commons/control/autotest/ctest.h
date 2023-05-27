#ifndef CTEST_H
#define CTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TMeteoData;

class CTest : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( CTest );
  CPPUNIT_TEST( control_limitsurface );
  CPPUNIT_TEST( control_limitair );
  CPPUNIT_TEST( control_consistsurf );
  CPPUNIT_TEST( control_consistair );
  CPPUNIT_TEST( control_sign );
  CPPUNIT_TEST( control_wind );
  CPPUNIT_TEST( control_time );
  CPPUNIT_TEST_SUITE_END();

public:
  void control_limitsurface();
  void control_limitair();
  void control_consistsurf();
  void control_consistair();
  void control_sign();
  void control_wind();
  void control_time();

private:
  void fillair(TMeteoData* data1, TMeteoData* data2);
  void fill_signair(TMeteoData* data1, TMeteoData* data2);
};

#endif // CTEST_H
