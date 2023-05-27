#include "autotest.h"
#include "test_pbhelper.pb.h"

#include <commons/textproto/pbtools.h>


//
class Test_PbHelper : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_PbHelper );

  CPPUNIT_TEST( setValueQString );
  CPPUNIT_TEST( setValueQByteArray );

  CPPUNIT_TEST_SUITE_END();

public:
  void setValueQString();
  void setValueQByteArray();
};



REGISTER_TEST( Test_PbHelper, "helper" );

//
//
//
//

//! TEST: Заполнение поля из QString
void Test_PbHelper::setValueQString()
{
  //! CASE: Заполнение одноуровневой структуры.
  {
    Simple msg;
    PbHelper help(&msg);

    QString str = QObject::tr("Текст!");
    QString num = "124";
    QString dbl = "12.4";
    QString enm = "kTwo";

    CPPUNIT_ASSERT( help.setValue(QStringList("str_field"), str) );
    CPPUNIT_ASSERT_EQUAL( str, pbtools::toQString(msg.str_field()) );

    CPPUNIT_ASSERT( help.setValue(QStringList("int_field"), num) );
    CPPUNIT_ASSERT_EQUAL( num.toInt(), msg.int_field() );

    CPPUNIT_ASSERT( help.setValue(QStringList("dbl_field"), dbl) );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dbl.toDouble(), msg.dbl_field(), .2 );

    CPPUNIT_ASSERT( help.setValue(QStringList("enm_field"), enm) );
    CPPUNIT_ASSERT_EQUAL( enm, QString::fromUtf8(TEnum_Name(msg.enm_field()).c_str()) );

    CPPUNIT_ASSERT( false == help.setValue(QStringList("_none_field_"), QString()) );
  }


  // TODO: тестировать repeted поля
  // TODO: тестировать заполнение вложенных структур
}

//! TEST: Заполнение поля из QByteArray
void Test_PbHelper::setValueQByteArray()
{
  //! CASE: Заполнение одноуровневой структуры.
  {
    Simple msg;
    PbHelper help(&msg);

    QByteArray ba = "raw data";

    CPPUNIT_ASSERT( help.setValue(QStringList("bytes_field"), ba) );
    CPPUNIT_ASSERT_EQUAL( ba, pbtools::fromBytes(msg.bytes_field()) );
  }
}
