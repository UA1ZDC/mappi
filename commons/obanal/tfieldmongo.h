#ifndef NOSQL_OBANAL_TFIELD_H
#define NOSQL_OBANAL_TFIELD_H

#include "tfield.h"
#include <sql/dbi/gridfile.h>
#include <sql/dbi/gridfs.h>


namespace obanal{

class TField;

//! будем считать, что поле представляет собой сетку покрывающую всю поверхность планеты
//! значение в точке сопровождается маской наличия данных
class TFieldMongo:
     public TField
{
  public:
    TFieldMongo();
    TFieldMongo(const TField& f);

    virtual ~TFieldMongo();
    
    virtual float pointValueF(const meteo::GeoPoint& point,  bool* ok = nullptr) const;
    virtual bool radKriv(int i, int j, float *rad)const;
    bool fromGridFile(const meteo::GridFile& file);


  private:
    //! Загружает данные из файла в память, если они не были загружены. Вызов данной функции должен предшествовать
    //! обращению к полям data_, datamask_ и т.д.
    bool loadToMemory();
    virtual bool isValidSheme(int i, int j)const;

    virtual bool ensureDataLoaded(int idx) const;
    virtual bool ensureDataLoaded(int i0, int i1, int i2, int i3) const;

    virtual bool ensureNetMaskLoaded(int idx) const;
    virtual bool ensureNetMaskLoaded(int i0, int i1, int i2, int i3) const;

    virtual bool ensureDataMaskLoaded(int idx) const;
    virtual bool ensureDataMaskLoaded(int i0, int i1, int i2, int i3) const;

    virtual bool ensurePageLoaded(int n) const;

  private:
    // при работе с GridFS файл делится на страницы размером kPageSize
    // подгружаются только требуемые страницы
    static const int kPageSize = 1*1024;

    bool  useGridFile_  = false;


    mutable meteo::GridFile gridFile_;
    mutable QBitArray pageLoaded_;
    int gridPages_ = 0;

    //meteo::GridFs *gridfs = nullptr;
    //meteo::GridFile *gridfile_ = nullptr;
    friend class TField;
  };
}
#endif
