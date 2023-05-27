#ifndef TCONSISTENCY_H
#define TCONSISTENCY_H

#include <cross-commons/singleton/tsingleton.h>
#include "tcontrolmethods.h"
#include "tmethodstempl.h"

class TMeteoData;
class QDomNode;

namespace control {
  
  struct ConsistencyRange {
    int number; //!< номер выражения
    CheckValueType type;
    int index; //!< если несколько значений для дескриптора, а использовать надо только одно (-1 - все)
    descr_t subDescr; //!< дескриптор значения, которое надо вычесть
    int subIndex;  //!< номер, если значение несколько у subDescr
    QStringList str;  //!< допустимые строковые значения
    QList<float> val; //!< допустимые числовые значение
    bool negation;    //!< отрицание условия, используется только с val (т.е. недопустимые числ. значения)
    QList<QPair<float, float> > segment; //!< допустимые диапазоны [min, max]
    QPair<bool,float> min; //!< минимальное значение, если есть
    QPair<bool,float> max; //!< максимальное значение, если есть
    ConsistencyRange():number(-1),index(-1),subDescr(-1),subIndex(-1),negation(false),min(false,0),max(false,0) {
    }
  };

  typedef QMultiMap<QString, ConsistencyRange> ConsistencyCheck; //!< дескриптор, набор выражений для него

  //! группа в xml
  struct  ConsistencyGroup {
    QList<descr_t> descrList; //!< список дескрипторов, для которых определяется качество
    QualityControl qual;   //!< значение качества для значений, соответствующих правилам
    ConsistencyCheck match; //!< условия, при которых используются values (дескриптор, интервал)
    QList<ConsistencyCheck> expr; //!< выражения для проверки
  };

  struct ConsistencySettings {
    QPair<descr_t,QStringList> type; //!< дескриптор, подходящие значения (категории)
    QList<ConsistencyGroup> rules;
  };

};

namespace tmcontrol {

class TConsistency :public TControlTemplateBase<control::ConsistencySettings, control::ConsistencyGroup> {
public:
  TConsistency(const QString& dirName);
  ~TConsistency();

  QString key() { return "TConsistency"; }

private:
  bool readValueSettings(control::ConsistencyCheck& check, const QDomElement& el);
  void readGroupSettings(control::ConsistencyGroup& group, QDomNode& node);
  void readCheckSettings(control::ConsistencyGroup& group, const QDomElement& el);
  void readRanges(control::ConsistencyCheck& values, const QDomElement& el, int number);

  bool makeControl(const QList<control::ConsistencyGroup>& rules , TMeteoData* data);
  bool limitRulesMatch(const control::ConsistencyCheck& rule, TMeteoData* data);
  bool checkCodeMatch(const QString& descr, const control::ConsistencyRange& range, TMeteoData* data);
  bool checkCodeMatch(const control::ConsistencyRange& range, const QString& code, const QString& subCode);
  bool checkLimits(const control::ConsistencyRange& range, float value);
  bool descrExist(QList<descr_t> descrList, TMeteoData* data);
  void setQuality(const QList<descr_t>& descrList, control::QualityControl qual, TMeteoData* data);

  void controlPress(TMeteoData* data);
};
 
}

//typedef TSingleton<tmcontrol::TConsistency, const QString&> TConsistency;

#endif
