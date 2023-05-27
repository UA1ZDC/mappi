#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <mappi/schedule/scheduler.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

using namespace mappi;
using namespace schedule;

#define SCHEDULE_PATH           MnCommon::varPath("mappi") + "schedule.txt"

int main(int argc, char** argv) 
{  
  TAPPLICATION_NAME("meteo");
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QCoreApplication app(argc, argv);

  Schedule schedule;
  Scheduler scheduler;
  
  //debug_log << scheduler.existTLE();

  if (!scheduler.create(schedule)) {
    return 1;
  }
  
  scheduler.resolveConfl( schedule );
 
  //schedule.print();
  
  if (!scheduler.save(schedule, SCHEDULE_PATH)) {
    return 2;
  }
  
  return 0;
}
