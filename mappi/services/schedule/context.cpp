#include "context.h"
#include <cross-commons/app/paths.h>

#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")
#define SCHEDULE_FILE (MnCommon::varPath("mappi") + "schedule.txt")


namespace mappi {

namespace schedule {

bool Context::confLoad()
{
  info_log << QObject::tr("Конфигурационный файл: %1").arg(CONF_FILE);
  if (conf.load(CONF_FILE)) {
    debug_log << conf.toString();
    return true;
  }

  error_log << QObject::tr("Ошибка загрузки");

  return false;
}

bool Context::makeSchedule()
{
  // если сервис не смог загрузить конфигурацию, он не запустится
  // if (conf.isValid() == false) {
  //   error_ = QObject::tr("Не удалость загрузить конфигурационный файл");
  //   return false;
  // }

  if (schedule.make(conf) == false) {
    error_ = QObject::tr("Ошибка формирования расписания");
    return false;
  }

  schedule.addGeo(conf);
  schedule.resolvConfl(conf, (0 < schedule.size()));
  saveSchedule();

  info_log << QObject::tr("Cформировано новое расписание");
  debug_log << QString("\n%1").arg(schedule.toString(true));

  return true;
}

void Context::saveSchedule()
{
  // WARNING не считается критической ошибкой
  if (schedule.save(SCHEDULE_FILE) == false)
    warning_log << QObject::tr("Ошибка сохранения расписания в файл: %1").arg(SCHEDULE_FILE);
}

QString Context::lastError()
{
  QString tmp(error_);
  error_.clear();

  return tmp;
}

void Context::setLastError(const QString& msg)
{
  error_ = msg;
}

bool Context::hasError() const
{
  return (!error_.isEmpty());
}

}

}
