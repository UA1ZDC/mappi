#ifndef TCONSISTENCY_WIND
#define TCONSISTENCY_WIND

#include <cross-commons/singleton/tsingleton.h>
#include "tcontrolmethods.h"
#include "tmethodstempl.h"

namespace control {
  
  struct ConsistWindRange {
    QList<QPair<float,float> > levelMatch; //!< интервалы уровней
    int max; //!< предельное значение проверяемой величины
  };

  struct ConsistWindCheck {
    QList<QPair<float,float> > matchSegment; //!< значения для выбора условия по descrMatch
    QList<ConsistWindRange> expr; //!< выражения для descr
  };

  struct ConsistWindGroup {
    CheckValueType type; //!<  тип интервалов (физ. велечина или кодовое значение)
    descr_t descr; //!< дескприптор проверяемого значения
    descr_t descrLevel; //!< дескриптор уровня
    descr_t descrMatch; //!< дескриптор для выбора условия
    QList<ConsistWindCheck> range; //!< диапазоны
  };

  struct ConsistWindSettings {
    QPair<descr_t,QStringList> type; //!< дескриптор, подходящие значения (категории)
    QList<ConsistWindGroup> rules;
  };
};

namespace tmcontrol {

  class TConsistWind : public TControlTemplateBase<control::ConsistWindSettings, control::ConsistWindGroup> {
  public:
    TConsistWind(const QString&);
    ~TConsistWind();
    
    QString key() {return "TConsistWind"; }
  private:
    
    bool makeControl(const QList<control::ConsistWindGroup>& rules, TMeteoData* data);
    bool controlWind(const control::ConsistWindGroup& rule, TMeteoData* data);
    float getRangeLimit(const QList<control::ConsistWindRange>& range, float level);

    void readGroupSettings(control::ConsistWindGroup& group, QDomNode& node);
    bool readValueSettings(control::ConsistWindGroup& group, QDomElement parent);
    void readRange(control::ConsistWindCheck& check, QDomNode node);
  };

}

//typedef TSingleton<tmcontrol::TConsistWind, const QString&> TConsistWind;

#endif
