#ifndef TCONSISTENCY_SIGN_H
#define TCONSISTENCY_SIGN_H

#include "tcontrolmethods.h"

class TMeteoParam;

namespace control {
  struct ConsistSignLimit {
    // descr_t descr; //!< дескриптор значения для стандартного уровня
    // descr_t levelDescr; //!< дескриптор значения уровня стандартной точки
    descr_t signDescr; //!< дескриптор уроня особых точек
    descr_t signLevelDescr; //!< дескриптор значения уровня особой точки
    int level; //!< уровень выше и ниже которого разный предел
    float lowLimit; //!< макс отклонение значения особой точки от значения на стандартном уровне, если последний ниже level
    float hiLimit;  //!< макс отклонение значения особой точки от значения на стандартном уровне, если последний выше level  
  };

  struct ConsistSignSettings {
    QPair<descr_t,QStringList> type; //!< дескриптор, подходящие значения (категории)
    QMap<uint, uint> ranges; //!< стандартный уровень, отклонение от него
    QList<ConsistSignLimit> interpol; //!< интерполяция значений
    ConsistSignLimit hydro; //!< исп-ие основного уровнения гидростатики
  };
}


namespace tmcontrol {
  class TConsistSign : public TControlMethodBase {
  public:
    TConsistSign(const QString& dirName);
    ~TConsistSign();

    bool control(TMeteoData* data);
    QString key() { return "TConsistSign"; }

  private:
    virtual bool readSettings(QDomDocument* dom);
    void readLevelSettings(const QDomElement& el);
    void readLimits(const QDomElement& el, QList<control::ConsistSignLimit>& limitList);
    void readLimits(const QDomElement& child, control::ConsistSignLimit& limit);

    void controlSignificantLevel(TMeteoData* data);
    void controlSignificantLevel(const QMap<uint, uint>& ranges,
				 const control::ConsistSignLimit& limit, 
				 TMeteoData* data);

    int getSignLimit(float press);

    void controlSignificantHeight(TMeteoData* parent, const QList<uint>& kPstd);
    int indexLowLevel(float level, const QList<TMeteoData*>& dSign);

 
  private:
    control::ConsistSignSettings* _settings;
  };
}

//typedef TSingleton<tmcontrol::TConsistSign, const QString&> TConsistSign;

#endif

