#ifndef TZOND_H
#define TZOND_H

#include <meteo/commons/zond/turoven.h>
#include <meteo/commons/zond/clouddata.h>
#include <meteo/commons/zond/diagn_func.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/proto/surface_service.pb.h>

#include <qlist.h>
#include <qvector.h>

#include "urovenlist.h"



namespace zond {

class Zond : public UrovenList {
   public:

    Zond();
    ~Zond();

    void test();


    //!< определение параметров на уровне станции методом поиска по уровням используется в oprHPoP()


     bool oprPkondens(float *, float *);
     bool oprTsost(float , float *);
     bool oprPsostRaznAll(float *, float *, float P_lev_end=100., float P_lev_start=-100., float step = 10.);
     bool oprPsostRaznAllFast(float *p, float *deltaT, float p_beg = -100);

// поиск экстремумов кусочно-линейной функции не целесообразен вне узловых точек

     bool oprHmaxV(float *, float *, float h_min = 0., float h_max = 40000.);
     bool oprIlina(float *, float *, float P_lev_end=10., float P_lev_start=1100., float step=10.);
     bool oprH1(double &, double &, int P_lev_end=10., int P_lev_start=1100., int step=10.);

     
     bool oprHSloymaxV(float *h, float max_v, float H_min, float H_max);
     bool oprNizHpoV(float *h, int *kol_prop,  float max_v, float H_min, float H_max);
     
//   быстрые: по основным уровням и особым точкам (поиск экстремумов целесообразен только по узловым точкам ввиду кусочно-линейной аппроксимации)
     bool oprHmaxVFast(float *, float *);

     bool oprGranKNS(QList<float> *p_kns);
     bool oprSkorDpKNS(QList<float> *, QList<float> *);
     bool oprSrKonvSkor(float, float, float * );

     bool oprGranOblak(QList<float> *p_oblak) const;  //анализ облачных слоев
     bool oprGranObled(QList<float> *p_obled)const;  //анализ обледенения
     bool oprGranBoltan(QList<float> *p_boltan)const;//анализ болтанки
     bool oprGranBoltan_Td(QList<float> *p_boltan, float fi) const;//анализ болтанки c учетом градиентов температуры
     bool oprGranTrace(QList<float> *p_trace)const;  //анализ конденсационных следов

     bool getTmaxTitov(float *t);//прогноз максимальной температуры по методу Титова (используется при расчете КНС)
     bool oprKNS(float *p, float Tmax = BAD_METEO_ELEMENT_VAL);//границы конвективно-неустойчивого слоя
     bool averConvecLevel(float *p, float Tmax = BAD_METEO_ELEMENT_VAL);            //средний уровнеь конвекции
     bool averCondensLevel(float *p, float *t, float Tmax = BAD_METEO_ELEMENT_VAL);//средний уровень конденсации

     bool getAverWindUV(float h1, float h2, float *u, float *v);
     bool getAverWindDF(float h1, float h2, float *dd, float *ff);

     bool getSloiInver(QVector<zond::InvProp> *layers);//Слои инверсии и изотермии
     bool getSrednWinds(QVector<zond::WindAverage> *v );//Средний ветер в слоях

     bool oprSrParamPoPH(ValueType type_par, ValueType type_key, float ph1, float ph2, float *sr_par);

     bool   oprIntLivnOrl(float *);
     int    oprZnakEnergy(double);

     double dp_func(float h, float p= BAD_METEO_ELEMENT_VAL, float t =BAD_METEO_ELEMENT_VAL );
     bool   getGamm(float lvl, float *gam) const;

 // bool oprD();
 // bool oprH();
  
private:
  bool oprNoObledCloud(double H) const; //для поиска методом половинного деления
  double bisectBorderObledCloud( double a, double b, double e, bool* noNull ) const;

  QMap<double, double> gradTxList_; //!< градиент Температуры
  QMap<double, double> gradTyList_; //!< градиент Температуры

};
}

#endif
