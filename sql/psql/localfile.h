#ifndef SQL_PSQL_LOCALFILE_H
#define SQL_PSQL_LOCALFILE_H

#include <qstring.h>
#include <qdatetime.h>

#include <sql/rapidjson/document.h>

#include <sql/dbi/gridfile.h>

class QFile;

namespace meteo {

class Dbi;
class Document;

class LocalFile : public GridFile::GridFilePrivate
{
  public:
    LocalFile();
    //Конструктор для чтения файла
    LocalFile( const QString& filename, const QString& prefix, Dbi* client );
    LocalFile( int64_t id, const QString& prefix, Dbi* client );
    //В doc должен содержать filename или _id
    LocalFile( const rapidjson::Document& doc, const QString& prefix, Dbi* client );
    //Конструктор для сохранения
    LocalFile( const QString& filename, const QByteArray& data, const QString& prefix, Dbi* client );
    ~LocalFile();

    bool isValid() const ;
    bool hasFile() const ;

    QString id() const;
    int64_t size() const;

    QString md5() const;
    bool setMd5(const QString& md5);

    QString contentType() const;
    bool setContentType(const QString& mimeType);

    QDateTime uploadDate() const;
    QString fileName() const;

    bool setFileName(const QString& fileName);

    int64_t write(const QByteArray& data, bool truncate = false );

    QByteArray readAll(bool* ok) const;
    QByteArray read(int64_t maxSize, bool* ok) const;

    uint64_t position(bool* ok) const;
    bool seek(int64_t pos);

    bool remove();
    bool save( bool* dup_md5 );

    QString lastError() const { return lastError_; }

  private:
    bool put2fs();
    bool put2db( bool* dup_md5 );
    bool updateDbEntry();
    bool ensurePathExists();

    bool loadFileInfo( const QString& filename, const QString& fileid );
    bool removeFromDb();

    static QString fs_table_name( const QString& prefix );


  private:
    QString filename_;
    QByteArray data_;
    QString prefix_;
    Dbi* client_ = nullptr;
    int64_t id_ = -1;
    QString md5_;
    int size_ = 0;
    QString dbname_;
    mutable QFile* file_ = nullptr;
    mutable QString lastError_;
};

}

#endif
