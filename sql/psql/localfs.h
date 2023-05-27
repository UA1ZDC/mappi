#ifndef SQL_PSQL_LOCALFS_H
#define SQL_PSQL_LOCALFS_H

#include <sql/psql/tpgsettings.h>
#include <qstring.h>
#include <qbytearraylist.h>

#include <sql/dbi/gridfs.h>
#include <sql/dbi/gridfile.h>
#include <sql/dbi/gridfilelist.h>

namespace meteo {

class Dbi;

class LocalFs : public GridFs::GridFsPrivate
{
  public:
    LocalFs(const LocalFs& o) = delete;
    LocalFs& operator=(const LocalFs& o) = delete;

    LocalFs();
    ~LocalFs();

    bool connect(const ConnectProp& prop);

    bool isConnected() const;

    void use( const QString& dbName, const QString& prefix );
    bool put( const QString& fileName, const QByteArray& data, GridFile* gridfile = nullptr, bool rewrite = true );

    int remove(const QString& filename, bool* ok = nullptr);

    GridFile findOne( const QString& filter );
    GridFile findOneByName( const QString& filename );
    GridFile findOneById( const QString& id );

    GridFileList find( const QString& filter ) const;
    GridFileList findByName( const QString& filename ) const ;

    QString lastError() const { return lastError_; }
    GridFs::ErrorCode error() const { return error_; }

    static QString rootLocalfsPath();
    static QString dirNameDb( const QString& db, const QString& subDir);
    static QString dirNameFs( const QString& db, const QString& prefix );
    static QString fileNameFs( const QString& db, const QString& prefix, const QString& filename, int64_t id );

  private:
    bool removeFromDb( const QString& filename );

  private:
    QString db_;
    QString prefix_;
    ConnectProp params_;
    mutable Dbi* client_ = nullptr;

    mutable QString lastError_;
    mutable GridFs::ErrorCode error_ = GridFs::kNoError;
};

}

#endif
