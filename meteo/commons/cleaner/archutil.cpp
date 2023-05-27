#include "archutil.h"
#include "param.h"
#include "cross-commons/debug/tlog.h"
#include <QtGui/private/qzipwriter_p.h>
#include <QDirIterator>
#include <QtDebug>


namespace meteo {

namespace cleaner {

static const int64_t ARCHIVE_FILE_SIZE_MAX = 2147483648;  // 2GB
static const QString dtFormat = "dd.MM.yyyy";

ArchUtil::ArchUtil() :
  prefix_(0),
  arch_(nullptr) {
}

ArchUtil::~ArchUtil()
{
  archClose();
}

bool ArchUtil::baseDir(const Param& param)
{
  QDir dir(param.compressPath());
  bool res = dir.mkpath(dir.path());
  error_log_if(!res) << QObject::tr("Не удалось создать директорию '%1'").arg(dir.path());

  return res;
}

void ArchUtil::archInit(const Param& param)
{
  archPath_ = QString("%1%2-%3")
    .arg(param.compressPath())
    .arg(param.dt().toString(dtFormat))
    .arg(param.currentDate().toString(dtFormat));
}

QString ArchUtil::archName()
{
  prefix_ = 0;
  while (true) {
    QString path = QString("%1%2.zip")
      .arg(archPath_)
      .arg(0 < prefix_ ? QString("_%1").arg(prefix_) : "");

    QFile file(path);
    if (!file.exists())
      return path;

    ++prefix_;
  }
}

void ArchUtil::archCreate()
{
  if (arch_ == nullptr) {
    arch_ = new QZipWriter(archName());

    ++prefix_;
    // QZipWriter::NeverCompress
    // QZipWriter::AlwaysCompress
    // QZipWriter::AutoCompress
    arch_->setCompressionPolicy(QZipWriter::NeverCompress);
    // arch_->setCreationPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup);
  }
}

void ArchUtil::archClose()
{
  if (arch_ != nullptr) {
    arch_->close();
    delete arch_;
    arch_ = nullptr;
  }
}

int64_t ArchUtil::storageRotate(const Param& param)
{
  if (!baseDir(param))
    return -1;

  archInit(param);
  return rotate(param.fsPath(), param.dt());
}

int64_t ArchUtil::rotate(const QString& root, const QDateTime& expired)
{
  int64_t res = 0;

  // отладка выводит первых 100 удаляемых файлов
  // debug_log << "root = " << root << " expired = " << expired;
  // int i = 100;

  QDirIterator it(root, (QDir::AllEntries | QDir::NoDotAndDotDot), QDirIterator::Subdirectories);
  while (it.hasNext()) {
    archCreate();

    QString path = it.next();
    QFileInfo info(path);
    // if (0 < i) {
    //   error_log << path;
    //   --i;
    // }

    QString arch_path = path.remove(root);

    if (info.isFile() && (info.birthTime() < expired)) {
      QFile file(info.filePath());
      if (file.open(QIODevice::ReadOnly)) {
        arch_->setCreationPermissions(info.permissions());
        arch_->addFile(arch_path, file.readAll());
      }

      QFile::remove(info.filePath());
      ++res;
    } else if (info.isDir()) {
      QDir subDir(info.filePath());
      if (!subDir.isEmpty()) {
        arch_->setCreationPermissions(info.permissions());
        arch_->addDirectory(arch_path);
      } else
        subDir.rmdir(info.filePath());
    }

    if (ARCHIVE_FILE_SIZE_MAX <= arch_->device()->size())
      archClose();
  }

  archClose();

  return res;
}

int64_t ArchUtil::rotate(const QDir& dir, const QDateTime& expired, const QString& root /*=*/)
{
  int64_t res = 0;
  QStringListIterator it(dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
  while (it.hasNext()) {
    archCreate();

    QFileInfo info(dir.absoluteFilePath(it.next()));
    QString path = info.filePath();
    QString arch_path = (root.isEmpty() ? info.fileName() : QString("%1/%2").arg(root).arg(info.fileName()));

    if (info.isFile() && (info.birthTime() < expired)) {
      QFile file(path);
      if (file.open(QIODevice::ReadOnly)) {
        arch_->setCreationPermissions(info.permissions());
        arch_->addFile(arch_path, file.readAll());
      }

      QFile::remove(path);
      ++res;
    } else if (info.isDir()) {
      QDir subDir(path);
      if (!subDir.isEmpty()) {
        arch_->setCreationPermissions(info.permissions());
        arch_->addDirectory(arch_path);

        res += rotate(subDir, expired, arch_path);
      } else
        subDir.rmdir(path);
    }

    if (ARCHIVE_FILE_SIZE_MAX <= arch_->device()->size())
      archClose();
  }

  return res;
}

}

}
