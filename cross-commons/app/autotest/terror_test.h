#ifndef TERROR_TEST_H
#define TERROR_TEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TError_Test : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( TError_Test );
  CPPUNIT_TEST( error );
  CPPUNIT_TEST_SUITE_END();

public:
  void error();
};

#endif // TERROR_TEST_H
