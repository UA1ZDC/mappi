#ifndef TUROVEN_H
#define TUROVEN_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/geobasis/geovector.h>

#include <qlist.h>
#include <qvector.h>

const float MAX_H = 50000.;
const float MAX_PRESS = 1084;

namespace zond {

 // const int COUNT_OF_GROUP = 17;
  /*
  enum GroupType {
    Gr_Station       = 1, //!< на уровне станции"
    Gr_Tropo          = 2, //!< на уровне тропопаузы"
    Gr_MaxWind    = 3, //!< на уровне максимального ветра"
    Gr_Tempr         = 4, //!< особые точки  по температуре
    Gr_Wind           = 5, //!< особые точки  по ветру
    Gr_Standart     = 6, //!< на стандартных изобарических поверхностях"
    Gr_Lidar     = 7, //!<
    Gr_MaxWind_D    = 13, //!< на уровне максимального ветра выше 100
    Gr_Tempr_D         = 14, //!< особые точки  по температуре выше 100
    Gr_Wind_D           = 15, //!< особые точки  по ветру выше 100
    Gr_Standart_C     = 16, //!< на стандартных изобарических поверхностях выше 100
 //   Gr_PrizInvHi    = 17, //!< на стандартных изобарических поверхностях выше 100
 //   Gr_PrizInvLow    = 18, //!< на стандартных изобарических поверхностях выше 100
    LastGroup        = 17,//!< для цикла по группам в упорядочении по абсолютным значениям

    Gr_5001000      = 20 //!< относительные значения P,T,V для карт относительной топографии ОТ 500/1000
  };*/

  enum ValueType {
    UR_P  =   0,  //!< давление
    UR_H  =   1,  //!< высота
    UR_T  =   2,  //!< температура
    UR_Td  =  3,  //!< темп. точки росы
    UR_D  =   4,  //!< дефицит точки росы
    UR_dd =   5,  //!< направление ветра
    UR_ff =   6,  //!< скорость ветра
    UR_u =    7,  //!<
    UR_v =    8,  //!<
    LAST_UR =  9,
    UR_CLOUD_type = 10,
    UR_CLOUD_Nh = 11,
    UR_CLOUD_h = 12
  };

 // int levelTypePoGroupType(GroupType type);
  int descrPoUrType(ValueType type);
  ValueType UrTypePoDescr( int descr);

  struct Uroven {
  public:
    Uroven();

    void reset();
    float value(ValueType type) const { return pair(type).first; }
    int quality(ValueType type) const { return pair(type).second; }
    int levelType() const {return level_type;}
    int countValid()const; //!<количество валидных значений
    QPair<float, int> pair(ValueType type) const;
    void setLevelType( meteodescr::LevelType );
    void setLevelTypeInt(int);

    void set(ValueType type, float val, int qual );
    void print()const;
    QString toString() const;
    bool isGood(ValueType type) const;

    void to_ddff();
    void to_uv();
    void to_D();
    void to_Td();

    int size() const;

    friend QDataStream &operator<<(QDataStream &out, const Uroven &mtp);
    friend QDataStream &operator>>(QDataStream &in, Uroven &mtp);

    Uroven& operator=( const Uroven& src );



    TMeteoData toMeteoData() const;

  private :
    //значение, показатель качества
    QPair<float, int> P =  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);  //!< давление
    QPair<float, int> H=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);  //!< высота геопотенциала
    QPair<float, int> T=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);  //!< температура
    QPair<float, int> D=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);  //!< дефицит
    QPair<float, int> Td=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE); //!< точка росы
    QPair<float, int> dd=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE); //!< направление ветра
    QPair<float, int> ff=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE); //!< скорость ветра
    QPair<float, int> u=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);  //!< направление ветра
    QPair<float, int> v=  QPair<float, int>(BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);  //!< скорость ветра
   // GroupType group_type; //группа данных уровня
    meteodescr::LevelType level_type; //тип уровня

    friend TLog& operator<<( TLog& log, const Uroven& gc );
  };

  struct InvProp {
    double h_lo;
    double h_hi;
    double T_lo;
    double T_hi;
    double w_min;
    int sign;
    enum InvType { IZOTERMIA = 0, INVERSIA = 1 } invType;
    double dT; //!< рассчитанный градиент

    InvProp() {
      h_lo = 0.;
      h_hi = 0.;
      T_lo = 0.;
      T_hi = 0.;
      w_min = 0.;
      sign = 0;
      invType = INVERSIA;
      dT = 0.;
    }
  };

  struct WindAverage {
    double h_lo;
    double h_hi;
    double u;
    double v;
  };


}



#endif
