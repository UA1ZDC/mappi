#include "autotest.h"

//
class Test_Timesheet : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_Timesheet );

  CPPUNIT_TEST( fromString );
  CPPUNIT_TEST( match );
  CPPUNIT_TEST( prevRun );

  CPPUNIT_TEST_SUITE_END();

public:
  void fromString();
  void match();
  void prevRun();
};


T_REGISTER_TEST( Test_Timesheet, "timesheet" );

//
//

#include <meteo/commons/planner/timesheet.h>

//! TEST:
void Test_Timesheet::fromString()
{
  // list : expected - input
  typedef QPair<QString,meteo::Timesheet> DataItem;
  QList<DataItem> data;
  data << qMakePair( QString(), meteo::Timesheet() );
  data << qMakePair( QString("* * * * *"), meteo::Timesheet::fromString(" *  *\t\t * * *  ") );
  // minute
  data << qMakePair( QString(), meteo::Timesheet::fromString("60 * * * *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("10-3 * * * *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("1,4,60 * * * *") );
  // hour
  data << qMakePair( QString(), meteo::Timesheet::fromString("* 24 * * *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* 10-3 * * *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* 1,4,60 * * *") );
  // day
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * 0 * *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * 5-3 * *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * 1,4,60 * *") );
  // month
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * * 0 *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * * 5-3 *") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * * 1,4,60 *") );
  // day-of-week
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * * * 0") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * * * 5-3") );
  data << qMakePair( QString(), meteo::Timesheet::fromString("* * * * 1,4,10") );
  // complex
  data << qMakePair( QString("00 12 1,5,20 1-5 */2"), meteo::Timesheet::fromString("00 12 1,5,20 1-5 */2") );

  foreach ( const DataItem& item, data ) {
    CPPUNIT_ASSERT_EQUAL( item.first, item.second.toString() );
  }
}

//! TEST:
void Test_Timesheet::match()
{
  // date time and match result
  typedef QPair<QString,bool> TimeResult;
  // timesheet string and list of input date time and match result
  typedef QPair<QString,QList<TimeResult> > DataItem;

  QList<DataItem> data;
  data << qMakePair(QString("* * * * *"), QList<TimeResult>() << qMakePair(QString("2015-01-05T00:00:00"), true)
                                                              << qMakePair(QString(), false)
                    );
  // minute
  data << qMakePair(QString("05 * * * *"),  QList<TimeResult>() << qMakePair(QString("2015-01-05T00:05:00"), true)
                                                                << qMakePair(QString("2015-01-05T00:15:00"), false)
                    );
  data << qMakePair(QString("5-10 * * * *"),  QList<TimeResult>() << qMakePair(QString("2015-01-05T00:05:00"), true)
                                                                  << qMakePair(QString("2015-01-05T00:10:00"), true)
                                                                  << qMakePair(QString("2015-01-05T00:15:00"), false)
                    );
  data << qMakePair(QString("1,3,5 * * * *"), QList<TimeResult>() << qMakePair(QString("2015-01-05T00:03:00"), true)
                                                                  << qMakePair(QString("2015-01-05T00:15:00"), false)
                    );
  data << qMakePair(QString("*/16 * * * *"),  QList<TimeResult>() << qMakePair(QString("2015-01-05T00:08:00"), false)
                                                                  << qMakePair(QString("2015-01-05T00:16:00"), true)
                                                                  << qMakePair(QString("2015-01-05T00:32:00"), true)
                    );
  // complex
  data << qMakePair(QString("* 12-14 1 */3 *"),  QList<TimeResult>() << qMakePair(QString("2015-03-01T12:05:00"), true)
                                                                     << qMakePair(QString("2015-06-05T10:15:00"), false)
                    );


  foreach ( const DataItem& item, data ) {
    meteo::Timesheet timesheet = meteo::Timesheet::fromString(item.first);
    foreach ( const TimeResult& v, item.second ) {
      QDateTime dateTime = QDateTime::fromString(v.first, Qt::ISODate);
      QString msg = item.first + "  " + v.first;
      CPPUNIT_ASSERT_EQUAL_MESSAGE( msg.toStdString(), v.second, timesheet.match(dateTime) );
    }
  }
}

void Test_Timesheet::prevRun()
{
  meteo::Timesheet timesheet = meteo::Timesheet::fromString("10 3 * * *");

  QDateTime prev = timesheet.prevRun(QDateTime::fromString("2010-03-12 15:20", "yyyy-MM-dd hh:mm"));
  CPPUNIT_ASSERT_EQUAL( QString("2010-03-12 03:10"), prev.toString("yyyy-MM-dd hh:mm") );

  prev = timesheet.prevRun(QDateTime::fromString("2010-03-12 03:10", "yyyy-MM-dd hh:mm"));
  CPPUNIT_ASSERT_EQUAL( QString("2010-03-11 03:10"), prev.toString("yyyy-MM-dd hh:mm") );
}
