#include "autotest.h"

class Test_TlgFuncs : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_TlgFuncs );
  CPPUNIT_TEST( readTelegram );
  CPPUNIT_TEST( badPath );
  CPPUNIT_TEST( convertedDt );
  CPPUNIT_TEST_SUITE_END();
public:
  void readTelegram();
  void badPath();
  void convertedDt();
};

T_REGISTER_TEST( Test_TlgFuncs, "funcs" );

#include <meteo/commons/msgparser/tlgfuncs.h>

#include <qstring.h>
#include <qbytearray.h>


void Test_TlgFuncs::readTelegram()
{
  const QByteArray tlgText = "Hello, this is tlg!\n";
  const QByteArray tarTlgText = "Hi, I'm from tgz!\n";
  const QByteArray nullBa;

  CPPUNIT_ASSERT_EQUAL( tlgText, meteo::readTelegram("./data/test.tlg") );
  CPPUNIT_ASSERT_EQUAL( tarTlgText, meteo::readTelegram("tar://data/test.tlg.tgz:data/test.tlg") );

  CPPUNIT_ASSERT_EQUAL( nullBa, meteo::readTelegram("data/null.tlg") );

  CPPUNIT_ASSERT_EQUAL( nullBa, meteo::readTelegram("tar://data/test.tlg.tgz:data/null.tlg") );
  CPPUNIT_ASSERT_EQUAL( nullBa, meteo::readTelegram("tar://data/null.tgz:data/null.tlg") );
}

void Test_TlgFuncs::badPath()
{
  CPPUNIT_ASSERT_EQUAL( QByteArray(), meteo::readTelegram("tar://data/test.tlg.tgz") );
}

void Test_TlgFuncs::convertedDt()
{
  const int kMetaDt = 0;
  const int kYYGGGG = 1;
  const int kDbDt   = 2;

  const QList<QStringList> data = QList<QStringList>()
      << (QStringList() << "2015-01-01T00:00:00Z" << "011200" << "2015-01-01T12:00:00Z")
      << (QStringList() << "2015-02-07T00:00:00Z" << "251200" << "2015-01-25T12:00:00Z")
      << (QStringList() << "2015-01-01T00:00:00Z" << "311200" << "2014-12-31T12:00:00Z")
         ;

  foreach ( QStringList v, data ) {
    QDateTime dbDt = meteo::convertedDateTime( QDateTime::fromString(v[kMetaDt],Qt::ISODate), v[kYYGGGG] );

    CPPUNIT_ASSERT_EQUAL( v[kDbDt], dbDt.toString(Qt::ISODate) );
  }

  const QList<QStringList> invalidData = QList<QStringList>()
      << (QStringList() << "2015-01-01T00:00:00Z" << "500906")
      << (QStringList() << "" << "311200")
      << (QStringList() << "" << "")
         ;

  foreach ( QStringList v, invalidData ) {
    QDateTime dbDt = meteo::convertedDateTime(
          QDateTime::fromString(v[kMetaDt],Qt::ISODate),
          v[kYYGGGG] );

    CPPUNIT_ASSERT_EQUAL( false, dbDt.isValid() );
  }
}
