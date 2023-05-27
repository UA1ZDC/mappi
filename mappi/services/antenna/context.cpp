#include "context.h"
#include <cross-commons/app/paths.h>

#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")


namespace mappi {

namespace antenna {

Context::Context(QObject* parent /*=*/) :
    QObject(parent),
  antenna(new Antenna),
  direct(new Direct(antenna)),
  tracker(new Tracker(antenna)),
  mTest(new MonkeyTest(antenna))
{
  // QObject::connect(direct, &Direct::terminate, this, [](){ info_log << QObject::tr("Сценарий наведения завершен"); });
  // QObject::connect(tracker, &Tracker::terminate, this, [](){ info_log << QObject::tr("Сценарий сопровождения завершен"); });
  // QObject::connect(mTest, &Tracker::terminate, this, [](){ info_log << QObject::tr("Сценарий прогона завершен"); });
}

Context::~Context()
{
  delete antenna;
  antenna = nullptr;
}

bool Context::init()
{
  info_log << QObject::tr("Конфигурационный файл: %1").arg(CONF_FILE);
  if (conf.load(CONF_FILE)) {
    debug_log << conf.toString();
    return (antenna->init(conf) && antenna->open());
  }

  // WARNING Starter не нужен
  error_log << QObject::tr("Ошибка загрузки");

  return false;
}

QDateTime Context::now() const
{
  return QDateTime::currentDateTimeUtc();
}

bool Context::isReplay() const
{
  return (sessionOriginal_prep.data() == sessionOriginal.data());
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
