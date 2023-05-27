#ifndef TCOLORGRAD_TEST_H
#define TCOLORGRAD_TEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TColorGrad_Test : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( TColorGrad_Test );
  CPPUNIT_TEST( colorList );
  CPPUNIT_TEST_SUITE_END();

public:
  void colorList();
};

#endif // TCOLORGRAD_TEST_H
