#include "autotest.h"

#include <meteo/commons/ui/custom/coordedit.h>

//
class Test_CoordData : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_CoordData );

  CPPUNIT_TEST( dmsFromString );
  CPPUNIT_TEST( checkFormat );
  CPPUNIT_TEST( decDegree );

  CPPUNIT_TEST_SUITE_END();

public:
  void dmsFromString();
  void checkFormat();
  void decDegree();
};


REGISTER_TEST( Test_CoordData, "coord" );

inline std::ostream & operator <<(std::ostream & os, const CoordData& d)
{
  os << "deg: " << QString::number(d.degree).toStdString()
     << "  min: " << QString::number(d.minute).toStdString()
     << "  sec: " << QString::number(d.second).toStdString()
        ;
  return os;
}

//
//
//
//


#define UTF( txt ) QString::fromUtf8( txt )

void Test_CoordData::dmsFromString()
{
  // CASE: разбор строки
  {
    CoordData expected(100, 50, 1);

    QStringList list = QStringList()
      << UTF(" 100°  50' 1\"")
      << UTF("100 50 1")
         ;

    foreach ( const QString& s, list ) {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( s.toLocal8Bit().constData(), expected, CoordData::fromString(s) );
    }
  }
  // CASE: разбор неполной строки
  {
    QMap<QString, CoordData> m;
    m[UTF("100")]           = CoordData(100, 0, 0);
    m[UTF("100°")]          = CoordData(100, 0, 0);
    m[UTF("100° 50'")]      = CoordData(100, 50, 0);
    m[UTF("100° 50' 1\"")]  = CoordData(100, 50, 1);
    m[UTF("1°'\"")]   = CoordData(1, 0, 0);
    m[UTF("1°2'\"")]  = CoordData(1, 2, 0);
    m[UTF("1°'3\"")]  = CoordData(1, 0, 3);
    m[UTF("°'3\"")]   = CoordData(0, 0, 3);
    m[UTF("°2'3\"")]  = CoordData(0, 2, 3);

    foreach ( const QString& s, m.keys() ) {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( s.toLocal8Bit().constData(), m[s], CoordData::fromString(s) );
    }
  }
}

void Test_CoordData::checkFormat()
{
  // CASE: корректные
  {
    bool e = true;

    QStringList list = QStringList()
      << QString::fromUtf8("")
      << QString::fromUtf8("°")
      << QString::fromUtf8("°'")
      << QString::fromUtf8("°'\"")
      << QString::fromUtf8("100° 50' 1\"")
      << QString::fromUtf8("100 50 1")
         ;

    foreach ( const QString& s, list ) {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( s.toLocal8Bit().constData(), e, CoordData::checkFormat(s) );
    }
  }

  // CASE: некорректные
  {
    bool e = false;

    QStringList list = QStringList()
      << QString::fromUtf8("100 50' 1")
          ;

    foreach ( const QString& s, list ) {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( s.toLocal8Bit().constData(), e, CoordData::checkFormat(s) );
    }
  }
}

void Test_CoordData::decDegree()
{
  for ( int d=0; d<360; ++d ) {
    for ( int m=0; m<60; ++m ) {
      for ( int s=0; s<60; ++s ) {
        QString str = QString("%1 %2 %3").arg(d).arg(m).arg(s);
        CoordData data1 = CoordData::fromString(str);

        CoordData data2;
        data2.setDecDegree(data1.toDecDegree());
//        var(str) << QString::number(data1.toDecDegree(), 'f', 10);

        CPPUNIT_ASSERT_EQUAL_MESSAGE( str.toLocal8Bit().constData(), data1, data2 );

      }
    }
  }
}
