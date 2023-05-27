#pragma once

#include "meteo/commons/proto/cleaner.pb.h"
#include <QDir>
#include <QString>


class QZipWriter;

namespace meteo {

namespace cleaner {

class Param;

class ArchUtil {
public :
  ArchUtil();
  ~ArchUtil();

  int64_t storageRotate(const Param& param);

private :
  bool baseDir(const Param& param);
  void archInit(const Param& param);
  QString archName();

  void archCreate();
  void archClose();

  int64_t rotate(const QString& root, const QDateTime& expired);
  int64_t rotate(const QDir& dir, const QDateTime& expired, const QString& root = "");

private :
  int prefix_;
  QZipWriter* arch_;
  QString archPath_;
};

}

}
