#ifndef TMETEO_DESCR_H
#define TMETEO_DESCR_H

#include <cross-commons/singleton/tsingleton.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>

#include <qstring.h>
#include <qhash.h>
#include <qdebug.h>

#include <stdint.h>

class QDomNode;
class QDomDocument;
class QDateTime;

namespace meteo {
class GeoPoint;
} // meteo

namespace meteodescr {

  struct Property {
    QString units; //!< Единицы измерения, соответствующие написанию BUFR (для преобразования при необходимости)
    QString unitsRu; //!< Единицы измерения на русском
    bool single;   //!< true - если для дескриптора только один параметр, false - если может быть набор параметров
    bool anyGroup; //!< Может входить в любую группу
    QString description; //!< Описание 

    Property(const QString& u="", const QString& uRu="", bool s = false, bool a = false, const QString d = ""):
      units(u), unitsRu(uRu), single(s), anyGroup(a), description(d) {}
  };

  //! Описание вертикальной координаты
  // struct VerticType {
  //   int type;			//!< Тип уровня (в БД sprinf.level_types)
  //   QString level;		//!< Название дескриптора уровня или верхней границы
  //   QString levelLo;		//!< Название дескриптора уровня или нижней границы
  //   QString height;		//!< Название дескриптора высоты/глубины или верхней границы
  //   QString heightLo;		//!< Название дескриптора высоты/глубины или нижней границы
  // };

  //! Свойства дополнительных имен дескрипторов
  struct Additional {
    descr_t descr;		//!< Номер дескриптора
    QString description;        //!< Описание
    int index;			//!< Индеск в наборе
    int min;			//!< Минимальное кодовое значение
    int max;			//!< Максимальное кодовое значение
    Additional(descr_t d, const QString& str, int idx, int amin, int amax):descr(d),description(str), index(idx),min(amin),max(amax) {}
    Additional():descr(BAD_DESCRIPTOR_NUM), index(-1), min(-1), max(-1) {}
  };

class TMeteoDescriptor {
public:
  TMeteoDescriptor();
  ~TMeteoDescriptor();

  descr_t descriptor(const QString& name) const;    //!< номер дескриптора по имени
  descr_t descriptor(const char* name) const;    //!< номер дескриптора по имени
  descr_t descriptor(const std::string& name) const;

  bool isValid(descr_t descr) const { return (descr != BAD_DESCRIPTOR_NUM); }
  bool isSingle(descr_t descr) const { return _prop.value(descr).single; }
  bool contains(const QString& name) const { return _descr.contains(name); }
  bool isAdditional(const QString& name) const { return _add.contains(name); }
  bool isAddParent(const QString& name) const { return _addParent.contains(name); }
  bool isExist(descr_t descr) const;
  bool isIdentDescr(descr_t descr) const;
  bool isNonLocateIdentDescr(descr_t descr) const;
  bool isComponent(descr_t descr, descr_t* complDescr) const;
  bool isComponent(const QString& name, QString* complName) const;
  bool isComponent(const QString& name, descr_t* complName) const;
  bool isComplex(const QString& compex) const;

  bool isMeteo(descr_t descr) const;
  bool isAllCompex(const TMeteoData& md, descr_t* complDescr) const;

  QList<QString> names() const { return _descr.keys(); }  //!< список имён дескрипторов
  QList<QString> addNames() const { return _add.keys(); }  //!< список дополнительных имен
  QList<QString> allNames() const { return _descr.keys() + _add.keys(); }   //!< список имён дескрипторов + дополнительные

  QString name(descr_t descr) const;   //!< имя дескриптора по номеру

  const Property property(descr_t descr) const { return  _prop[descr]; }
  bool property(const QString& name,  Property* prop) const;
  QString description(const QString& name) const;
  QString description(descr_t descr, int order, float value) const; //!< для учета дополнительных
  QString findAdditional(descr_t descr, int order, float value) const;

  //  const QHash<descr_t, VerticType>& links();
  //  bool getVerticCoord(const TMeteoData& data, descr_t descr, int index, int* type, float* level, float* height);
  //  bool getVerticCoord(const TMeteoData& data, descr_t descr, int index, int* type, float* level);
  // bool getLayer(const TMeteoData& data, descr_t descr, int index, 
  // 		int* type, float* levelLo, float* levelHi, float* heightLo, float* heightHi);

  Additional additional(const QString& name) const { return _add.value(name); }

  QString stationIdentificator(const TMeteoData& data) const;
  QString stationIdentificator(const TMeteoData& data, const meteo::GeoPoint& gp, bool withName = false) const;
  QString stationName(const TMeteoData& data) const;
  QString stationShortName(const TMeteoData& data) const;
  QString stationFullName(const TMeteoData& data) const;
  
  int station(const TMeteoData& data, int defaultValue = BAD_METEO_ELEMENT_VAL) const; //!< номер станции
  int buoy(const TMeteoData& data, int defaultValue = BAD_METEO_ELEMENT_VAL) const; //номер буя
  bool getCoord(const TMeteoData& data, float *fi, float *la, float *h = 0) const; //!< местоположение данных
  bool getCoord(const TMeteoData& data, meteo::GeoPoint* point) const; //!< местоположение данных
  bool getDegCoord(const TMeteoData& data, meteo::GeoPoint* point) const;
  QDateTime dateTime(const TMeteoData& data) const; //!< дата/время данных
  bool getDayHour(const TMeteoData& data, int *day, int *hour, 
		  control::QualityControl* qual1, control::QualityControl* qual2) const; //!< день/час данных



  
  float convertUnit(descr_t descr, float val, const QString& srcUnit) const;   //!< преобразование величины в соответствии с единицами измерения
  float convertUnit(float val, const QString& srcUnit, const QString& dstUnit) const; //!< преобразование величины в соответствии с единицами измерения
  
  bool findVertic(const TMeteoData& src, int* levelType, float* levelLo, float* levelHi) const;
  //  bool findTime(const TMeteoData& src, int* type, QDateTime* start, QDateTime* end) const;
  bool findTime(const TMeteoData& src, TimeParam* tp, int* type, QDateTime* start, QDateTime* end) const;
  int mountainLevel(const TMeteoData& data) const;  

  void fillTypeLevel(TMeteoData* data) const;
  void setType(TMeteoData* data, meteodescr::LevelType type) const;

  void copyChildIdentification(TMeteoData* data, const TMeteoData* parent) const;
  void setFullIdentification(TMeteoData* data, TMeteoData* parent) const;
  QList<TMeteoData*> splitWithLevel(TMeteoData* data, int ltype, float levLo, float levHi) const;

  //  void modifyForSave(TMeteoData* data);
  
  
private:
  bool readFile(QDomDocument* xml, const QString& fileName); //!< загрузка файла в xml
  void read(QDomNode node); //!< парсинг xml с дескрипторами
  //  void readLinks(QDomNode node); //!< парсинг xml со связями дескрипторов
  void readAdditional(QDomNode node);
  void readComplex(QDomNode node);
  
private:
  QHash<QString, descr_t> _descr; //!< список дескрипторов имя-номер
  QHash<descr_t, Property> _prop; //!< список свойств измерения по номеру
  //  QHash<descr_t, VerticType> _links; //!< взаимосвязи дескрипторов (напр. тепература от уровня), (номер категории,(дескриптор родитель, список зависимых дескрипторов))

  QHash<QString, Additional> _add; //!< дополнительные имена дескрипторов
  QStringList _addParent; //!< список имен дескрипторов, имеющих дополнительные
  QMap<descr_t, descr_t> _complex;//!< child и его составной дескриптор
};

  descr_t create(uint group, descr_t descr); //!< добавление номера группы к дескриптору
  bool hasGroup( descr_t descr );
  int group( descr_t descr );
  descr_t  descrWithoutGroup( descr_t descr );

}

//QDebug& operator<<(QDebug& out, const meteodescr::VerticType& vt);

typedef TSingleton<meteodescr::TMeteoDescriptor> TMeteoDescriptor;

#endif
