#include "context.h"
#include <cross-commons/app/paths.h>

#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")


namespace mappi {

namespace receiver {

Context::Context() :
  receiver(new Receiver)
{
}

Context::~Context()
{
  delete receiver;
  receiver = nullptr;
}

bool Context::init()
{
  info_log << QObject::tr("Конфигурационный файл: %1").arg(CONF_FILE);
  if (conf.load(CONF_FILE)) {
    debug_log << conf.toString();
    return receiver->tuned(conf);
  }

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
