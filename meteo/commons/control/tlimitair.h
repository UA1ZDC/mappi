#ifndef TLIMITTABLE_H
#define TLIMITTABLE_H

#include "tcontrolmethods.h"
#include "tmethodstempl.h"

class QDomDocument;
class QDomNode;
class TMeteoParam;

namespace control {
  
  struct LimitTableCheck {
    int number; //!< номер правила
    CheckValueType type; //!< тип интервалов (физ. велечина или кодовое значение)
    QMap<descr_t, LimitRange> match; //!< условия, при которых используются values (дескриптор, интервал)
    descr_t descr;      //!< дескриптор проверяемого значения
    descr_t descrMatch; //!< дескриптор для уровня
    QMap<descr_t, LimitRange> values; //!< key - значение уровня
    LimitTableCheck(CheckValueType t = CODEFORM_CHECK):number(-1),type(t),descr(0),descrMatch(0) {}
  };

  typedef QList<LimitTableCheck> LimitTableGroup; //!< группы в xml

  struct LimitTableSettings {
    QPair<descr_t,QStringList> type; //!< дескриптор, подходящие значения (категории)
    QList<LimitTableGroup> rules;
  };

}

namespace tmcontrol {

 class TLimitTable: public TControlTemplate<control::LimitTableSettings, control::LimitTableGroup, control::LimitTableCheck> {
public:
  TLimitTable(const QString& dirName);
  ~TLimitTable();

  QString key() { return "TLimitTable"; }

  private:
   void limitRulesCheck(const control::LimitTableCheck& rule, TMeteoData* data);
   bool readValueSettings(control::LimitTableCheck& check, const QDomElement& el);
};

}

//typedef TSingleton<tmcontrol::TLimitTable, const QString&> TLimitTable;

#endif
