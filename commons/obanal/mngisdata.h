#ifndef MNGISDATA_H
#define MNGISDATA_H

#include <stdio.h>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include <cross-commons/app/paths.h>
#include <commons/geobasis/geovector.h>

namespace TGis {

enum DataType {
  UNKNOWN_DATA = -1,
  TOPO_DATA = 0,        //!< Тип для географической основы (geo-files)
  GSHHS_DATA = 1,       //!< Тип для географической основы (gshhs-files)
  TMETEO_DATA = 2,       //!< Тип для метеовеличин
  TFIELD_DATA = 3,       //!< Тип для полей метеовеличин
  TCITY_DATA = 4,       //!< Тип для городов
  BASEDATACOUNT         //!< Типы данных, определённые в других местах, дб > BASEDATACOUNT
};

/*!
 * Базовый абстрактный тип данных. Пока считается, что все типы метеорологических данных
 * наследуются от Data
 */
class Data {
  public:
    Data();
    virtual ~Data();
    virtual int type() const = 0;
};


}

#endif

