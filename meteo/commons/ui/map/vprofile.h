#ifndef METEO_COMMONS_UI_VERTICALCUT_CORE_VERTICALCUT_H
#define METEO_COMMONS_UI_VERTICALCUT_CORE_VERTICALCUT_H

#include <qmap.h>
#include <qstring.h>
#include <qvector.h>

#include <commons/geobasis/geopoint.h>
#include <commons/meteo_data/meteo_data.h>

#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/placedata.h>
#include <meteo/commons/ui/map/geoaxis.h>

namespace obanal {
class TField;
} // obanal

namespace meteo {
namespace map {

enum VPLayerType {
  CUT_LAYER_H  =  1,
  CUT_LAYER_T  =  2,
  CUT_LAYER_Td =  3,
  CUT_LAYER_D  =  4,
  CUT_LAYER_dd =  5,
  CUT_LAYER_ff =  6,
  CUT_LAYER_uu =  7,
  CUT_LAYER_vv =  8,
  CUT_LAYER_TROPO   = 10,//Тропопауза
  CUT_LAYER_M_WIND  = 11,//Максимальный ветер
  CUT_LAYER_NANOSKA = 12,
  CUT_LAYER_OBLAK   = 13,
  CUT_LAYER_OBLED   = 14,
  CUT_LAYER_BOLT    = 15,
  CUT_LAYER_TRACE   = 16,
  CUT_LAYER_AXIS    = 17,//Оси по станциям
  LAST_CUT_LAYER    = 18
};

//! Данный тип используется для обозначения вида вертикального разреза атмосферы.
enum Type {
  kSpaceType,     //!< Пространственный.
  kTimeType,      //!< Временной.
  kSpaceTimeType  //!< Пространственно-временной.
};

//! Класс VerticalCut предоставляет интерфейс для расчёта вертикального разреза атмосферы по аэрологическим данным.
class VProfile
{
public:
  //! Данный тип используется для обозначения метеопараметра для которого расчитан вертикальный разрез.
  // enum MeteoParam { kT, kTd, kH, kdd, kff };

  //!
  VProfile();
  //!
  ~VProfile();

  void setData(QList<zond::PlaceData>* zonds, const QVector<float>& levels);
  //!
  bool getTropo(QVector<DecartData>* temperature);
  //!
  bool getMaxWind(QVector<DecartData>* dd, QVector<DecartData>* ff);
  //!
  bool initField(obanal::TField* field, int type_r) ;
  //!
  bool calc(obanal::TField* field, int type_r) ;
  //!
  void clear();
  bool getCoord(int x, int y, GeoPoint *gp);

private:
  //!
  /* bool calc(obanal::TField* field, const QVector<DecartData>& srcData, const QList<float>,
            float minP,float maxP,float minX,float maxX ) const;*/

  void findMin(float* x, float* y, const QList<DecartData>& vector) const;
  void findMax(float* x, float* y, const QList<DecartData>& vector) const;
  int interpolX(const QVector<float> &netfi, obanal::TField *rez,
                             const QVector<float> &netfi_fact, const QVector<float> &rez_vs, int kol_fi, int kol_fi_fact, int kol_la);
  int oprFunSpline1(float *coef, float *fl, float *fun,
                                 float *f, const float *net, int n, int kol);
  
private:
 
  QVector<DecartData> tropoData_T_;
  QVector<DecartData> maxwindData_dd_;
  QVector<DecartData> maxwindData_ff_;
  QMap <int,DecartData> anyData;
  QMap <int,float > miss_p;

  QList<zond::PlaceData> profiles_;
  
  QVector<float> fi_;
  QVector<float> la_;
  QVector<float> rvs_;
  QVector<float> fun_;
  
};

} // meteo
}


#endif // METEO_COMMONS_UI_VERTICALCUT_CORE_VERTICALCUT_H
