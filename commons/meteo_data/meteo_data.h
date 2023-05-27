#ifndef METEODATA_H
#define METEODATA_H

#include <qstring.h>
#include <qdatetime.h>
#include <QMultiHash>
#include <qtextstream.h>
#include <limits>

#define CODE_STRING_MAX_LENGTH 7

#define BAD_METEO_ELEMENT_VAL -9999
#define BAD_DESCRIPTOR_NUM -1

//! тип для номера дескриптора
typedef qint64 descr_t;
typedef uint8_t ctrlfl_t; //!< тип для флага контроля

class TMeteoData;
namespace meteodescr {
  class TMeteoDescriptor;
}

namespace control {
  //wmo 305, гл.6, табл. 20
  enum QualityControl {
    RIGHT              = 0, // прошло контроль
    HAND_CORRECTED     = 1, // корректировалось оператором
    AUTO_CORRECTED     = 2, //корректировалось программой
    ABSENT_CORRECTED   = 3, // не было значения, рассчитывалось программой
    DOUBTFUL_CORRECTED = 4,
    MISTAKEN_CORRECTED = 5,
    HAND_CORR_DOUBTFUL = 6,
    HAND_CORR_MISTAKEN = 7,
    HAND_CORR_ABSENT   = 8,
    NO_CONTROL         = 9,  //не проверялось
    SPECIAL_VALUE      = 10, //специализированное значение
    DOUBTFUL           = 11, // сомнительное
    MISTAKEN           = 12, // не прошло контроль
    DOUBTFUL_STATION   = 13, // параметр по станции считается сомнительным
    NO_OBSERVE         = 9999
  };

  //  int qualityCompare(QualityControl q1, QualityControl q2);

  QString titleForQuality( int qual );

  //маска для выбора/установки типа контроля
  enum ControlType {
    NOCONTROL    = 0x0, //!< контроль не проведён
    LIMIT_CTRL   = 0x1, //!< проверка на предельные значения (правдоподобие)
    CONSISTENCY_CTRL = 0x2, //!< на согласованность
    CONSISTENCY_SIGN = 0x4, //!< на согласованность
    SPACE_CTRL  = 0x8,  //!< пространственный контроль
    TIME_CTRL   = 0x10, //!< согласованность по времени между двумя телеграммами
  };

}

namespace meteodescr {
  //! Тип уровня, дескриптор "level_type"
  enum LevelType {
    kUnknownLevel = 0,		//!< в т.ч. статистические данные, отклонения
    kSurfaceLevel = 1,		//!< Земная или водная поверхность
    kCloudBaseLevel = 2,	//!< Уровень основания облаков		  
    kCloudTopLevel  = 3,	//!< Уровень вершин облаков		  
    kIsotermLevel   = 4,	//!< Уровень изотермы 0°C (-20°C, -10°C)
    kAdiabatCondensLevel = 5,	//!< Уровень адиабатической конденсации, отстоящий от поверхности
    kMaxWindLevel   = 6,	//!< Уровень максимального ветра	  		  
    kTropoLevel	    = 7,	//!< Тропопауза				  
    kTopAtmoshLevel = 8,	//!< Номинальная верхняя граница атмосферы
    kSeaBottomLevel = 9,	//!< Морское дно
    kAllAtmosphere  = 10,	//!< Вся атмосфера    
    kCbBaseLevel    = 11,       //!< Основание кучево-дождевого (CB) облака, м
    kCbTopLevel     = 12,       //!< Вершина кучево-дождевого (CB) облака, м
    kIsobarLevel    = 100,	//!< Изобарическая поверхность, гПа
    kMeanSeaLevel   = 101,	//!< Средний уровень моря						    
    kHeightMslLevel = 102,	//!< Установленная высота над средним уровнем моря, м			    
    kHeightLevel    = 103,	//!< Установленный уровень высоты над поверхность, м
    kDepthSurfLevel = 106,	//!< Глубина ниже поверхности земли, м
    kGeopotentialLevel = 112,   //!< Геопотенциальная высота, гп.м
    kDepthLevel     = 160,	//!< Глубина ниже уровня моря, м
    kAllOcean       = 201,	//!< Весь океан
    kDepthPressLevel = 256,     //!< Давление воды (TODO, переводить в метры ?)
    kOt500to1000    = 15000 //!< ОТ500/1000
  };

  //! Значимость времени. Дескриптор t_sign(08021).
  enum TimeType {
    kNormalTime = 0,
    kSeriesTime  = 1,		//!< Временные ряды
    kAvgTime     = 2,		//!< Усредненные по времени (единицы постоянно усредняются по периоду времени)
    kAccumTime   = 3,		//!< Накопленные
    kForecastTime = 4,		//!< Прогноз
    kForecastSeriesTime = 5,	//!< Прогноз по временным рядам
    kForecastAvgTime    = 6,	//!< Прогноз, усредненный по времени (единицы постоянно усредняются по периоду времени)
    kForecastAccumTime  = 7,	//!< Прогноз накопленный
    //...
    kAnalyseTime = 16,		//!< Анализ
    kStartPhenTime = 17,	//!< Начало явления
    kZondLaunch    = 18,        //!< Время запуска зонда
    //...
    kNominalTime = 25,		//!< Номинальный срок сообщения (используется для указания фактического срока измерения из группы 9GGgg FM12)
    kLastPosTime = 26		//!< Время последнего известного положения
    //...
  };

  enum tt_type {
    kUnkTT = -1,
    kFM = 0,
    kTL = 1,
    kAT = 2
  };

  struct MeteoDateTime {
    int year;
    int month;
    int day;
    int hour;
    int min;
    float sec;  //!< float для точности в мкс

    MeteoDateTime():year(-1), month(-1), day(-1), hour(-1), min(-1), sec(std::numeric_limits<float>::max()) {}
    MeteoDateTime(bool):year(0), month(0), day(0), hour(0), min(0), sec(0) {}
    MeteoDateTime(const MeteoDateTime& m):year(m.year), month(m.month), day(m.day), 
					  hour(m.hour), min(m.min), sec(m.sec) {}
    MeteoDateTime(int y, int mo, int d, int h, int m, int s) :
      year(y), month(mo), day(d), hour(h), min(m), sec(s) {}
    void set(const MeteoDateTime& m) {
      year = m.year; month = m.month; day = m.day;
      hour = m.hour; min = m.min;     sec = m.sec;
    }
    void readUp(const TMeteoData& md, int dy, int dmo, int dd, int dh, int dm, int ds, int add_descr = 0);
    void readCur(const TMeteoData& md, int dy, int dmo, int dd, int dh, int dm, int ds, int add_descr = 0);
    void setIfEmpty(const MeteoDateTime& m);

    bool toQDateTime(QDateTime* dt);
  };

  //! Для передачи от родителей к дочерним параметров времени
  struct TimeParam {
    int tf_sign;
    int tf_count;
    MeteoDateTime dt0;
    MeteoDateTime dt1;
    int tt;
    MeteoDateTime dt_tt;
    int tttt;
    TimeParam():tf_sign(-1), tf_count(0), tt(kUnkTT), tttt(-1) {}
  };

}



//! Описание метеопараметра
class  TMeteoParam
{
public:
 
  TMeteoParam(const QString& ac, float av, control::QualityControl aq, const QString& descrip = QString());
  TMeteoParam(const QString& ac, float av, control::QualityControl aq,
	      ctrlfl_t controlFlag, const QString& descrip);
  TMeteoParam();
  ~TMeteoParam();
 
  QString code() const { return _code; }
  float value() const { return _value; }
  //QChar quality()const { return QChar(_quality); }
  control::QualityControl quality()const { return _quality; }
  QString description() const { return _description; }
  uint8_t controlFlag() const { return _controlFlag; }

  bool isInvalid() const; //!< Ошибочное или нет
  bool isValid() const;   //!< Верное значение (не ошибочное, не сомнительное)

  void setQuality(control::QualityControl qual, ctrlfl_t controlFlag, bool anyway);
  void setQuality(QChar qual, ctrlfl_t controlFlag, bool anyway);
  
  void setValue(float val) { _value = val; }
  void setCode(const QString& code) { _code = code; }
  void setDesription(const QString& descrip) { _description = descrip; }
  
  bool equal( const float& other )const;

  QString toString() const;

  bool operator==( const TMeteoParam& other )const;
  friend QDataStream &operator>>(QDataStream &in, TMeteoParam &mtp);

private: 
  QString _code;		//!< Кодовое значение метеопараметра
  float _value;			//!< В виде числа
  control::QualityControl _quality;  //!< Показатель качества
  QString _description;		//!< Описание параметра
  uint8_t _controlFlag;         //!< Маска контроля (бит в 1 - данный вид контроля проводился)
};


QDataStream &operator<<(QDataStream &out, const TMeteoParam &mtp);
QDataStream &operator>>(QDataStream &in, TMeteoParam &mtp);

QDebug& operator<<(QDebug& out, const TMeteoParam& mtp);

struct ParamsForDb {
  QString descriptor;	//!< Базовая часть дескриптора
  QString group;		//!< Группа дескриптора
  QString index;		//!< Индекс параметра
  QString val;			//!< Числовое значение
  QString code;			//!< Строковое значение
  QString qual;			//!< Показатель качества
  QString levelType;	//!< Тип уровня
  QString levelLo;		//!< Уровень, верхняя граница
  QString levelHi;		//!< Уровень, нижняя граница
  QString dtType;		//!< Тип даты/времени
  QString dtStart;		//!< Начальное время (или в соотв. с типом)
  QString dtEnd;		//!< Конечно время (или в соотв. с типом)
  QString number;		//!< Номер узла
  QString fi;           //!< широта станции
  QString la;           //!< долгота станции
  QString height;       //!< высота станции
  QString stnumber;       //!< индекс станции
  QString parent;       //!< Номер родительского узла
};

class TMeteoData
{
public:
  TMeteoData();
  TMeteoData(const TMeteoData& src);
  virtual ~TMeteoData();

  bool isEmpty() const { return (_data.isEmpty() && _childs.size() == 0); }
  
  void clear() { _data.clear(); _childs.clear(); }

  const QList<descr_t> getDescrList() const { return _data.keys(); }

  QStringList allNames() const;

  void addChildParamsForDb(const TMeteoData& data, meteodescr::TimeParam* tp, uint parent, int* num, int mountainLev, ParamsForDb* pdb) const;
  int getParamsForDb(const QStringList &descrList, QString *adescr, QString *aadd_descr,
		     QString *aval, QString *aqual)const;
  int getParamsForDb(ParamsForDb* pdb)const;
  QString toJson(int* levelType, bool isChild = false) const;
  QString toJson(const QString& header, bool isChild /*= false*/) const;
  //  virtual QString identJson(const QDateTime& dt) const;

  virtual TMeteoData& addChild();
  int childsCount() const { return _childs.count(); }
  const QList<TMeteoData*>& childs() const { return _childs; }
  TMeteoData* child(int idx) { if (idx >= _childs.count()) return 0; return _childs[idx]; }
  const TMeteoData* child(int idx) const { if (idx >= _childs.count()) return 0; return _childs[idx]; }

  bool hasParent() const { return 0 != _parent; }
  const TMeteoData* parent() const { return _parent; }
  TMeteoData* parent() { return _parent; }

  
  //----- добавление метеопараметров
  void add(descr_t descr, const TMeteoParam& param, int add_descr);
  void add(descr_t descr, const TMeteoParam& param, int add_descr, bool wichoutCheck);
  
  void add(descr_t descr, const QString& ac, float aval, control::QualityControl aquality) {
    add(descr, TMeteoParam(ac, aval, aquality)); 
  }
  void add(descr_t descr, const TMeteoParam& param);
  void addNoSingle(descr_t descr, const TMeteoParam& param);
  void add(descr_t descr, const QString& ac, float aval, control::QualityControl, 
	   const QString unit);  
  void add(const QString& name, const TMeteoParam& param);

  void set(const TMeteoData& data);

  //добавление с заменой последнего
  void set(descr_t descr, const QString& acode, float aval, control::QualityControl aquality) {
    set(descr, TMeteoParam(acode, aval, aquality));
  }
  void set(descr_t descr, const TMeteoParam& param);

  void set(const QString& name, const QString& acode, float aval, control::QualityControl aquality) {
    set(name, TMeteoParam(acode, aval, aquality));
  }
  void set(const QString& name, const TMeteoParam& param);

  void setStation(int station_number);
  //! Устанавливает station_index или CCCC
  void setStation(const QString& station);
  void setCCCC( const QString& CCCC );
  void setDateTime(const QDateTime &adt );
  void setCoord(float fi, float la);
  void setCoord(float fi, float la, float h);

  //
  TMeteoData* findParent(descr_t descr);
  TMeteoData* findParent(const QString& name);
  const TMeteoData* findParent(descr_t descr) const;
  const TMeteoData* findParent(const QString& name) const;
  QList<const TMeteoData*> findChildsConst(descr_t descr) const;
  QList<TMeteoData*> findChilds(descr_t descr);
  QList<TMeteoData*> findChilds(const QString& name);

  QList<TMeteoData*> findChilds(descr_t descr, meteodescr::LevelType type);
  const TMeteoData* findChild(meteodescr::LevelType type, int lo, int hi) const;
  const TMeteoData* findChild(meteodescr::LevelType type, int lo) const;
  const TMeteoData* findChild(meteodescr::LevelType type) const;

  QList<TMeteoData*> takeChilds(descr_t descr);

  bool hasParam(descr_t descr, meteodescr::LevelType type) const;
  bool hasParam(descr_t descr, bool withChilds = true) const;
  bool hasParam(const QString& name, bool withChilds = true) const;
  uint countParam(descr_t descr, bool withChilds)const;
  int count() const { return _data.count(); }
  //  int findIndex(descr_t descr, float val) const;

  //  void printData_old(int lev = 0, int childNum = -1, control::QualityControl maxQual = control::NO_OBSERVE) const;
  void toText(QTextStream* out) const;
  virtual void printData(meteodescr::TimeParam* tp = 0, int lev = 0, int childNum= -1, control::QualityControl maxQual = control::NO_OBSERVE) const;
  virtual QString toString(int lev = 0) const;

  QList<QPair<descr_t, TMeteoParam> > allParams() const;  
  QMap<QString, TMeteoParam> allByNames(bool withChilds = true) const;  

  //----- чтение по имени
  //в первую очередь, чтоб учесть доп.имена. В иных случаях лучше пользоваться TMeteoDescriptor::descriptor() - проще
  //искать в коде, если понадобиться что-то исправить
  const TMeteoParam& meteoParam(const QString& name, bool* ok = 0) const;
  TMeteoParam* meteoParamPtr(const QString& name, bool withChilds = true);
  const QMap<int, TMeteoParam> meteoParamList(const QString& name) const;

  //----- чтение по номеру дескриптора от текущего узла к потомкам (соответственно, без доп. имён)
  QString getCode(descr_t descr, bool withChilds = true) const; 
  bool getCode(descr_t descr, QString* code, control::QualityControl* qual) const;

  float getValue(descr_t descr, float defValue, bool withChilds = true) const;
  bool getValue(descr_t descr, float* value, control::QualityControl* qual, bool withChilds = true) const;
  bool getValue(descr_t descr, int* value, control::QualityControl* qual) const;
  bool getValue(descr_t descr, int index, float* val, control::QualityControl* qual, bool withChilds = true) const;

  const TMeteoParam& getParam(descr_t descr, bool withChilds = true) const;
  const TMeteoParam& getParam(descr_t descr, int index, bool* ok = 0, bool withChilds = true) const;        
  const QMap<int, TMeteoParam> getParamList(descr_t descr, bool* ok = 0 ) const;

  TMeteoParam* paramPtr(descr_t descr, TMeteoData*& owner, bool withChilds);
  TMeteoParam* paramPtr(descr_t descr, bool withChilds = true);
  TMeteoParam* paramPtr(descr_t descr, int index, bool withChilds = true);
  QMap<int, TMeteoParam>& operator[](descr_t descr) { return _data[descr]; }

  //  QList<TMeteoParam*> getParamList(descr_t descr, const QList<int>& indexes);

  // const TMeteoParam& getCorrespondParam(descr_t baseDescr, uint baseValue, descr_t descr) const;
  // int getIdexList(descr_t descr, QList<int>* indexes)const;
  // int meteoDataPoIndex(int index,TMeteoData *dataNew )const;


  //Up - поиск с учетом родителей
  QString getCodeUp(descr_t descr) const; 
  bool  getCodeUp(descr_t descr, QString* code, control::QualityControl* qual) const;
  float getValueUp(descr_t descr, float defValue) const;
  float getValueUp(descr_t descr, int index, float defValue) const;
  bool getValueUp(descr_t descr, float *ret_val, control::QualityControl *qual) const;
  const TMeteoParam& getParamUp(descr_t descr) const;

  //только текущий узел
  float getValueCur(descr_t descr, int index, float defValue) const;
  float getValueCur(descr_t descr, float defValue) const { return getValue(descr, defValue, false); }

  int remove(const QString& name, bool withChilds = true);
 
  //-----
  //  bool smartUnion(const TMeteoData& dataNew);
  //void unite(const TMeteoData& dataNew);
    
  TMeteoData& operator=(const TMeteoData& );
  void copyFrom(const TMeteoData&);
  // bool operator==( const TMeteoData& other )const { return _data == other._data; }
  virtual void toDataStream(QDataStream& ds) const;
  virtual void fromDataStream(QDataStream& ds);
  
  friend QDataStream &operator<<(QDataStream &out, const TMeteoData &mtd);
  friend QDataStream &operator>>(QDataStream &in,TMeteoData &mtd);
  friend class meteodescr::TMeteoDescriptor;

private:

  const TMeteoParam& first(descr_t descr) const { return _data.value(descr).constBegin().value(); }
  
  // bool unionWithLink(const QHash<descr_t, QList<descr_t> >* links, const TMeteoData& dataNew, QList<descr_t>&
  // checkedKeys);
  // bool unionWithLink(const QHash<descr_t, QList<descr_t> >* links, descr_t pd, const TMeteoData& dataNew);
  // bool unionSingle( const TMeteoData& dataNew, QList<descr_t>& checkedKeys);

protected:
  void addChild(TMeteoData* parent, TMeteoData* child) { parent->_childs.append(child); child->_parent = parent; }
private:

  QHash<descr_t, QMap<int, TMeteoParam>> _data; //!< Метеоданные. <дескриптор, <индеск, метеопараметр> >. Индекс
  // формируется с учетом заданного базового индекса методом инкрементирования
  QList<TMeteoData* > _childs;
  TMeteoData* _parent = 0;

  TMeteoParam _empty; //!< Пустышка

public:
  //auto data() const -> const decltype(_data)  { return _data; } //временно!!
  const QHash<descr_t, QMap<int, TMeteoParam>>& data() const  { return _data; } //временно!!
};


QDataStream &operator>>(QDataStream &in, TMeteoData &mdt);
QDataStream &operator<<(QDataStream &out, const TMeteoData &str);

const TMeteoData& operator>>(const TMeteoData& data, QByteArray& out);
TMeteoData& operator<<(TMeteoData& data, const QByteArray& ba);
// void convert(TMeteoData* dataList, const QList<QByteArray>& ba);
// void convert(QList<TMeteoData>& data, const QList<QByteArray>& ba);


#endif


