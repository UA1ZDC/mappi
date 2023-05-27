#ifndef TXMLPROTO_TEST_H
#define TXMLPROTO_TEST_H


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TXmlProto_Test : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( TXmlProto_Test );
  CPPUNIT_TEST( toXml );
  CPPUNIT_TEST( toXmlWithMessage );
  CPPUNIT_TEST( toXmlWithRepeatedMessage );
  CPPUNIT_TEST( toXmlEscape );
  CPPUNIT_TEST( toXmlMultiMessages );
  CPPUNIT_TEST( toXmlSeveralSimilarFields );
  CPPUNIT_TEST( toXmlGroup );
  CPPUNIT_TEST( fromXml );
  CPPUNIT_TEST( fromXmlWithMessage );
  CPPUNIT_TEST( fromXmlWithRepeatedMessage );
  CPPUNIT_TEST( fromXmlSeveralSimilarFields );
  CPPUNIT_TEST_SUITE_END();

public:
  void toXml();
  void toXmlWithMessage();
  void toXmlWithRepeatedMessage();

  void toXmlEscape();
  void toXmlMultiMessages();

  void toXmlSeveralSimilarFields();
  void toXmlGroup();

  void fromXml();
  void fromXmlWithMessage();
  void fromXmlWithRepeatedMessage();

  void fromXmlSeveralSimilarFields();
};


#endif // TXMLPROTO_TEST_H
