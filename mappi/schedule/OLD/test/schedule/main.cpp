#include <mappi/schedule/schedule.h>
#include <mappi/schedule/scheduleinfo.h>
#include <mappi/schedule/session.h>
#include <cross-commons/app/paths.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

using namespace mappi;
using namespace schedule;


//! информация о расписании
ScheduleInfo info() {
    ScheduleInfo sInfo;
    sInfo.setTLE(MnCommon::varPath("mappi") + "weather.txt");
    QDateTime dt = QDateTime::currentDateTimeUtc();
    dt = dt.addSecs( -3600 * 24 );
    sInfo.setStart( dt );
    dt = dt.addSecs( 3600 * 24 * 5 );
    sInfo.setStop( dt );
    Coords::GeoCoord gCoord;
    gCoord.lat = 59.97;
    gCoord.lon = 30.30;
    gCoord.alt = 6/1000;
    sInfo.setCoordStation( gCoord );
    QStringList satList;
    satList += "NOAA 18";
    satList += "NOAA 19";
    sInfo.setSatNamesList( satList );
    sInfo.setMinEl( 1 );
    sInfo.setMaxEl( 2 );
    sInfo.setRuleConflResolv( mappi::conf::kBeginPrior/*kContinPrior*//*kWestPrior*//*kEastPrior*/);

    sInfo.print();
    printf( "diffTimeRange=%g;\n", sInfo.diffTimeRange() );
    return sInfo;
}

//! создать расписание с решенными конфликтами
Schedule testschedule() {
    Schedule sched;
    ScheduleInfo sInfo = info();
    sched.create( sInfo );
    sched.resolveConfl( sInfo.getRuleConflResolv() );
    sched.print();

    printf( "count=%i;\n", sched.countSession() );
    printf( "aos=%s;\n", (const char*)sched.start().toString( Qt::ISODate ).toUtf8() );
    printf( "los=%s;\n", (const char*)sched.end().toString( Qt::ISODate ).toUtf8() );
    return sched;
}

//! сеансы
void session() {
    Schedule sched;
    sched.create( info() );
    sched.resolveConfl( info().getRuleConflResolv() );
    sched.print();

    Session session;
    session = sched.firstSession();
    printf( "FIRST SESSION\n" );
    printf( "%s\t", (const char*)session. getNameSat().toUtf8() );
    printf( "%s\t", (const char*)session.getAos().toString( Qt::ISODate ).toUtf8());
    printf( "%li\t", session.getRevol() );
    printf( "%i\n", (int)session.getState() );
    printf( "adopted=[%i];\n", session.adopted() );
    printf( "stage=%i;\n", (int)session.currentStage() );
    //    printf( "wait=%i;\n\n", session.diffTimeAos() );

    session = sched.lastSession();
    printf( "LAST SESSION\n" );
    printf( "%s\t", (const char*)session. getNameSat().toUtf8() );
    printf( "%s\t", (const char*)session.getAos().toString( Qt::ISODate ).toUtf8() );
    printf( "%li\t", session.getRevol() );
    printf( "%i\n", (int)session.getState() );
    printf( "adopted=[%i];\n", session.adopted() );
    printf( "stage=%i;\n", (int)session.currentStage() );
    //    printf( "wait=%i;\n\n", session.diffTimeAos() );

    session = sched.nearSession();
    printf( "NEAR SESSION\n" );
    printf( "%s\t", (const char*)session. getNameSat().toUtf8() );
    printf( "%s\t", (const char*)session.getAos().toString( Qt::ISODate ).toUtf8() );
    printf( "%li\t", session.getRevol() );
    printf( "%i\n", (int)session.getState() );
    printf( "adopted=[%i];\n", session.adopted() );
    printf( "stage=%i;\n", (int)session.currentStage() );
    //    printf( "wait=%i;\n", session.diffTimeAos() );
    printf( "number=%i;\n\n", sched.getNumberSession( session ) );

    session = sched.nextSession();
    printf( "NEXT SESSION\n" );
    printf( "%s\t", (const char*)session. getNameSat().toUtf8() );
    printf( "%s\t", (const char*)session.getAos().toString( Qt::ISODate ).toUtf8() );
    printf( "%li\t", session.getRevol() );
    printf( "%i\n", (int)session.getState() );
    printf( "adopted=[%i];\n", session.adopted() );
    printf( "stage=%i;\n", (int)session.currentStage() );
    //    printf( "wait=%i;\n", session.diffTimeAos() );
    printf( "number=%i;\n\n", sched.getNumberSession( session ) );

    session = sched.defaultSession();
    printf( "DEFAULT SESSION\n" );
    printf( "%s\t", (const char*)session. getNameSat().toUtf8() );
    printf( "%s\t", (const char*)session.getAos().toString( Qt::ISODate ).toUtf8() );
    printf( "%li\t", session.getRevol() );
    printf( "%i\n", (int)session.getState() );
    printf( "adopted=[%i];\n", session.adopted() );
    printf( "stage=%i;\n", (int)session.currentStage() );
    //    printf( "wait=%i;\n\n", session.diffTimeAos() );

    unsigned int count = sched.countSession();
    for( unsigned int i = 0; i < count; ++i )
        printf( "%s\n", (const char*)sched.getSession( i ).toLine().toUtf8() );
}

//! только принимаемые сеансы
void accepted() {
    Schedule sched1;
    sched1.create( info() );
    sched1.resolveConfl( info().getRuleConflResolv() );
    sched1.print();
    printf( "sched=%p\n", &sched1 );

    Schedule sched2;
    sched2 = sched1.acceptedSessions();
    Session& session = sched2.getSession( 0 );
    session.setState( conf::kUnkState );
    sched2.print();
    printf( "sched=%p\n", &sched2 );
}


int main(int argc, char** argv) 
{
  TAPPLICATION_NAME("meteo");

  QCoreApplication app(argc, argv);

    //info();
    testschedule();
    //session();
    //accepted();

    return 0;
}
