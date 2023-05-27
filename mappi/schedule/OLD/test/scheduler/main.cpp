#include <mappi/schedule/scheduler.h>
#include <cross-commons/app/paths.h>


using namespace mappi;
using namespace schedule;


int main() 
{
  TAPPLICATION_NAME("meteo");

    Schedule schedule;
    Scheduler scheduler;
    printf( "tle = %i;\n", scheduler.existTLE() );

    printf( "create schedule = %i;\n", scheduler.create( schedule ) );
    scheduler.resolveConfl( schedule );
    schedule.print();

    //bool save( Schedule& sched, const QString& path );
    //bool restore( Schedule& sched, const QString& path );

    return 0;
}
