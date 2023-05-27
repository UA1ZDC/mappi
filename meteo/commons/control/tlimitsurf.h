#ifndef TLIMIT_SIMPLE_H
#define TLIMIT_SIMPLE_H

#include <qpair.h>
#include <qmap.h>

#include "tcontrolmethods.h"
#include "tmethodstempl.h"

class QDomDocument;
class QDomNode;
class TMeteoParam;

namespace control {

  struct LimitCheck {
    int number; //!< порядковый номер правила
    CheckValueType type; //!< тип интервалов (физ. велечина или кодовое значение)
    QMap<descr_t, LimitRange> match;  //!< условия, при которых используются values (дескриптор, интервал)
    QMap<descr_t, LimitRange> values; //!< допустимые интервалы (дескриптор, интервал)
    LimitCheck(CheckValueType t = CODEFORM_CHECK):number(-1),type(t) {}
  };

  //! содержимое группы в xml
  typedef QList<LimitCheck> LimitCheckGroup; 

  struct LimitSettings {
    QPair<descr_t,QStringList> type; //!< дескриптор, подходящие значения (категории)
    QList<LimitCheckGroup> rules; //!< все правила
  };
}


namespace tmcontrol {
  class TLimitSimple: public TControlTemplate<control::LimitSettings, control::LimitCheckGroup, control::LimitCheck> {
  public:
    TLimitSimple(const QString& dirName);
    ~TLimitSimple();
    
    QString key() { return "TLimitSimple"; }
    
  private:
    void limitRulesCheck(const control::LimitCheck& rule, TMeteoData* data);
    bool readValueSettings(control::LimitCheck& check, const QDomElement& el);
  private:
  };
}

//typedef TSingleton<tmcontrol::TLimitSimple, const QString&> TLimitSimple;


#endif
