#ifndef TCONSISTENCY_TIME_H
#define TCONSISTENCY_TIME_H

#include <cross-commons/singleton/tsingleton.h>
#include "tcontrolmethods.h"
#include "tmethodstempl.h"

namespace control {

  struct TimeRange {
    QPair<bool,float> min; //!< минимальное значение, если есть
    QPair<bool,float> max; //!< максимальное значение, если есть
    TimeRange()/*:subDescr(-1),addDescr(-1)*/ {}
  };

  struct  TimeCheck {
    QMap<QString, TimeRange> match; //!< условия, при которых используются values
    QMap<descr_t, TimeRange> expr; //!< выражения для проверки
  };

  //! группа в xml
  struct TimeGroup {
    bool isCustom; //!< false - общие правила, true - специальная обработка
    QList<TimeCheck> check; 
  };

  struct TimeSettings {
    QPair<descr_t,QStringList> type; //!< дескриптор, подходящие значения (категории)
    QList<TimeGroup> rules;
  };
}

namespace tmcontrol {
  class TConsistTime: public TControlTemplateBase<control::TimeSettings, control::TimeGroup> {
  public:
    TConsistTime(const QString& dirName);
    ~TConsistTime();
    
    QString key() { return "TConsistTime"; } 

    bool control(TMeteoData* cur, TMeteoData* prev);

  private:
    bool makeControl(const QList<control::TimeGroup>& rules, TMeteoData* cur);
    void limitRulesCheck(const QMap<descr_t, control::TimeRange>& rule, TMeteoData* cur, TMeteoData* prev);
    void limitPressueCheck(const control::TimeCheck& check, TMeteoData* cur, TMeteoData* prev);
    bool checkLimits(const control::TimeRange& range, float val);
    bool limitRulesMatch(const QMap<QString, control::TimeRange>& match, TMeteoData* cur, TMeteoData* prev);

    void readGroupSettings(control::TimeGroup& group, QDomNode& node);
    bool readValueSettings(control::TimeCheck& check, QDomElement el);
    void readRanges(control::TimeRange& range, QString& name, QDomElement el);

  private:
    TMeteoData* _prev;
  };
  
}

//typedef TSingleton<tmcontrol::TConsistTime, const QString&> TConsistTime;

#endif //TCONSISTENCY_TIME_H
