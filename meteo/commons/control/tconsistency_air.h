#ifndef TCONSIST_AIR_H
#define TCONSIST_AIR_H

#include <cross-commons/singleton/tsingleton.h>
#include "tcontrolmethods.h"
#include "tmethodstempl.h"

namespace control {
  struct ConsistMatch {
    descr_t descr; //!< дескриптор
    QList<QPair<float,float> > segment; //!< интервалы значений
  };

  struct ConsistTableRange {
    QPair<float,float> segment; //!< интервалы для descrMatch (уровни)
    int diff;   //!< толщина слоя
    float step; //!< шаг изменения инверсии с изменением уровня
    float max;  //!< максимальное значение инверсии между уровнями
  };

  struct ConsistTableCheck {
    CheckValueType type; //!< тип интервалов (физ. велечина или кодовое значение)
    QList<ConsistMatch> match; //!< условия, при которых используются values (дескриптор, интервал)
    descr_t descr; //!< дескриптор проверямого значения
    descr_t descrMatch; //!< дескриптор уровня
    QList<ConsistTableRange> expr; //!< проверочные выражения
  };
  

  typedef QList<ConsistTableCheck> ConsistTableGroup; //!< группа в xml

  struct ConsistTableSettings {
    QPair<descr_t,QStringList> type; //!< дескриптор, подходящие значения (категории)
    QList<ConsistTableGroup> rules;
  };

  //! для проверки на сверхадиабатические вертикальные градиенты
  struct ConsistAdiabatSettings {
    descr_t descr; //!< дескриптор проверямого значения
    descr_t descrLevel;
    QList<ConsistTableRange> expr; //!< проверочные выражения
  };
}

namespace tmcontrol {
  //! проверка вертикального градиента профилей температуры
  class TConsistAir : public TControlTemplateBase<control::ConsistTableSettings, control::ConsistTableGroup> {
public:
  TConsistAir(const QString&);
  ~TConsistAir();
  
  QString key() {return "TConsistAir"; }

private:
    bool rulesMatch(const QPair<descr_t,QStringList>& type, TMeteoData* data);

    void readGroupSettings(control::ConsistTableGroup& group, QDomNode& node);
    bool readValueSettings(control::ConsistTableCheck& check, QDomNode el);
    bool readMatch(control::ConsistMatch& match, const QDomElement& el);
    bool readValue(control::ConsistTableRange& range, const QDomElement& el);
    void readAdiabatSettings(QDomNode parent);

    bool makeControl(const QList<control::ConsistTableGroup>& rules , TMeteoData* data);

    void controlInversion(const control::ConsistTableCheck& check, TMeteoData* data);
    bool rulesMatch(QList<control::ConsistMatch> match, TMeteoData* data);
    float getLimit(float p1, float p2, const QList<control::ConsistTableRange>& range);
    float getAdiabatLimit(float p, const QList<control::ConsistTableRange>& range) ;
    void controlSuperAdiabatic(TMeteoData* data, const control::ConsistAdiabatSettings& adiabat);

    void controlHydroBalance(TMeteoData* parent);
    void controlHydroBalance(TMeteoData* data, TMeteoData* dataNext, float Ttr, float Ptr,
			     float* E, control::QualityControl* qual);
    bool tropopauseLevel(float t1, float h1, float t2, float h2);
    void calcHeight(float T, float P, float Tnext, float Pnext, float* D, float* Da, float* Db);
    
    
  private:
    QList<control::ConsistAdiabatSettings>* _adiabat;

  };

}

//typedef TSingleton<tmcontrol::TConsistAir, const QString&> TConsistAir;

#endif
