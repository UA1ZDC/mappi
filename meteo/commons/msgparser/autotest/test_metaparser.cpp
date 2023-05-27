#include "autotest.h"

#include <qdatetime.h>
#include <qstringlist.h>

#include <meteo/commons/msgparser/msgmetainfo.h>

class Test_MetaParser : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_MetaParser );
  CPPUNIT_TEST( parse );
  CPPUNIT_TEST( calcConvertedDt );
  CPPUNIT_TEST( bug );
  CPPUNIT_TEST( bugFuture );
  CPPUNIT_TEST_SUITE_END();
public:
  void parse();
  void calcConvertedDt();
  void bug();
  void bugFuture();
};

T_REGISTER_TEST( Test_MetaParser, "parser" );

//
//
//

using namespace meteo;

class InputBox
{
public:
  InputBox(const QString& name, const QString& t, const QString& dt) :
    fileName(name),
    templ(t),
    result(dt)
  {}

  QString fileName;
  QString templ;
  QString result;
};

void Test_MetaParser::parse()
{
  QList<InputBox> in;
  in << InputBox("200101012359", "{YYYY}{MM}{DD}{hh}{mm}", "2001-01-01T23:59:00Z" );
  in << InputBox("200101012359.tlg", "{YYYY}{MM}{DD}{hh}{mm}", "2001-01-01T23:59:00Z" );

  for ( int i=0,isz=in.size(); i<isz; ++i ) {
    MsgMetaInfo info(in[i].templ, in[i].fileName);
    QDateTime dt = info.calcConvertedDt();
    CPPUNIT_ASSERT_EQUAL( in[i].result, dt.toString(Qt::ISODate) );
  }

  in.clear();
  in << InputBox("gda01-synop-201511191305.txt", "gda01-synop-{YYYY}{MM}{DD}", "2015-11-19");
  in << InputBox("text--2015-text-11=text=191305", "text--{YYYY}-text-{MM}=text={DD}", "2015-11-19");
  in << InputBox("text--2015-text-28=text=011305", "text--{YYYY}-text-{DD}=text={MM}", "2015-01-28");
  in << InputBox("FDP/Z_SAT+2017010914", "FDP/Z_SAT+{YYYY}{MM}{DD}", "2017-01-09");
  in << InputBox("path/20010101", "*****{YYYY}{MM}{DD}", "2001-01-01" );

  for ( int i=0,isz=in.size(); i<isz; ++i ) {
    MsgMetaInfo info(in[i].templ, in[i].fileName);
    QDate d = info.calcConvertedDt().date();
    CPPUNIT_ASSERT_EQUAL( in[i].result, d.toString(Qt::ISODate) );
  }

  in.clear();
  in << InputBox("UAXX_11_EDZW_122_201251", "{T1}{T2}{A1}{A2}_{ii}_{CCCC}_{J}_{YYGGgg}", "U-A-X-X-11-EDZW-201251");

  for ( int i=0,isz=in.size(); i<isz; ++i ) {
    meteo::MsgMetaInfo info(in[i].templ, in[i].fileName);
    QStringList ttaaccc = QStringList()
                          << info.t1
                          << info.t2
                          << info.a1
                          << info.a2
                          << info.ii
                          << info.cccc
                          << info.yygggg
                             ;
    CPPUNIT_ASSERT_EQUAL( in[i].result, ttaaccc.join("-") );
  }
}

void Test_MetaParser::calcConvertedDt()
{
  // TEST: Проверяем вычисление даты формирования по значению YYGGgg.
  const QDateTime begDt(QDate(1997,1,1), QTime(0,0,0), Qt::UTC);
  const QDateTime endDt(QDate(2003,1,1), QTime(0,0,0), Qt::UTC);

  QDateTime curDt = begDt;
  while ( curDt != endDt ) {
    MsgMetaInfo meta;

    // проверяем вычисление даты телеграмм пришедших с опозданием (относительно даты curDt)
    QDateTime pastDt = curDt.addDays(-20);
    for ( QDateTime iDt=pastDt; iDt<curDt; iDt = iDt.addDays(1) ) {
      meta.yygggg = iDt.toString("ddhhmm");
      QDateTime dt = meta.calcConvertedDt(curDt);

      CPPUNIT_ASSERT_EQUAL( iDt.toString(Qt::ISODate), dt.toString(Qt::ISODate) );
    }

    // проверяем вычисление даты телеграмм пришедших с небольшим опережением (относительно даты curDt)
    QDateTime futureDt = curDt.addSecs(120);
    meta.yygggg = futureDt.toString("ddhhmm");
    QDateTime dt = meta.calcConvertedDt(curDt);

    CPPUNIT_ASSERT_EQUAL( futureDt.toString("yyyy-MM-dd hh"), dt.toString("yyyy-MM-dd hh") );

    curDt = curDt.addDays(1);
  }
}

void Test_MetaParser::bug()
{
  // BUG: Т.к. при инициализации переменной хранящей дату/время используется текущая дата, а заполняется
  //      по порядку (год, месяц, число), то значение переменной становится невалидным когда устанавливается
  //      месяц, в котором меньше дней, чем текущий день.
  QList<InputBox> in;
  in << InputBox("20020202_0000.tlg", "{YYYY}{MM}{DD}_{hh}{mm}", "2002-02-02" );

  for ( int i=0,isz=in.size(); i<isz; ++i ) {
    MsgMetaInfo info(in[i].templ, in[i].fileName);
    QDateTime dt = info.calcConvertedDt();
    CPPUNIT_ASSERT_EQUAL( in[i].result, dt.toString("yyyy-MM-dd") );
  }
}

void Test_MetaParser::bugFuture()
{
  // BUG: Если год не определяется из имени файла, то неправильно определяется дата.
  const QDateTime begDt(QDate(2001,1,1), QTime(0,10,0), Qt::UTC);
  const QDateTime endDt(QDate(2003,1,1), QTime(0,10,0), Qt::UTC);

  QDateTime curDt = begDt;
  while ( curDt != endDt ) {
    MsgMetaInfo meta;

    QList<InputBox> in;
    in << InputBox(curDt.toString("ddhhmm"), "{DD}{hh}{mm}", curDt.toString("yyyy-MM-dd hh:mm"));

    for ( int i=0,isz=in.size(); i<isz; ++i ) {
      MsgMetaInfo meta(in[i].templ, in[i].fileName);
      QDateTime dt = meta.calcConvertedDt(curDt);
      CPPUNIT_ASSERT_EQUAL( in[i].result, dt.toString("yyyy-MM-dd hh:mm") );
    }

    curDt = curDt.addDays(1);
  }
}
