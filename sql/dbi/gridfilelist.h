#ifndef SQL_DBI_GRIDFILELIST_H
#define SQL_DBI_GRIDFILELIST_H

#include <sql/dbi/gridfile.h>

namespace meteo {

class GridFileList
{
  public:
    GridFileList();
    GridFileList(const GridFileList& other);
    ~GridFileList();

    GridFileList& operator=(const GridFileList& other);

    bool isValid() const ;
    bool hasList() const ;

    bool next();

    GridFile file() const ;

  private:
    class GridFileListPrivate {
      public:
        GridFileListPrivate();
        virtual ~GridFileListPrivate();

        virtual bool isValid() const = 0;
        virtual bool hasList() const = 0;

        virtual bool next() = 0;

        virtual GridFile file() const = 0;
    };
    GridFileListPrivate* filelist_ = nullptr;
    int32_t* refcnt_ = nullptr;

  private:
    GridFileList( GridFileListPrivate* filelist );
    void release();

  friend class NosqlFileList;
  friend class NosqlFs;
  friend class LocalFileList;
  friend class LocalFs;
};

}

#endif
