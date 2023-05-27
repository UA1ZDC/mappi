#include "configloader.h"

#include <qfile.h>
#include <qfilesystemwatcher.h>

#include <google/protobuf/message.h>

#include <cross-commons/debug/tlog.h>
#include <commons/textproto/tprototext.h>

namespace meteo {

ConfigLoader::ConfigLoader(const QString& path, bool autoLoad, QObject* parent)
  : QObject(parent)
{
  watcher_ = 0;

  if ( autoLoad ) {
    watcher_ = new QFileSystemWatcher(this);
    watcher_->addPath(path);
    connect( watcher_, SIGNAL(directoryChanged(QString)), SLOT(slotLoad(QString)) );
  }
}

bool ConfigLoader::loadConfigPart(::google::protobuf::Message* part, const QString& filename) const
{
  QFile file(filename);
  if ( false == file.exists() ) {
    error_log << QObject::tr("Отсутствует конфигурационный файл '%1'.").arg(file.fileName());
    return false;
  }
  if ( false == file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при открытии файла конфигурации '%1'").arg(file.fileName());
    debug_log << file.errorString();
    return false;
  }
  QString text = QString::fromUtf8(file.readAll());

  if ( false == TProtoText::fillProto(text, part) ) {
    error_log << QObject::tr("Ошибка при обработке конфигурационного файла '%1'").arg(file.fileName());
    return false;
  }
  return true;
}

} // meteo
