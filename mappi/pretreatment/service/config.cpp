#include "config.h"
#include <commons/textproto/tprototext.h>


namespace mappi {

namespace pretreatment {

Configuration::Configuration()
{
}

Configuration::~Configuration()
{
}

bool Configuration::load(const QString& filePath)
{
  isValid_ = false;

  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly) == false)
    return false;

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  conf::Reception conf;
  if (TProtoText::fillProto(text, &conf) == false)
    return false;

  dataDir = conf.file_storage().root().c_str();
  sessionDir = conf.file_storage().session().c_str();

  isValid_ = true;

  return true;
}

QString Configuration::toString() const
{
  QStringList res = {
    "",
    QString("is valid:    %1").arg(isValid_ ? "true" : "false"),
    QString("data dir:    %1").arg(dataDir),
    QString("session dir: %1").arg(sessionDir),
  };

  return res.join("\n");
}

}

}
