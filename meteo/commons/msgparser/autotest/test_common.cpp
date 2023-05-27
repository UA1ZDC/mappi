#include "autotest.h"

#include <qdatetime.h>
#include <qstringlist.h>

#include <meteo/commons/msgparser/common.h>

class Test_Common : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_Common );
  CPPUNIT_TEST( proto2tlg );
  CPPUNIT_TEST_SUITE_END();
public:
  void proto2tlg();
};

T_REGISTER_TEST( Test_Common, "common" );

//
//
//

// TEST: формирование телеграммы из полей прото-сообщения
void Test_Common::proto2tlg()
{
  // CASE: циркулярное сообщений ВМО
  {
    meteo::tlg::MessageNew msg;
    msg.set_format(meteo::tlg::kWMO);
    msg.set_addrmode(meteo::tlg::kCircular);
    msg.mutable_header()->set_t1("T");
    msg.mutable_header()->set_t2("E");
    msg.mutable_header()->set_a1("S");
    msg.mutable_header()->set_a2("T");
    msg.mutable_header()->set_ii(32);
    msg.mutable_header()->set_cccc("FROG");
    msg.set_msg("hello world!\r\n");

    QByteArray raw = meteo::tlg::proto2tlg(msg);

    meteo::tlg::MessageNew parsed = meteo::tlg::tlg2proto(raw);

    CPPUNIT_ASSERT_EQUAL( parsed.SerializeAsString(), msg.SerializeAsString() );
  }
}
