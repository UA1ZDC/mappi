#include "autotest.h"

#include <meteo/commons/msgparser/msgfilter.h>
#include <meteo/commons/proto/msgfilter.pb.h>
#include <meteo/commons/proto/msgparser.pb.h>

#include <qstring.h>
#include <qbytearray.h>

using namespace meteo;

//
//
//

class Test_MsgFilter : public CppUnit::TestCase
{
  //!
  tlg::MessageNew createMessage(const QString& tt = QString(),
                                 const QString& aa = QString(),
                                 const QString& cccc = QString(),
                                 meteo::tlg::Format format = meteo::tlg::kWMO)
  {
    tlg::MessageNew msg;

    if ( tt.size() > 0 ) { msg.mutable_header()->set_t1(tt.left(1).toUtf8().constData()); }
    if ( tt.size() > 1 ) { msg.mutable_header()->set_t2(tt.mid(1,1).toUtf8().constData()); }

    if ( aa.size() > 0 ) { msg.mutable_header()->set_a1(aa.left(1).toUtf8().constData()); }
    if ( aa.size() > 1 ) { msg.mutable_header()->set_a2(aa.mid(1,1).toUtf8().constData()); }

    if ( !cccc.isNull() )   { msg.mutable_header()->set_cccc(cccc.toUtf8().constData()); }

    msg.set_format(format);

    return msg;
  }

public:
  void noRules();
  void allowRegExp();
  void denyRegExp();
  void fromProto();
  void fromString();
  void exactMatch();
  void repeated();

private:
  CPPUNIT_TEST_SUITE( Test_MsgFilter );
  CPPUNIT_TEST( noRules );
  CPPUNIT_TEST( allowRegExp );
  CPPUNIT_TEST( denyRegExp );
  CPPUNIT_TEST( fromProto );
  CPPUNIT_TEST( fromString );
  CPPUNIT_TEST( exactMatch );
  CPPUNIT_TEST( repeated );
  CPPUNIT_TEST_SUITE_END();
};

T_REGISTER_TEST( Test_MsgFilter, "filter" );

//
//
//

//! CASE: Пустой фильтр пропускает все сообщения.
void Test_MsgFilter::noRules()
{
  MsgFilter filter;

  CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage()) );
  CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage("TT", "AA", "CCCC")) );
}

//! CASE: Разрешающий фильтр. Фильтрация по регулярному выражению.
void Test_MsgFilter::allowRegExp()
{
  //! CASE: Поле TT.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kTT, QRegExp("[A-B]{1}G"));

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage()) );

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage("AG")) );
    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage("BG")) );

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("CG")) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("AK")) );
  }

  //! CASE: Поле AA.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kAA, QRegExp("G."));

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage()) );

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), "G1")) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage(QString(), "CG")) );
  }

  //! CASE: Поле CCCC.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kCCCC, QRegExp("..M."));

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage()) );

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), QString(), "MMMM")) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage(QString(), QString(), "AAAA")) );
  }

  //! CASE: Поле format.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kFormat, QRegExp("(kWMO|kOther)"));

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), QString(), QString())) );
    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), QString(), QString(), tlg::kOther)) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage(QString(), QString(), QString(), tlg::kGMS)) );
  }
}

//! CASE: Запрещающий фильтр. Фильтрация по регулярному выражению.
void Test_MsgFilter::denyRegExp()
{
  //! CASE: Поле TT.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kTT, QRegExp("[A-B]{1}G"), true);

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage()) );

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("AG")) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("BG")) );

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage("CG")) );
    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage("AK")) );
  }

  //! CASE: Поле AA.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kAA, QRegExp("G."), true);

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage()) );

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage(QString(), "G1")) );
    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), "CG")) );
  }

  //! CASE: Поле CCCC.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kCCCC, QRegExp("..M."), true);

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage()) );

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage(QString(), QString(), "MMMM")) );
    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), QString(), "AAAA")) );
  }

  //! CASE: Поле format.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kFormat, QRegExp(".GMS"), true);

    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), QString(), QString())) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage(QString(), QString(), QString(), tlg::kGMS)) );
    CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage(QString(), QString(), QString(), tlg::kOther)) );
  }
}

//! CASE: Установка правил фильтрации из прото-сообщения.
void Test_MsgFilter::fromProto()
{
  FilterOptions opt;
  opt.mutable_allow()->add_tt()->set_regexp(".T");
  opt.mutable_allow()->add_cccc()->set_regexp("AABB");

  opt.mutable_deny()->add_tt()->set_regexp("G.");
  opt.mutable_deny()->add_aa()->set_regexp(".A");

  MsgFilter filter;
  filter.setRules(opt);

  CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage("AT", "DD", "AABB")) );
  CPPUNIT_ASSERT_EQUAL( true, filter.allow(createMessage("AT", "DD", "AAAA")) );
  CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("AT", "AA", "AABB")) );
  CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("GT", "DD", "AABB")) );
  CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("GT", "AA", "AABB")) );
}

void Test_MsgFilter::fromString()
{
  QMap<QString,QString> data;
  data.insert("regexp:[A-Z]{1}.*", "[A-Z]{1}.*");
  data.insert("regexp:T:A:B", "T:A:B");
  data.insert(".*", QString());

  foreach ( const QString& input, data.keys() ) {
    FilterRule rule;
    MsgFilter::ruleFromString(&rule, input);

    CPPUNIT_ASSERT_EQUAL( data[input], QString::fromUtf8(rule.regexp().c_str()) );
  }
}

void Test_MsgFilter::exactMatch()
{
  FilterOptions opt;
  opt.mutable_allow()->add_tt()->set_regexp("WG");
  opt.mutable_allow()->add_cccc()->set_regexp("ABCD");

  opt.mutable_deny()->add_aa()->set_regexp("DD");

  MsgFilter filter;
  filter.setRules(opt);

  CPPUNIT_ASSERT_EQUAL( true,  filter.exactMatch(createMessage("WG", "AA", "ABCD")) );
  CPPUNIT_ASSERT_EQUAL( false, filter.exactMatch(createMessage("WG", "DD", "ABCD")) );
}

//! CASE: Несколько правил для одного поля
void Test_MsgFilter::repeated()
{
  //! CASE: Поле TT.
  {
    MsgFilter filter;
    filter.addRule(MsgFilter::kTT, QRegExp("SA"));
    filter.addRule(MsgFilter::kTT, QRegExp("SD"));

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage(), true) );

    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("SA"), true) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("SD"), true) );
    CPPUNIT_ASSERT_EQUAL( false, filter.allow(createMessage("DD"), true) );
  }
}

