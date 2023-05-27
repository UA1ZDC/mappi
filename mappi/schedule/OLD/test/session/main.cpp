#include <stdio.h>
#include <mappi/schedule/session.h>

using namespace mappi;
using namespace schedule;

// преобразование в строку
void toLine() {
    Session session;
    QString line = session.toLine();
    printf( "%s;\n", line.toUtf8().data() );
}

// преобразование в сеанс
void toSession() {
    Session session;
    QString line = "NOAA 17\t2010-06-09T15:00:02\t2010-06-09T15:20:58\t89.09\t0\t0\t-1";
    //QString line = "NOAA 17\t2010-06-09T15:00:02\t2010-06-09T15:20:58\t89.09\t0\t0";
    printf( "toSession=%i;\n",  session.toSession( line ) );
    printf( "name=%s;\n", session.getNameSat().toUtf8().data() );
    printf( "aos=%s;\n", session.getAos().QDateTime::toString( Qt::ISODate ).toUtf8().data() );
    printf( "los=%s;\n", session.getLos().QDateTime::toString( Qt::ISODate ).toUtf8().data() );
    printf( "maxEl=%g;\n", session.getMaxEl() );
    printf( "revol=%li;\n", session.getRevol() );
    printf( "dir=%i;\n", session.getDirection() );
    printf( "state=%i;\n", (int)session.getState() );
    printf( "adopted=%i;\n", session.adopted() );
    printf( "stage=%i;\n", (int)session.currentStage() );
    printf( "sec=%i;\n", session.secsToAos() );
}

// копирование сеансов
void copySession() {
    Session session1;
    session1.setMaxEl( 54 );
    printf( "1 session=[%p] maxEl=[%g];\n", &session1, session1.getMaxEl() );

    Session session2;
    session2.setMaxEl( 45 );
    printf( "2 session=[%p] maxEl=[%g];\n", &session2, session2.getMaxEl() );

    session1 = session2;
    printf( "1 session=[%p] maxEl=[%g];\n", &session1, session1.getMaxEl() );
}

// конструктор копирования
void constrCopy() {
    Session session1;
    session1.setMaxEl( 54 );
    printf( "1 session=[%p] maxEl=[%g];\n", &session1, session1.getMaxEl() );

    //Session session2( session1 );
    Session session2 = session1;
    printf( "2 session=[%p] maxEl=[%g];\n", &session2, session2.getMaxEl() );
}

void ravenstvo() {
    Session session1;
    Session session2 = session1;
    if ( session1 == session2 )
        printf( "session1 == session2\n" );
    else
        printf( "session1 != session2\n" );

    session1.setRevol( 45 );
    if ( session1 == session2 )
        printf( "session1 == session2\n" );
    else
        printf( "session1 != session2\n" );

    session1 = session2;
    if ( session1 == session2 )
        printf( "session1 == session2\n" );
    else
        printf( "session1 != session2\n" );
};

int main() 
{
  //toLine();
  //toSession();
  //copySession();
  //constrCopy();
  ravenstvo();

    return 0;
};
