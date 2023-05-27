#pragma once

#include <sql/psql/localfs.h>


namespace meteo {

namespace cleaner {

class Param {
public :
  Param(const FileDbRemove& opt) :
    opt_(opt),
    currentDt_(QDateTime::currentDateTimeUtc()),
    dbName_(QString::fromStdString(opt.conn_name())),

    msgTable_(QString::fromStdString(opt.collection())),
    msgFsField_(QString::fromStdString(opt.gridfs_field())),

    dtField_(QString::fromStdString(opt.dt_field())),
    dt_(currentDt_.addDays(-opt.delete_period())),

    dtWriteField_(QString::fromStdString(opt.dt_write_field())),
    dtWrite_(currentDt_.addDays(-opt.store_period())),

    fsTable_(QString::fromStdString(opt.gridfs_prefix())),
    fsField_(QString::fromStdString(opt.gridfs_path())),

    dtCompress_(currentDt_.addDays(-opt.archive_period())) {
  }

  QDateTime currentDate() const { return currentDt_; }
  QString dbName() const { return dbName_; }

  QString msgTable() const { return msgTable_; }
  QString msgFsField() const { return msgFsField_; }

  QString dtField() const { return dtField_; }
  QDateTime dt() const { return dt_; }

  QString dtWriteField() const { return dtWriteField_; }
  QDateTime dtWrite() const { return dtWrite_; }

  bool isFsTable() const { return opt_.has_gridfs_prefix(); }
  QString fsTable() const { return fsTable_; }
  QString fsField() const { return fsField_; }
  QString fsPath() const { return LocalFs::dirNameFs(dbName_, fsTable_); }

  bool isCompress() const { return opt_.has_archive_period(); }
  QDateTime dtCompress() const { return dtCompress_; }
  QString compressPath() const { return LocalFs::dirNameDb(dbName_, "archives"); }

private :
  FileDbRemove opt_;

  QDateTime currentDt_;
  QString dbName_;

  QString msgTable_;
  QString msgFsField_;

  QString dtField_;
  QDateTime dt_;

  QString dtWriteField_;
  QDateTime dtWrite_;

  QString fsTable_;
  QString fsField_;

  QDateTime dtCompress_;
};

}

}
