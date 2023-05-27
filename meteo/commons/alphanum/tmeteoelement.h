#ifndef TMETEO_ELEMENTS_H
#define TMETEO_ELEMENTS_H

#include<qstring.h>
#include<qregexp.h>
#include <qmap.h>
#include <qhash.h>
#include <commons/meteo_data/meteo_data.h>

class QDomDocument;

#define DEFAULT_ELEM_CTRL_REGEXP "[\\d/]{1,5}"

typedef float (*FuncGetValue)(const TMeteoData* data, const QString& strelem, bool* ok);

namespace meteo {

  enum ElementConvertType {
    SIMPLE_CONVERT = 0, //!< преобразование во float с учётом множителя mul
    TABLE_CONVERT  = 1, //!< табличное преобразование
    CUSTOM_CONVERT = 2, //!< используется спец. функция
    COMPLEX_CONVERT = 3,  //!< используется спец. функция
    TRANSLIT_CONVERT = 4, //!< восстановление кириллицы
    CONVERT_CNT    = 5
  };



  //NOTE с ключом QString qhash работает медленнее, чем с int (при 1000000 считываний на 20мс). Если понадобиться ускорить, то надо ввести свой внутренни номер, не связанный с номером дескриптора, используемым для передачи (и последующего сохранения в БД)
  struct Descriptor {
    // QString name;  	     //!< название элемента кодовой формы, теперь ключ в qhash
    QString outname;  	     //!< внешнее имя для получения номера дескриптора
    QRegExp rx;       	     //!< для semanticControl
    ElementConvertType type; //!< тип преобразования в физ. вел-ну
    QMap<QString,float> table;   //!< таблица для преобразования
    float mul; 		     //!< множитель для преобразования
    float add; 		     //!< слагаемое для преобразования
  };
};


class MeteoElement
{
public:
  MeteoElement();
  ~MeteoElement();

  float getValue(const QString& name, const QString& strelem, control::QualityControl* qual) const;
  bool contains(const QString& name) const { return _descr.contains(name); }

  QString outName(const QString& name) const;
  descr_t getDescriptor(const QString& name) const;
  meteo::ElementConvertType convertType(const QString& name) const;

  void setMeteoData(TMeteoData* data) { _data = data; }

  bool parseComplex(const QString& name, const QString& str, TMeteoData* data) const;
  
private:
  void readSettings(const QString& file);
  void readSettings(QDomDocument* dom);
  void registerGetValueFunc();
  void registerGetValueFunc(const QStringList& dl, FuncGetValue func);

  float customConvert(const QString& name, const QString& strelem , bool* ok) const;
  float complexConvert(const QString& name, const QString& strelem , bool* ok);

  bool semanticControl(const QString& name, const QString& strelem) const {
    return _descr.value(name).rx.exactMatch(strelem);
  };
  

private:
  QHash<QString, meteo::Descriptor> _descr;
  QHash<QString, FuncGetValue> _funcs; //!< Функции для преобразования кодового значения в число
  
  TMeteoData* _data;
};


#endif

