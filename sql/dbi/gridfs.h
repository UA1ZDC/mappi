#ifndef SQL_DBI_GRIDFS_H
#define SQL_DBI_GRIDFS_H

#include <qstring.h>
#include <qobject.h>
#include <sql/psql/tpgsettings.h>
#include <sql/dbi/gridfile.h>
#include <sql/dbi/gridfilelist.h>

namespace meteo {

static const QString kMsgConnectionNotInitialized = QObject::tr("Подключение к БД не инициализировано!");

class GridFs
{
  public:
    enum ErrorCode {
      kNoError          = 0,
      kErrorRead        = 1,
      kErrorQuery       = 2,
      kErrorDuplicate   = 3,
      kErrorConnect     = 4,
      kErrorSave        = 5,
      kErrorRemove      = 6,
      kNotInitialized   = 7
    };
    GridFs(const GridFs& o) = delete;
    GridFs& operator=(const GridFs& o) = delete;

    GridFs();
    ~GridFs();

    bool connect( const meteo::ConnectProp& prop );
    bool isConnected() const;

    void use(const QString& dbName, const QString& prefixName);

    //! Загрузка в БД файла из ФС
    bool put(const QString& fileName, const QByteArray& data, GridFile* gridfile = nullptr, bool rewrite = true);

//    //! Поиск всех файлов соответствующих filter.
//    //! @arg filename - фильтр отбора документов в формате json.
//    //!               Пример: { "filename": "file.txt" }
//    //! Возвращает количество удалённых файлов.
    int remove(const QString& filename, bool* ok = nullptr);

    //! @arg filter - фильтр отбора документов в формате json.
    //!               Пример: { "_id": { "$oid": "5b926f7443f8ab054317a50f" } }
    GridFile findOne(const QString& filter);
    GridFile findOneByName( const QString& filename );
    GridFile findOneById( const QString& id );
    GridFile findOneByName( const std::string& filename );
    GridFile findOneById( const std::string& id );

    //! Поиск всех файлов соответствующих filter.
    //! @arg filter - фильтр отбора документов в формате json.
    //!               Пример: { "filename": "file.txt" }
    GridFileList find( const QString& filter) const ;
    GridFileList findByName( const QString& filename ) const ;

    QString lastError() const ; //FIXME сделать errorString
    ErrorCode error() const ;


  private:
    QString dbName_;
    QString prefixName_;
    class GridFsPrivate {
      public:
        GridFsPrivate();
        virtual ~GridFsPrivate();

        virtual bool connect( const meteo::ConnectProp& prop ) = 0;
        virtual bool isConnected() const = 0;

        virtual void use(const QString& dbName, const QString& prefixName) = 0;

        virtual bool put(const QString& fileName, const QByteArray& data, GridFile* gridfile = nullptr, bool rewrite = true) = 0;

        virtual int remove(const QString& filname, bool* ok = nullptr) = 0;

        virtual GridFile findOne(const QString& filter) = 0;
        virtual GridFile findOneByName(const QString& filename) = 0;
        virtual GridFile findOneById(const QString& id) = 0;

        virtual GridFileList find( const QString& filter ) const = 0;
        virtual GridFileList findByName( const QString& filename ) const = 0;

        virtual QString lastError() const = 0;
        virtual ErrorCode error() const = 0;
    };
    GridFsPrivate* gridfs_ = nullptr;

    friend class NosqlFs;
    friend class LocalFs;
};

}

#endif
