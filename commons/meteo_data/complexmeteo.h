#ifndef COMMONS_METEO_DATA_COMPLEXMETEO_H
#define COMMONS_METEO_DATA_COMPLEXMETEO_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/geobasis/geopoint.h>

//! метеоданные, при необходимости сгруппированные под общим дескриптором
class ComplexMeteo : public TMeteoData {
public:
  ComplexMeteo();
  ~ComplexMeteo();

  bool fill(const TMeteoData& data);
  void setCoord(const meteo::GeoPoint& coord)  { _coord = coord; }
  void setLevel(int levType, float levLo, float levHi = -1.f) {
    _levelType = levType;
    _levelLo = levLo;
    _levelHi = levHi;
  }
  void setDtType(int dtType) {
    _dtType = dtType;
  }
  void setDt1(const QDateTime& dt1) {
    _dt1 = dt1;
  }
  void setDt2(const QDateTime& dt2) {
    _dt2 = dt2;
  }
  void setDescriptor(descr_t descr) {
    _descriptor = descr;    
  }

  QString identJson(const QDateTime& dt) const;
  void printData(meteodescr::TimeParam* tp = 0, int lev = 0, int = -1, control::QualityControl maxQual = control::NO_OBSERVE) const;

  descr_t descriptor() const { return _descriptor; }
  int dtType() const { return _dtType; }
  const QDateTime& dtBeg()const { return _dt1; }
  const QDateTime& dtEnd()const { return _dt2; }
  int levelType()const { return _levelType; }
  float levelLo()const { return _levelLo; }
  float levelHi()const { return _levelHi; }
  const meteo::GeoPoint& coord()const { return _coord; }

  virtual ComplexMeteo& addChild();
  ComplexMeteo* createChild(descr_t complDescr = BAD_DESCRIPTOR_NUM);

  const QList<ComplexMeteo*> findComplChilds(descr_t complDescr) const;
  
  void toDataStream(QDataStream& ds) const;
  void fromDataStream(QDataStream& ds);

private:
  
  void fillIdentification(const TMeteoData& md, meteodescr::TimeParam* tp);
  bool fillChild(const TMeteoData& data, meteodescr::TimeParam* tp, ComplexMeteo* parent);
  bool copyDataFrom(const TMeteoData& md);
  void fillMeteo(const TMeteoData& md, bool* curexist, QMap<descr_t, ComplexMeteo*>* cmlist);
  
private:
  int _levelType = -1;
  float _levelLo = -1;
  float _levelHi = -1;
  meteo::GeoPoint _coord;
  
  int _dtType = -1;
  QDateTime _dt1;
  QDateTime _dt2;

  descr_t _descriptor = BAD_DESCRIPTOR_NUM;
};

const ComplexMeteo& operator>>(const ComplexMeteo& data, QByteArray& out);
ComplexMeteo& operator<<(ComplexMeteo& data, const QByteArray& ba);


#endif
