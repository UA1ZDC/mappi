#include "tcolorgrad_test.h"
#include "../tcolorgrad.h"

#include <cross-commons/debug/tlog.h>

CPPUNIT_TEST_SUITE_REGISTRATION( TColorGrad_Test );


std::ostream & operator <<(std::ostream & os, const QColor& c)
{
  os << "rgb(" << c.red() << ',' << c.green() << ',' << c.blue() << ')';
  return os;
}


void TColorGrad_Test::colorList()
{
  TColorGradList c;
  c.append( TColorGrad(.0,QColor(0,100,0)) );
  c.append( TColorGrad(.1,QColor(0,150,0)) );

  TColorGradList colors;
  for( double i=.0; i<=.5; i+=0.1 ){
    colors.append( TColorGrad(i, i+0.1, c.color(i)) );
  }

  CPPUNIT_ASSERT_EQUAL( c.color(0.0), colors.color(.0) );
  CPPUNIT_ASSERT_EQUAL( c.color(0.0), colors.color(.05) );
  CPPUNIT_ASSERT_EQUAL( c.color(0.0), colors.color(.1) );

  CPPUNIT_ASSERT_EQUAL( c.color(0.1), colors.color(.15) );
  CPPUNIT_ASSERT_EQUAL( c.color(0.1), colors.color(.2) );
}
