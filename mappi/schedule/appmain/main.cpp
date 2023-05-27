#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <mappi/schedule/schedule.h>
#include <stdlib.h>

#define SCHEDULE_FILE (MnCommon::varPath("mappi") + "schedule.txt")
#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")

using namespace mappi;
using namespace schedule;


int main(/*int argc, char* argv[]*/)
{
  TAPPLICATION_NAME("meteo");

  info_log << QObject::tr("Конфигурационный файл: %1").arg(CONF_FILE);
  Configuration conf;
  if (conf.load(CONF_FILE) == false) {
    error_log << QObject::tr("Не удалость загрузить конфигурационный файл");
    return EXIT_FAILURE;
  }

  debug_log << conf.toString();

  Schedule schedule;
  if (schedule.make(conf) == false) {
    error_log << QObject::tr("Ошибка формирования расписания");
    return EXIT_FAILURE;
  }

  schedule.resolvConfl(conf);

  debug_log << schedule.toString();
  if (schedule.save(SCHEDULE_FILE) == false) {
    error_log << QObject::tr("Не удалось сохранить расписание в файл: %1").arg(SCHEDULE_FILE);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
