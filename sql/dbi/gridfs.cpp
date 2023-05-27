#include "gridfs.h"

#include <cross-commons/debug/tlog.h>
#include <sql/psql/localfs.h>

namespace meteo {

GridFs::GridFs()
{
}

GridFs::~GridFs()
{
  delete gridfs_; gridfs_ = nullptr;
}

bool GridFs::connect( const ConnectProp& params )
{
  if ( nullptr != gridfs_ ) {
    if ( true == gridfs_->isConnected() ) {
      warning_log << QObject::tr("Соединение уже установлено.");
      return true;
    }
    delete gridfs_;
    gridfs_ = nullptr;
  }
  switch ( params.driver() ) {
    case settings::kMongo:
      break;
    case settings::kPsql:
    default:
      gridfs_ = new LocalFs();
      break;
  }
  if ( nullptr == gridfs_ ) {
    return false;
  }
  return gridfs_->connect(params);
}

bool GridFs::isConnected() const
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfs_->isConnected();
}

void GridFs::use( const QString& dbName, const QString& prefixName )
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return;
  }
  gridfs_->use( dbName, prefixName );
}

bool GridFs::put(const QString& fileName, const QByteArray& data, GridFile* gridfile, bool rewrite )
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return false;
  }
  return gridfs_->put( fileName, data, gridfile, rewrite );
}

int GridFs::remove(const QString& filename, bool* ok )
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return -1;
  }
  return gridfs_->remove( filename, ok );
}

GridFile GridFs::findOne( const QString& filter )
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return GridFile();
  }
  return gridfs_->findOne(filter);
}

GridFile GridFs::findOneByName( const QString& filename )
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return GridFile();
  }
  return gridfs_->findOneByName(filename);
}

GridFile GridFs::findOneById( const QString& id )
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return GridFile();
  }
  return gridfs_->findOneById(id);
}

GridFileList GridFs::find( const QString& filter ) const
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return GridFileList();
  }
  return gridfs_->find( filter );
}

GridFileList GridFs::findByName(const QString& filename) const
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return GridFileList();
  }
  return gridfs_->findByName( filename );
}

QString GridFs::lastError() const
{
  if ( nullptr == gridfs_ ) {
    warning_log << kMsgConnectionNotInitialized;
    return kMsgConnectionNotInitialized;
  }
  return gridfs_->lastError();
}

GridFs::ErrorCode GridFs::error() const
{
  if ( nullptr == gridfs_ ) {
    return kNotInitialized;
  }
  return gridfs_->error();
}

GridFile GridFs::findOneByName( const std::string& filename )
{
  return gridfs_->findOneByName(QString::fromStdString(filename));
}

GridFile GridFs::findOneById( const std::string& id )
{
  return gridfs_->findOneById(QString::fromStdString(id));
}

GridFs::GridFsPrivate::GridFsPrivate()
{
}

GridFs::GridFsPrivate::~GridFsPrivate()
{
}

}
