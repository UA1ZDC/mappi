#ifndef METEO_COMMONS_OCEAN_TUROVEN_H
#define METEO_COMMONS_OCEAN_TUROVEN_H

#include <qpair.h>

#include <cross-commons/debug/tlog.h>

namespace ocean {
const int COUNT_OF_GROUP = 4;

enum GroupType {
  Gr_Surface       = 1, //!< на уровне поверхности океана
  Gr_Mass          = 2, //!< на уровне в толще океана
  Gr_Bottom        = 3, //!< на уровне дна океана
  LastGroup        = 4  //!< для цикла по группам в упорядочении по абсолютным значениям
};

enum ValueType {
  UR_H  =  0,
  UR_T  =  1,
  UR_S  =  2,
  LAST_UR =  3
};

int descrPoUrType(ValueType type);

struct Uroven {
  public:
    Uroven();

    void reset();
    float value(ValueType type) const { return pair(type).first; }
    int quality(ValueType type) const { return pair(type).second; } 
    GroupType groupType() const {return group_type;}
    int levelType() const {return level_type;}
    int countValid()const; //!<количество валидных значений 
    QPair<float, int> pair(ValueType type) const;
    void setGroupType( GroupType );
    void setLevelType( int );

    void set(ValueType type, float val, int qual );
    void print()const;
    bool isGood(ValueType type) const;

  private :    
    //значение, показатель качества
    QPair<float, int> H;  //!< глубина
    QPair<float, int> T;  //!< температура
    QPair<float, int> S;  //!< солёность
    GroupType group_type; //группа данных уровня
    int level_type; //тип уровня

  friend QString& operator<<( QString& str, const Uroven& gc );
  friend TLog& operator<<( TLog& log, const Uroven& gc );
};
}


#endif
