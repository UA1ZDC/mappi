#ifndef SQL_DBI_GRIDFILE_H
#define SQL_DBI_GRIDFILE_H

#include <qstring.h>
#include <qdatetime.h>

namespace meteo {

class GridFile
{
  public:
    GridFile();
    GridFile(const GridFile& gridfile);
    ~GridFile();

    GridFile& operator=(const GridFile& other);

    bool isValid() const ;
    bool hasFile() const ;

    QString id() const ;
    int64_t size() const ;

    QString md5() const ;
    bool setMd5(const QString& md5);

    QString contentType() const ;
    bool setContentType( const QString& mimeType );

    QDateTime uploadDate() const ;
    QString fileName() const ;

    bool setFileName(const QString& fileName);

    //! Дописывает блок данных в конец файла.
    //! Возвращает количество записанных байт.
    int64_t write(const QByteArray& data, bool truncate = false);

    QByteArray readAll(bool* ok) const ;
    QByteArray read(int64_t maxSize, bool* ok) const ;

    uint64_t position(bool* ok) const ;
    bool seek(int64_t pos);

    bool remove();

    QString lastError() const ;

  private:
    class GridFilePrivate {
      public:
        GridFilePrivate();
        virtual ~GridFilePrivate();

        virtual bool isValid() const = 0;
        virtual bool hasFile() const = 0;

        virtual QString id() const = 0;
        virtual int64_t size() const = 0;

        virtual QString md5() const = 0;
        virtual bool setMd5(const QString& md5) = 0;

        virtual QString contentType() const = 0;
        virtual bool setContentType( const QString& mimeType ) = 0;

        virtual QDateTime uploadDate() const = 0;
        virtual QString fileName() const = 0;
      //  Document metadata() const ; TODO починить!!!

        virtual bool setFileName(const QString& fileName) = 0;

        //! Дописывает блок данных в конец файла.
        //! Возвращает количество записанных байт.
        virtual int64_t write(const QByteArray& data, bool truncate = false) = 0;

        virtual QByteArray readAll(bool* ok) const = 0;
        virtual QByteArray read(int64_t maxSize, bool* ok) const = 0;

        virtual uint64_t position(bool* ok) const = 0;
        virtual bool seek(int64_t pos) = 0;

        virtual bool remove() = 0;

        virtual QString lastError() const = 0;
    };
    GridFilePrivate* gridfile_ = nullptr;
    int32_t* refcnt_ = nullptr;

  private:
    GridFile( GridFilePrivate* gridfile );
    void release();

  friend class NosqlFs;
  friend class NosqlFile;
  friend class NosqlFileList;
  friend class LocalFs;
  friend class LocalFile;
  friend class LocalFileList;
};

}

#endif
