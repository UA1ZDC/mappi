#include "gridfile.h"

#include "gridfs.h"

#include <cross-commons/debug/tlog.h>

namespace meteo {

GridFile::GridFile()
{
}

GridFile::GridFile( const GridFile& gf )
{
  refcnt_ = gf.refcnt_;
  if ( nullptr != refcnt_ ) {
    *refcnt_ += 1;
  }
  gridfile_ = gf.gridfile_;
}

GridFile::GridFile( GridFilePrivate* gridfile )
  : refcnt_ ( new int32_t )
{
  *refcnt_ = 1;
  gridfile_ = gridfile;
}

GridFile::~GridFile()
{
  release();
}

void GridFile::release()
{
  if ( nullptr != refcnt_ ) {
    *refcnt_ -= 1;
    if ( 0 == *refcnt_ ) {
      delete refcnt_;
      refcnt_ = nullptr;
      delete gridfile_; gridfile_ = nullptr;
    }
  }
}

GridFile& GridFile::operator=(const GridFile& other)
{
  release();
  refcnt_ = other.refcnt_;
  gridfile_ = other.gridfile_;
  if ( nullptr != refcnt_ ) {
    *refcnt_ += 1;
  }
  return *this;
}

bool GridFile::isValid() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfile_->isValid();
}

bool GridFile::hasFile() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfile_->hasFile();
}

QString GridFile::id() const
{
  if ( nullptr == gridfile_ ) {
//    warning_log << kMsgConnectionNotInitialized;
    return QString();
  }
  return gridfile_->id();
}

int64_t GridFile::size() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return -1;
  }
  return gridfile_->size();
}

QString GridFile::md5() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return QString();
  }
  return gridfile_->md5();
}

bool GridFile::setMd5(const QString& md5)
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfile_->setMd5(md5);
}

QString GridFile::contentType() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return QString();
  }
  return gridfile_->contentType();
}

bool GridFile::setContentType( const QString& mimeType )
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfile_->setContentType(mimeType);
}

QDateTime GridFile::uploadDate() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return QDateTime();
  }
  return gridfile_->uploadDate();
}

QString GridFile::fileName() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return QString();
  }
  return gridfile_->fileName();
}

bool GridFile::setFileName(const QString& fileName)
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfile_->setFileName(fileName);
}

int64_t GridFile::write(const QByteArray& data, bool truncate )
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return -1;
  }
  return gridfile_->write( data, truncate );
}

QByteArray GridFile::readAll(bool* ok) const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    if ( nullptr != ok ) {
      *ok = false;
    }
    return QByteArray();
  }
  return gridfile_->readAll(ok);
}

QByteArray GridFile::read(int64_t maxSize, bool* ok) const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    if ( nullptr != ok ) {
      *ok = false;
    }
    return QByteArray();
  }
  return gridfile_->read( maxSize, ok );
}

uint64_t GridFile::position(bool* ok) const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    if ( nullptr != ok ) {
      *ok = false;
    }
    return 0;
  }
  return gridfile_->position(ok);
}

bool GridFile::seek(int64_t pos)
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfile_->seek(pos);
}

bool GridFile::remove()
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfile_->remove();
}

QString GridFile::lastError() const
{
  if ( nullptr == gridfile_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return kMsgConnectionNotInitialized;
  }
  return gridfile_->lastError();
}

GridFile::GridFilePrivate::GridFilePrivate()
{
}

GridFile::GridFilePrivate::~GridFilePrivate()
{
}

}
