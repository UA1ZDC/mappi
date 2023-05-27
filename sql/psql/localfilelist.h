#ifndef SQL_PSQL_LOCALFILELIST_H
#define SQL_PSQL_LOCALFILELIST_H

#include <sql/rapidjson/document.h>

#include <sql/dbi/gridfilelist.h>
#include <sql/dbi/gridfile.h>
#include <sql/dbi/dbi.h>

namespace meteo {

class LocalFileList : public GridFileList::GridFileListPrivate
{
  public:
    LocalFileList( const QString& prefix, Dbi* client );
    ~LocalFileList();

    bool isValid() const ;
    bool hasList() const ;

    bool next();

    GridFile file() const ;

    bool find( const rapidjson::Document& doc );
    bool findByName( const QString& filename );

  private:
    QString prefix_;
    Dbi* client_ = nullptr;
    QList< int64_t > files_;
    int currentpos_ = -1;

};

}

#endif
