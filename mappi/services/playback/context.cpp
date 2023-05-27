#include "context.h"
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>

#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")


namespace mappi {

namespace playback {

Context::Context() :
  demoMode(false)
{
  timeOffset = 0;
}

Context::~Context()
{
}

bool Context::confLoad()
{
  info_log << QObject::tr("Конфигурационный файл: %1").arg(CONF_FILE);

  QFile file(CONF_FILE);
  if (file.open(QIODevice::ReadOnly) == false) {
    error_log << QObject::tr("Ошибка открытия");
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  conf::Reception conf;
  if (TProtoText::fillProto(text, &conf) == false) {
    error_log << QObject::tr("Ошибка формата данных");
    return false;
  }

  demoMode = conf.demo_mode();

  return true;
}

QDateTime Context::now() const
{
  return QDateTime::currentDateTimeUtc().addSecs(timeOffset);
}

QDateTime Context::actualAos() const
{
  return QDateTime::currentDateTimeUtc().addMSecs(session.msecsToAos(now()));
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
