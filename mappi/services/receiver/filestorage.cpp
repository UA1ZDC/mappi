#include "filestorage.h"
#include <cross-commons/debug/tlog.h>
#include <qfile.h>
#include <qdir.h>


namespace mappi {

namespace receiver {

FileStorage::FileStorage(Context* ctx) :
  ctx_(ctx)
{
}

FileStorage::~FileStorage()
{
  ctx_ = nullptr;
}

bool FileStorage::create() const
{
  QDir dir;
  if (dir.mkpath(ctx_->conf.sessionDir) == false) {
    error_log << QObject::tr("Ошибка создания директории: %1").arg(ctx_->conf.sessionDir);
    return false;
  }

  QString dst = sessionFile();
  QFile file(dst);
  if (file.open(QIODevice::WriteOnly) == false) {
    error_log << QObject::tr("Ошибка создания raw-файла: %1").arg(dst);
    return false;
  }
  file.close();

  info_log << QObject::tr("raw-файл: %1").arg(dst);

  QString linkName = ctx_->conf.rawFile;
  // удаление старой ссылки
  QFile link(linkName);
  link.remove();

  if (QFile::link(dst, linkName) == false) {
    error_log << QObject::tr("Ошибка создания ссылки на raw-файл: %1").arg(linkName);
    return false;
  }

  info_log << QObject::tr("raw-файл (ссылка): %1").arg(linkName);

  return true;
}

bool FileStorage::setHeader(const meteo::global::StreamHeader& header) const
{
  QString dst = sessionFile();

  QFile file(dst);
  if (file.open(QIODevice::WriteOnly) == false) {
    error_log << QObject::tr("Не удалось открыть файл: %1").arg(dst);
    return false;
  }

  using namespace meteo::global;

  QByteArray buf;
  toBuffer(header, buf);
  PreHeader phead(kRawFile, buf.size());

  QDataStream out(&file);
  out << phead;
  out.writeRawData(buf.data(), buf.size());

  file.close();

  return true;
}

QString FileStorage::sessionFile() const
{
  return QString("%1/%2")
    .arg(ctx_->conf.sessionDir)
    .arg(ctx_->sessionOriginal.data().fileName());
}

}

}
