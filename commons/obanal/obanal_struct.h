#ifndef OBANAL_STRUCT_H
#define OBANAL_STRUCT_H

#include "mngisdata.h"

#include <commons/geobasis/geopoint.h>
#include <commons/mathtools/mnmath.h>

#include <QDateTime>
#include <QHash>

class QTextStream;

enum DataSource{
  REGRESS_SRC,
  DATABASE_SRC,
  MODEL_SRC,
  FILE_SRC
};

//TODO 
enum ModelType{
  HWM=0,
  MSISE=1,
  MSISE1=2,
  MOSCOW=15
};

//TODO а они нужны заданные или это настройка?
enum obanalParam {
  OB_UNKNOWN_PARAM = -1,
  H_PARAM  = 0,   //!< геопотенциал
  T_PARAM  = 1,   //!< температура
  D_PARAM  = 2,   //!< дефицит точки росы
  FF_PARAM = 3,   //!< скорость ветра
  P_PARAM  = 4,   //!< атм. давление
  TEND_PARAM = 5, //!< тенденция изменения давления
  TSEA_PARAM = 6, //!< температура пов. моря
  VOLNSEA_PARAM =  7, //!< волнение моря
  TROPO_PARAM   =  8, //!< высота тропопаузы
  MAXWIND_PARAM =  9, //!< максимальный  ветер
  UWIND_PARAM   = 33, //!< зональная составляющая ветра
  VWIND_PARAM   = 34, //!< меридиональная составляющая ветра
  TOPO_PARAM    = 12, //!< высота над уровнем моря
  DD_PARAM      = 13, //!< направление ветра
  TW_PARAM    = 10,   //!< количество осадков
  KOL_PARAM
};

enum NetType {
  STEP_500x500 = 1,  //!< через 5 градусов по широте и долготе
  STEP_250x250 = 2,  //!< через 2.5 градуса по широте и долготе
  STEP_125x125 = 3,  //!< через 1.25 градуса по широте и долготе
  STEP_100x100 = 4,  //!< через 1 градус по широте и долготе
  STEP_0625x0625 = 5,//!< через 0.625 градуса по широте и долготе
  STEP_2983x100 = 6, //!< через 2.983 градуса по широте и долготе
  STEP_0500x0500 = 7, //!< через .5 градуса по широте и долготе
  STEP_0100x0100 = 8, //!< через .1 градуса по широте и долготе
  NO_TYPE = 9,
  GENERAL_NET = 10
};

namespace obanal {
  NetType netTypeFromStep(int di, bool* ok = nullptr);
  NetType nearNetTypeFromStep(int di);
  QString netTypetoStr(NetType type);
}
enum laplaValue{
  PLUS_LAPLA,
  MINUS_LAPLA,
  NONE_LAPLA
};

struct fieldExtremum{
    fieldExtremum(){
      tip = 2;
      znach = -9999;
    }
    fieldExtremum(int aex,meteo::GeoPoint agp,double aznach){
      tip = aex;
      znach = aznach;
      koord = agp;
    }
    int tip;//!типы экстремумов поля (0 - "+") (1 - "-")
    meteo::GeoPoint koord;//!координаты экстремумов поля
    double znach;//!значения экстремумов поля
};

//! срок
struct HourType {
  int p1; //!< первая точка времени
  int p2; //!< вторая точка времени
  int time_range; //!< смысловое значение p1 и p2
};

struct PropertyChooseData
{
  public:
  PropertyChooseData();
  PropertyChooseData(const QDateTime & dt,int aparam = P_PARAM, int atypelevel = 1, int alevel = 0, 
		     int srok = 0, int acenter =0, int amodel = 0);
  //  bool fillShoursList(int shour, int interval);
  bool getSteps(meteo::GeoPoint *steps);
  QString getParamName() const;
  static bool getSteps(NetType atypenet,meteo::GeoPoint *steps);
  QDateTime dateTimePrognoz() const;
  QDateTime dateTime() const;
  void clear();

  //интервал для выборки
  QDateTime beginDt; 
  QDateTime endDt;

  QString sdate;        //!< дата 
  QString stime;        //!< время
  QString scenter_name; //!< название центра 
  int scenter;          //!< номер центра
  double heigth;        //!< высота
  int slevelType;       //!< тип у ровная TODO для TMeteoData:1 - призменые данные; 0 - высотные; у GRIB для этого большая табличка
  QVector<int> slevels; //!< уровни в гПа
  ModelType smodel;     //!< модель прогнозирования
  //HourType hour;
  int shour;            //!< срок прогноза
  NetType stypenet;     //!< тип сетки
  obanalParam param;    //!< номер параметра

  // int svaluelevel; //!< уровень (гПа) * 10 (из-за уровня тропопаузы, максимального ветра TODO)
  // int shour; //!< наверное, то же, что sstrhour
  // int stype_info; //TODO наверное, убрать
  // int step_smooth; //TODO наверное, убрать 
  // QString sstrhour;   //!< срок прогноза
  // DataSource source; //!< TODO Убрать
  // double f10; //Индекс солнечной активности  TODO Убрать
  // double ap;//Индекс геомагнитной активности  TODO Убрать
};

QDataStream &operator>>(QDataStream &in, PropertyChooseData& prop);
QDataStream &operator<<(QDataStream &out, const PropertyChooseData& prop);
QTextStream &operator>>(QTextStream &in, PropertyChooseData& prop);
QTextStream &operator<<(QTextStream &out, const PropertyChooseData& prop);






enum isoTypes{
ISO_BAR = 100,
ISO_TEND = 101,
ISO_HYPSE = 102,
ISO_GIETS = 103,
ISO_THERM = 104,
ISO_TAKH = 105,
ISO_THERMSEA = 106,
ISO_VOLN = 107,
ISO_TROPO = 108,
ISO_MAXWIND = 109,
ISO_TUNDER = 110,
ISO_ANALOG = 111
};

struct IsoParam
{
  IsoParam(){};
  IsoParam(const obanalParam &,int svaluelevel =0);
  void set(const obanalParam &,int svaluelevel =0);
//  void fillIsoParam(int type_layer,bool is_bound,int svaluelevel=0);
  void fillIsoParam(float amin,float amax,float astep);
  void fillIsohybse(int svaluelevel);
  void fillIsotherm(int svaluelevel);

  QList<double> fiso; //!< значения изолиний
  double min; //!<  минимальное значение
  double max; //!<  максимальное значение
  int iso_min; //!<  индекс минимального значения
  int iso_max; //!<  индекс максимального значения
  double step; //!<  шаг
  double exmax; //!<  максимальное значение экстремума
  double exmin; //!<  минимальное значение экстремума
  double exstep; //!<  шаг
  QString unit; //< ед. измерения
  QString name; //!< название линии
  
};

struct RadarParam{
  
  meteo::GeoPoint center; //!< координаты
  int kol_x;           //! количество точек по x
  int kol_y;           //! количество точек по y
  int dx;           //!  размер элемента
  int dy;           //! размер элемента
  int proj_type;           //!< тип проекции (3 - merkator)
};


struct RegionParam{
  meteo::GeoPoint start; //!< координаты
  meteo::GeoPoint end; //!< координаты
  int start_i;
  int start_j;
  int kol_i;
  int kol_j;
  //   double start_fi;
  //   double end_fi;
  //   double start_la;
  //   double end_la;
  
  RegionParam(float f1,float l1,float f2,float l2){
    start.setFiDeg(f1);
    start.setLaDeg(l1);
    end.setFiDeg(f2);
    end.setLaDeg(l2);
  }
  RegionParam(){
    start.setFiDeg(-89.0);
    start.setLaDeg(-180.);
    end.setFiDeg(89.0);
    end.setLaDeg(180.);
    start_i =0;
    start_j =0;
    kol_i = 0;
    kol_j = 0;
  }
  
  bool isIn(float, float)const;
  
  friend QDataStream& operator<<(QDataStream& in,const RegionParam& gc);
  friend QDataStream& operator>>(QDataStream& out, RegionParam& gc);
   bool operator== (const RegionParam& right) const ;
   bool operator!=(const RegionParam &right) const;
  
};


struct isoLine{
  isoLine(){}
  isoLine(const QVector<meteo::GeoVector> &alines, double aval, const QString &aunit){
    lines = alines;
    value = aval;
    unit = aunit;
  }
  QVector<meteo::GeoVector> lines;
  double value;
  QString unit;
};

struct PropertyPerenosData
{
  bool traek;
  bool pryam;
  int vrem;
  int vrem_pech;
  double koef_pere;
  double heigth;
  PropertyChooseData property_data;
  QList<PropertyChooseData> chooseDataList;
};


class TFieldKey{
  public:
  TFieldKey() {
    clear();
  }
  
  TFieldKey(const obanalParam& aparam,const QDateTime &adate,const int &alevel,bool afact = false){
    _levelType = 0;
    _level=alevel;
    _date = adate;
    _param = aparam;
    _center = 0;
    _isfact = afact;
    _hour = 0; //TODO какое значение по умолчанию?
  }
  
  TFieldKey(const PropertyChooseData& prop) {
    clear();

    _levelType = prop.slevelType;
    if (prop.slevels.count() != 0) {
      _level = prop.slevels.at(0);
    }
    _hour = prop.shour;
    if(_hour != 0) 
      _isfact = false;
    else _isfact = true;
    _date =  prop.dateTimePrognoz();
    _param = prop.param;
    _center = prop.scenter;
  }
  
  void clear() {
    _level = -1;
    _levelType = -1;
    _date  = QDateTime();
    _param = OB_UNKNOWN_PARAM;
    _center = -1;
    _isfact = true;
    _hour  = -1; 
  }

  QDate date()const 	    {return _date.date();}
  QTime  time()const        {return _date.time();}
  QDateTime dateTime()const {return _date;}
  void setDateTime(const QDateTime& dt) { _date = dt; }
  
  int center() const 	      {return _center;}
  void setCenter(int center)  { _center = center; }

  int level() const  	      {return _level;}
  void setLevel(int level)    { _level = level; }
  
  int levelType() const  	      {return _levelType;}
  void setLevelType(int levelType)    { _levelType = levelType; }

  int hour() const  	    {return _hour;}
  void setHour(int hour)    { _hour = hour; }


  obanalParam param()const  {return _param;}
  void setParam(obanalParam param) { _param = param; }

  void printDebug();

  friend QDataStream &operator>>(QDataStream &in, TFieldKey& prop);
  friend QDataStream &operator<<(QDataStream &out, const TFieldKey& prop);

private:
  QDateTime _date;    //!< дата/время создания данных
  int _center; //!< номер центра

  int _levelType; //!< тип уровня
  int _level; //!< уровень
  int _hour;  //!< срок
  obanalParam _param; //!< номер параметра

  bool _isfact;
};

QDataStream &operator>>(QDataStream &in, TFieldKey& key);
QDataStream &operator<<(QDataStream &out, const TFieldKey& key);

inline bool operator==(const TFieldKey &e1, const TFieldKey &e2)
{
  return e1.level() == e2.level()
  && e1.date() == e2.date()
  && e1.time() == e2.time()
  && e1.center() == e2.center()
  && e1.param() == e2.param();
}

inline uint qHash(const TFieldKey &key){
  return ((key.dateTime().secsTo(QDateTime(QDate(key.date().year(), 1,1), QTime(0, 0)))/60)<<16) |
    key.center() | key.levelType()  | ( key.level()) | key.hour() | key.param();
}

#endif

