#include "diagn_func.h"
#include <commons/mathtools/mnmath.h>

#include <cross-commons/debug/tlog.h>

using namespace MnMath;

namespace zond {


  void sdvigVdf(int d1, int f1,int d2, int f2, int &d, int &f)
  {
    double gtor=3.1415926/180.;
    double rtog=180./3.1415926;
    double dx1,dx2,dy1,dy2,dx,dy;

    dx1=f1*cos((270.-d1)*gtor);
    dy1=f1*sin((270.-d1)*gtor);
    dx2=f2*cos((270.-d2)*gtor);
    dy2=f2*sin((270.-d2)*gtor);
    dx=dx1-dx2;
    dy=dy1-dy2;
    f=ftoi_norm(sqrt(dx*dx+dy*dy));
    d=ftoi_norm(atan2(dy,dx)*rtog);
    d=ftoi_norm(M0To360(270.-d));
  }



  bool oprTsm(float p_nach,float p_kon, float t_nach, float *val)
  {
    if(!oprTPot(p_nach,t_nach,val)) return false;
    if(!oprTPoTPot(p_kon,*val,val)) return false;
    return true;
  }

  bool isvalidTP(float t, float p){
    if(t<-201.f||t>500.f) {
        debug_log<<"invalid T"<<t;
        return false;
      }
    if(p<0.f||p>1100.f){
        debug_log<<"invalid P"<<p;
        return false;
      }
    return true;
  }


  bool oprTPot(float p, float t, float *val)//определение псевдопотенциальной температуры
  {
    if(t<-110.f||t>500.f) return false;
    if(p<100.f||p>1100.f) return false;
    float s=kPars*EP(t)/p;                         //массовая доля водяного пара
    //    cp = 1004.7*(1.0-s)+1846.0*s;          //удельная теплоемкость при постоянном давлении ~1E+3 Дж/(кг*К)
    //    l=(3.14821-0.00237*(t+kKelvin))*1.e6;   //удельная теплота парообразования ~2.5E+6 Дж/кг при t=0
    //    val=t+kKelvin+s*l/cp;                   //псевдоэквивалентная температура (очень грубо)
    s=s/(1.0f-s);                            //отношение смеси
    float kap = 0.2854f*(1.0f - 0.28f*s);             //показатель адиабаты
    //    val=(t+kKelvin)*exp(s*l/(cp*(t+kKelvin)));                   //псевдоэквивалентная температура (грубо)
    (*val) = (t+kKelvin)*exp((3376/(t+kKelvin)-2.54f)*s*(1.0f+0.81f*s)); //псевдоэквивалентная температура
    (*val)*=powf(1000.f/p,kap);                                     //псевдопотенциальная температура
    (*val)-=kKelvin;                                                 //псевдопотенциальная температура
    if(isnan((*val)) ){
        return false;
      }
    return true;
  }

/*
  Sm1=kPars*EP(t-kKelvin)/P;        //массовая доля водяного пара
  Sm1 = Sm1/(1.0-Sm1);             //отношение смеси
  kap = 0.2854*(1.0 - 0.28*Sm1);   //показатель адиабаты
  tet0=t*exp((3376/t-2.54)*Sm1*(1.0 + 0.81*Sm1));        //псевдоэквивалентная температура
  tet0*=pow(1000./P,kap);          //псевдопотенциальная температура
  */



  bool oprTet(float p, float t, float *val)
  {
    if (!isvalidTP(t,p)) return false;

    *val =t+kKelvin;
    (*val) *=pow(1000.f/p,kKap);
    (*val)-=kKelvin;
    return true;
  }
  /**
   * @brief Давление насыщениия (эмпирическая формула)
   * @return давление насыщениия, гПа
   */
  double EP(double t)
  {
    //    return   exp ((10.79574*(1.0-273.16/(t+kKelvin))-
    //		   5.02800*log((t+kKelvin)/273.16)*0.4343+
    //		   0.000150475*(1.0-((t+kKelvin)/273.16-1.0))+
    //		   0.0000001745*(58823.38332*(1.0-273.16/(t+kKelvin))-1.0)+
    //		   0.78614)*log(10.0));

    return        exp ((10.79574*(1.0-273.16/(t+kKelvin))-
                        5.02800*log10((t+kKelvin)/273.16)+
                        0.000150475*(1.0-pow(10., -8.2969*((t+kKelvin)/273.16-1.0)))+
                        0.00042873*(pow(10., 4.76955*(1.0-273.16/(t+kKelvin)))-1.0) +
                        0.78614)*log(10.0));
    
  }

  bool oprSPoTP(float p, float t, float *res)
  {
    if (!isvalidTP(t,p)) return false;
    *res=kPars*EP(t)/p;
    return true;
  }

  bool oprTPoSP(float p,float s, float *res)
  {
    float t,t1,t2,s1,s2,ss;
    if(s<0.f) return false;
    if(p<100.f||p>1100.f) return false;
    t1=-80.;
    t2=50.;

    if(!oprSPoTP(p,t1,&s1)) return false;
    if(!oprSPoTP(p,t2,&s2)) return false;
    ss=s2;
    if((s1-s)*(s2-s)>0.f) return false;
    t=t1;
    //    while(fabs(s-ss)>LITTLE_VALUE)
    float delta = 0.0001f;
    while( false == MnMath::isEqual( s,ss, delta ))
      {
        t=(t1+t2)*0.5f;
        if(!oprSPoTP(p,t,&ss)) return false;
        if((s-ss)*(s1-ss)>0.f) t2=t; else t1=t;
      }
    *res = t;
    return true;
  }

  bool oprTPoTet(float p, float tet, float *res)
  {
    if (!isvalidTP(tet,p)) return false;
    (*res) = (tet+kKelvin)/powf(1000.f/p,kKap)-kKelvin;
    return true;
  }

  bool oprTPoTPot(float P,float tet, float *res)
  {
    float t, Tet,ss;//, Cp,l;
    float t1 = 0.0,t2 = 0.0,tt,tt1,tet0,tet1;
    tt1=0.;
    ss  = tet;
    *res = 0;

    // хз почему так, изменения возвращены из коммита номер fbef2f9516a2b8ba84286866a35a4bc16cef0a51
    // по-другому цикл не начинался
//    t1=-200.+kKelvin;
//    t2=200.+kKelvin;
    t1 = -100.;
    t2 = 100.;

    for(t=t1;t<t2;t++)
      {
        tt=t-1.f;
        ///**************************************************
        if(!oprTPot(P, t,&tet0)) return false;

        ///****************************************************
        if(!oprTPot(P, tt,&tet1)) return false;
        //*****************************************************/
        if((tet0-ss)*(tet1-ss)>0.f) continue;
        Tet=tet0;
        float delta = 0.0001f;
        tt1 = t; //на случай, если в цикл не зайдем
        while( false == MnMath::isEqual(Tet,ss,delta) )
          {
            tt1=(tt+t)*0.5f;
            if(!oprTPot(P, tt1,&Tet)) return false;
            if((tet0-ss)*(Tet-ss)>0.f) {t=tt1; tet0=Tet;} else {tt=tt1;}
          }
     //   *res =tt1-kKelvin; // хз почему так, изменения возвращены из коммита номер fbef2f9516a2b8ba84286866a35a4bc16cef0a51
        *res =tt1; // хз почему так, изменения возвращены из коммита номер fbef2f9516a2b8ba84286866a35a4bc16cef0a51
        break;
      }
    if(t2<=t) return false;
    else return true;
  }

  bool oprPkondens(float t, float td, float p, float *res_p, float *res_t)
  {
    float p_vs,s;
    float tPot,tPot1,tPot2,p1,p2;

    p_vs=0.;
    *res_p=0.;
    *res_t=0.;
    float tet =0,tTet,tTet2,tTet1;
    if(!oprTet(p,t,&tet)) return false;
    if(!oprSPoTP(p,td,&s)) return false;

    p1=1050.f;
    p2=100.f;
    if(!oprTPoTet(p1,tet,&tTet1)) return false;
    if(!oprTPoTet(p2,tet,&tTet2)) return false;
    if(!oprTPoSP(p1,s,&tPot1)) return false;
    if(!oprTPoSP(p2,s,&tPot2)) return false;
    if((tTet1-tPot1)*(tTet2-tPot2)>0.f) return false;
    tTet=tTet1;
    tPot=tPot1;
//  while(fabs(tTet-tPot)>LITTLE_VALUE)
    float delta = 0.0001f;
    while(false == MnMath::isEqual(tTet,tPot,delta))
      {
        p_vs=(p1+p2)*0.5f;
        if(!oprTPoTet(p_vs,tet,&tTet)) return false;
        if(!oprTPoSP(p_vs,s,&tPot)) return false;
        if((tTet1-tPot1)*(tTet-tPot)>0.f) p1=p_vs; else p2=p_vs;
      }
    *res_p = p_vs;
    *res_t = tTet;
    return true;
  }




  /**(надо бы обратную зависимость построить h(P), но нет зависимости gamIsa(P))
   * @brief международная Стандартная (политропная) атмосфера
   *
   * @param h Требуемая высота, м
   * @param t температура на высоте h_start, К
   * @param p давление на высоте h_start, гПа
   * @param h_start высота начала расчетов, м
   **/
  void ISA( float h, float *t, float *p, float h_start )
  {

    if(h>51000){
        h=51000;
      }
    QVector<float> gamIsa,hIsa;
    gamIsa<<0.0065f<<0.<< -0.001f<<-0.0028f<<0.;
    hIsa<<0.<<11600.<<20000.<<32000.<<47000.<<51000.;
    int j=0;

    h = h - h_start;
    for(j=0; j<hIsa.count(); ++j){
        if(h_start < hIsa.at(j) )
          break;
      }

    float h_new = hIsa.at(j)- h_start;
    if(h_new > h){
        h_new = h;
      }
    for(int i=j; i<hIsa.count(); ++i){
        if(h <= h_new){
            float gam = gamIsa.at(i-1);
            oprPt(p, t, h, gam);
            break;
          }else{
            float gam = gamIsa.at(i-1);
            oprPt(p, t, h_new, gam);
            h -= h_new;
            h_new = (hIsa.at(i+1)-hIsa.at(i));
          }
      }

  }


  /**
   * @brief  Стандартная  атмосфера 81
   *
   * @param h Требуемая высота, м
   * @param t температура на высоте , К
   * @param p давление на высоте, гПа
   **/
  void SA81_P( float h, float* t, float* p )
  {
    *p = 1013.25;
    *t = 288.15f;

    if(h > 32000.f){
        h = 32000.;
      }
    if(h < 0.f){
        h = 0.;
      }

    QVector<float> gamSa,hSa,pSa,tSa;
    gamSa<<0.0065f<<0.    << -0.001f<<-0.0028f<<0.;
    hSa  <<0.    <<11000.<<20000.<<32000. <<47000.;
    pSa  <<1013.5<<226.32f<<54.74f <<8.68f <<1.15f;
    tSa  <<288.15f<<216.65f<<216.65f <<228.65f <<269.68f;
    int j = 0;
    float gam = 0.;
    float h0 = 0.;
    float t0 = 0.;
    float p0 = 0.;

    for(j = 0; j < hSa.count()-1; ++j){
        if( (h >= hSa.at(j)) && (h < hSa.at(j+1)) ){
            gam = gamSa.at(j);
            h0 = hSa.at(j);
            t0 = tSa.at(j);
            p0 = pSa.at(j);
            break;
          }
      }
    oprPt(&p0, &t0, h-h0, gam);
    *p = p0;
    *t = t0;
  }

  /**
   * @brief  Стандартная  атмосфера 81
   *
   * @param p давление, гПа
   * @param t температура на высоте , К
   * @param h давление на высоте, гПа
   **/
  void SA81_H( float p, float *t, float *h )
  {

    *h = 0.;

    float p0 = 1013.5;
    float t0 = 288.15f;
    if(p > 1013.5f){
        p = 1013.5;
      }
    if(p < 10.f){
        p = 10.;
      }

    QVector<float> gamSa,hSa,pSa,tSa;
    gamSa<<0.0065f<<0.    << -0.001f<<-0.0028f<<0.;
    hSa  <<0.    <<11000.<<20000.<<32000. <<47000.;
    pSa  <<1013.5<<226.32f<<54.74f <<8.68f <<1.15f;
    tSa  <<288.15f<<216.65f<<216.65f <<228.65f <<269.68f;
    int j = 0;
    float gam = 0.;
    float h0 = 0.;
    float t1 = 0.;

    for(j = 0; j < hSa.count()-1; ++j){
        if( (p < pSa.at(j)) && (p >= pSa.at(j+1)) ){
            gam = gamSa.at(j);
            h0 = hSa.at(j);
            p0 = pSa.at(j);
            t0 = tSa.at(j);
            t1 = tSa.at(j+1);
            break;
          }
      }


    float t_sr = Rc*((t1 + t0)*0.5f)/G;
    //debug_log << "tvs" <<tvs;

    if(MnMath::isZero(gam)){ //если gam=0 почти изотремия
        *h = - t_sr*logf(p / p0);
      } else {
        //double vs = -g /(gam*R); // почему с минусом?
        //vs = exp(log( p_ / p_0)/vs)*(t_0 + tK); //exp(logX) =X
        //h=-(t_0 + tK - vs)/gam;  // вот почему минус! (тут надо убарть и вверху)
        *h = (t0 )*(1. - powf(p / p0, gam * Rc / G)) / gam; //TODO может проще надо заменить на эту формулу
      }
    *h = h0 + *h;
    oprPt(&p0, &t0, *h, gam);
    *t = t0;
  }

  /**
   * @brief  Стандартная  атмосфера 81, расширенная до 1200 гПа
   * @brief  Используется в качестве функции трансформации для аэрологической диаграммы
   *
   * @param h Требуемая высота, м
   * @param t температура на высоте , К
   * @param p давление на высоте, гПа
   **/
  void SA81_P_ext( float h, float* t, float* p )
  {
    *p = 1200.f;
    *t = 297.575457283f;

    if(h > 32000.f){
        h = 32000.;
      }
    if(h < 0.f){
        h = 0.;
      }

    QVector<float> gamSa,hSa,pSa,tSa;
    gamSa<<0.0065f<<0.    << -0.001f<<-0.0028f<<0.;
    hSa  <<0.<<12450.07035121f<<21450.07035121f<<32000. <<47000.;
    pSa  <<1200.<<226.32f<<54.74f <<8.68f <<1.15f;
    tSa  <<297.575457283f<<216.65f<<216.65f <<228.65f <<269.68f;
    int j = 0;
    float gam = 0.;
    float h0 = 0.;
    float t0 = 0.;
    float p0 = 0.;

    for(j = 0; j < hSa.count()-1; ++j){
        if( (h >= hSa.at(j)) && (h < hSa.at(j+1)) ){
            gam = gamSa.at(j);
            h0 = hSa.at(j);
            t0 = tSa.at(j);
            p0 = pSa.at(j);
            break;
          }
      }
    oprPt(&p0, &t0, h-h0, gam);
    *p = p0;
    *t = t0;
  }

  /**
   * @brief  Стандартная  атмосфера 81, расширенная до 1200 гПа
   * @brief  Используется в качестве функции трансформации для аэрологической диаграммы
   *
   * @param p давление, гПа
   * @param t температура на высоте , К
   * @param h давление на высоте, гПа
   **/
  void SA81_H_ext( float p, float *t, float *h )
  {

    *h = 0.;

    float p0 = 1200.;
    float t0 = 297.57f;
    if(p > 1200.f){
        p = 1200.;
      }
    if(p < 10.f){
        p = 10.;
      }

    QVector<float> gamSa,hSa,pSa,tSa;
    gamSa<<0.0065f<<0.    << -0.001f<<-0.0028f<<0.;
    hSa  <<0.<<12450.07035121f<<21450.07035121f<<32000. <<47000.;
    pSa  <<1200.<<226.32f<<54.74f <<8.68f <<1.15f;
    tSa  <<297.575457283f<<216.65f<<216.65f <<228.65f <<269.68f;
    int j = 0;
    float gam = 0.;
    float h0 = 0.;
    float t1 = 0.;

    for(j = 0; j < hSa.count()-1; ++j){
        if( (p < pSa.at(j)) && (p >= pSa.at(j+1)) ){
            gam = gamSa.at(j);
            h0 = hSa.at(j);
            p0 = pSa.at(j);
            t0 = tSa.at(j);
            t1 = tSa.at(j+1);
            break;
          }
      }

    float t_sr = Rc*((t1 + t0)*0.5f)/G;
    //debug_log << "tvs" <<tvs;

    if(MnMath::isZero(gam)){ //если gam=0 почти изотремия
        *h = - t_sr*logf(p / p0);
      } else {
        //double vs = -g /(gam*R); // почему с минусом?
        //vs = exp(log( p_ / p_0)/vs)*(t_0 + tK); //exp(logX) =X
        //h=-(t_0 + tK - vs)/gam;  // вот почему минус! (тут надо убарть и вверху)
        *h = (t0 )*(1. - pow(p / p0, gam * Rc /G)) / gam; //TODO может проще надо заменить на эту формулу
      }
    *h = h0 + *h;
    oprPt(&p0, &t0, *h, gam);
    *t = t0;
  }


  /**
   * @brief определение давления и температуры на высоте h (толщина слоя, если p и t не у пов Земли)
   *
   * @param p давление, гПа
   * @param t температура, К
   * @param h высота (толщина слоя, если p и t не у пов Земли)
   * @param gam вертикальный градиент температуры
   **/
  void oprPt(float *p, float *t, float h, float gam){
    if(!MnMath::isZero(gam)){
        *p = (*p)*powf((1.f - gam*h/(*t)),G/Rc/gam);
        *t = *t - gam*h;
      } else {
        *p = (*p)*expf( -h*G/Rc/(*t));
      }
  }

  float oprdHpoPT(double p, double p0, double t0, double t1, float gam){

    double t_sr = Rc*(kKelvin+(t1 + t0)/2.)/G;
    //debug_log << "tvs" <<tvs;
    float h = 0.;
    if(MnMath::isZero(gam)){ //если gam=0 почти изотремия
        h = - t_sr*log(p / p0);
      } else {
        h = (t0 + kKelvin)*(1. - pow(p / p0, gam * Rc / G)) / gam; //TODO может проще надо заменить на эту формулу
      }
  return h;
}
  /**
   * @brief определение высоты по давлению в точке между соседними уровнями и давлению на уровнях
   * @param p давление в искомой точке, гПа
   * @param p0 давление на нижнем (верхнем) уровне, гПа
   * @param p1 давление на верхнем (нижнем) уровне, гПа
   * @param t0 температура на нижнем (верхнем) уровне, С
   * @param t1 температура на верехнем (нижнем) уровне, С
   * @param h0 высота нижнего (верхнего) опорного уровня, м
   **/


  double getHpoPinPT(double p, double p0, double p1, double t0, double t1, double h0)
  {
    double gam = (p0 < p1)? getGamPoPT(p1,p0,t1,t0): //движение относительно h0 вниз (p0 < p1)
                            getGamPoPT(p0,p1,t0,t1); // на основе политропной зависимости P(H)
    return h0 + oprdHpoPT(p,p0,t0,t1,gam);
  }

  /**
   * @brief определение высоты по давлению в точке между соседними уровнями и высоте уровней
   * @param p давление в искомой точке, гПа
   * @param h0 высота нижнего (верхнего) уровня, гПа
   * @param p1 высота верхнего (нижнего) уровня, гПа
   * @param t0 температура на нижнем (верхнем) уровне, С
   * @param t1 температура на верехнем (нижнем) уровне, С
   * @param p0 давление на нижнем (верхнем) опорном уровне, м
   **/
  ///FIXME проверить функцию!!!

  double getHpoPinHT(double p, double h0, double h1, double t0, double t1, double p0)
  {
    float gam = (h0 > h1)? getGamPoHT(h1,h0,t1,t0): //движение относительно h0 вниз (h0 > h1)
                            getGamPoHT(h0,h1,t0,t1); // на основе политропной зависимости P(H)
    return h0 + oprdHpoPT(p,p0,t0,t1,gam);
  }




  /**
   * @brief определение давления по высоте точки и давлению на соседних уровнях
   * @param p давление в искомой точке, гПа
   * @param p0 давление на нижнем (верхнем) уровне, гПа
   * @param p1 давление на верхнем (нижнем) уровне, гПа
   * @param t0 температура на нижнем (верхнем) уровне, С
   * @param t1 температура на верехнем (нижнем) уровне, С
   * @param h0 высота нижнего (верхнего) опорного уровня, м
   **/

  double getPpoHinPT(double h, double p0, double p1, double t0, double t1, double h0)
  {

    double p_ = 0.; //искомая неизвестная
    double gam_ = (p0 < p1)? getGamPoPT(p1,p0,t1,t0): //движение относительно h0 вниз (p0 < p1)
                             getGamPoPT(p0,p1,t0,t1); // на основе политропной зависимости P(H)
    double t_ = t0 - gam_*(h - h0);//проинтерполированное значение T

    double x,y;
    if(qFuzzyCompare(1. + gam_, 1.))  // если gam=0 почти изотермия
      {
        x = (-G*(h - h0))/(Rc*(t0 + kKelvin));
        p_ = MnMath::ftoi_norm(p0*exp(x));
      } else {
        x=(t_+ kKelvin)/(t0+ kKelvin); // t_0 - gam*(h-h_0)=t_
        y=G/(Rc*gam_); // TODO должно быть так
        p_ = MnMath::ftoi_norm(p0*pow(x,y));
      }
    return p_;
  }

  /**
   * @brief определение давления по высоте точки и высоте соседних уровней
   * @param p давление в искомой точке, гПа
   * @param h0 высота нижнего (верхнего) уровня, гПа
   * @param p1 высота верхнего (нижнего) уровня, гПа
   * @param t0 температура на нижнем (верхнем) уровне, С
   * @param t1 температура на верехнем (нижнем) уровне, С
   * @param p0 давление на нижнем (верхнем) опорном уровне, м
   **/


  double getPpoHinHT(double h, double h0, double h1, double t0, double t1, double p0)
  {

    double p_ = 0.; //искомая неизвестная
    double gam_ = getGamPoHT(h0, h1, t0, t1);//TODO надо ли уситывать условие h0 > h1
    double t_ = t0 - gam_*(h - h0);//проинтерполированное значение T
    double x,y;
    if(MnMath::isZero(gam_))  // если gam=0 почти изотермия
      {
        x = (-G*(h - h0))/(Rc*(t0 + kKelvin));
        p_ = MnMath::ftoi_norm(p0*exp(x));
      } else
      //    if(gam<0.) // TODO почему >0 должно быть <>0 протестить!!
      {
        x=(t_+ kKelvin)/(t0 + kKelvin); // t_0 - gam*(h-h_0)=t_
        y=G/(Rc*gam_); // TODO должно быть так
        p_ = MnMath::ftoi_norm(p0*pow(x,y));
      }
    return p_;
  }



  /**
   * @brief определение вертикального градиента температуры по температуре и давлению между соседними уровнями
   * @param p0 давление на нижнем уровне, гПа
   * @param p1 давление на верхнем уровне, гПа
   * @param t0 температура на нижнем уровне, С
   * @param t1 температура н верехнем уровне, С
   **/

  double getGamPoPT(double p0, double p1, double t0, double t1)
  {
    double g = G, R = Rc, tK = kKelvin;

    return g*log((t1+tK)/(t0+tK))/(R*log(p1/p0)); // на основе политропной зависимости P(H)
  }

  double getGamPoHT(double h0, double h1, double t0, double t1)
  {
    return MnMath::isZero(h1-h0)? 0 : -(t1 - t0)/(h1 - h0);
  }


  bool getObled(double T, double D)
  {
    return T < -8.*D;
  }

  bool getNoObledCloud(double T, double D)
  {
    if(T >= 0.)   return true;
    if(   0. > T && T >= -7.5 && D > 2.)    return true;
    if(-7.5  > T && T >= -15.5  && D > 3.) return true;
    if(-15.5 > T && T >= -25.0 && D > 4.) return true;
    if(-25.5 > T) return true;
    return false;  //нет условий для отсутствия обледенения
  }

  /**
   * @brief определение наличия болтанки в слое с заданными вертикальными градиентами по скорости и направлению ветра и температуре
   * @param gam_ff - градиент скорости ветра, м/с/м = 1/с
   * @param gam_dd - градиент направления ветра, град/м
   * @param gam_t - градиент температуры, град/м
   * @return факт наличия болтанки
   **/
  bool getBoltan(double gam_ff, double gam_dd, double gam_t)
  { //критические значения для образования болтанки
    return gam_ff >= 10./1000. || gam_dd >= 15./1000. || gam_t >= 7./1000.;
  }

  /**
   * @brief определение наличия болтанки в слое с заданными вертикальными градиентами по скорости и направлению ветра и температуре
   * @param gam_u - градиент скорости ветра, м/с/м = 1/с
   * @param gam_v - градиент скорости ветра, м/с/м = 1/с
   * @param gam_t - градиент температуры, град/м
   * @param t_sr - средняя температура слоя, град. C
   * @return факт наличия болтанки
   **/
  bool getBoltan(double gam_u, double gam_v, double gam_t, double t_sr)
  {

//  double div = (gam_u*gam_u+gam_v*gam_v == 0. ) ? LITTLE_VALUE : ;
    double div = gam_u*gam_u + gam_v*gam_v;
    if( false == MnMath::isZero(div) ){
        div = (t_sr+kKelvin)*(gam_u*gam_u+gam_v*gam_v);
      }
   double Ri = 9.8*(GamTDryAdiab - gam_t)/div; ;
    //qDebug() << "GamT=" << gam_t << "t_sr=" << t_sr << "div=" << div << "Ri=" << Ri;
   return !(Ri > 1.) ;
  }

  /**
   * @brief Влажноадиабатический градиент температуры
   * @param p атмосферное давление, гПа
   * @param t температура частицы воздуха, C
   * @return значение влажноадиабатического градиента С/м
   **/
  double getGamTWetAdiab(double p, double t)
  {
    double E = EP(t);
    double a = p + kPars*Lwater*E/R/(t + kKelvin);
    double b = p + kPars*Lwater*Lwater*E/Cp/Rp/(t + kKelvin)/(t + kKelvin);
    return GamTDryAdiab*a/b;
  }

  //! @param T температура, C
  //! @param P атмосферное давление, гПа
  //! @return значение виртуальной температуры, C
  double oprTvirt(double T, double P)
  {
    T += kKelvin;
    P *= 100;
    T = T / (1 - (kPars/P)*(1 - kPars));
    return T - kKelvin;
  }


  double interpolT(double p0,double p1,double p,double t0,double t1){
    double e =  log(p / p0) / log(p1 / p0);
    return t0 + (t1-t0)*e;
  }

  float interpolT(float p0,float p1,float p,float t0,float t1){
    float e =  logf(p / p0) / logf(p1 / p0);
    return t0 + (t1-t0)*e;
  }

  void interpolDDFF(float p0, float p1, float p, float dd0, float ff0, float dd1, float ff1, float *dd, float *ff){
    dd0=deg2rad(dd0);
    dd1=deg2rad(dd1);
    float e =  logf(p / p0) / logf(p1 / p0);
    
    float k6 = -ff0*cosf(dd0);
    float k8 = -ff1*cosf(dd1);
    float j6 = -ff0*sinf(dd0);
    float j8 = -ff1*sinf(dd1);
    float k7 = k6+(k8-k6)*e;
    float j7 = j6+(j8-j6)*e;
    *dd = rad2deg(atan2f(-k7,-j7));
    if(*dd < 0) *dd += 360;
    *ff = sqrtf(j7*j7+k7*k7);
  }

  //исходный ветер в UV

  /*
  double Y2(double yy)
  {
   return (104.4722*(7.3124-pow(yy, kKap)));
  }
*/

  bool interpolWind(const Uroven & down_ur,const Uroven & up_ur, Uroven * aur)
  {
    float gam;
    if(nullptr == aur) return false;
    if( down_ur.isGood(UR_H) && up_ur.isGood(UR_H) && down_ur.isGood(UR_u) && up_ur.isGood(UR_u) &&  aur->isGood(UR_H)
        && down_ur.isGood(UR_v) && up_ur.isGood(UR_v) )
      {
        float H_ = aur->value(UR_H);
        float H_1 = up_ur.value(UR_H);
        float u1 = up_ur.value(UR_u);
        float v1 =   up_ur.value(UR_v);
        float H_0 = down_ur.value(UR_H);
        float u0 = down_ur.value(UR_u);
        float v0 =   down_ur.value(UR_v);
        gam = (u1 - u0)/(H_1 - H_0);
        float u = MnMath::ftoi_norm(u0 + gam*(H_ - H_0));
        gam = (v1 - v0)/(H_1 - H_0);
        float v   = MnMath::ftoi_norm(v0 + gam*(H_ - H_0));
        aur->set(UR_u,u,control::ABSENT_CORRECTED);
        aur->set(UR_v,v,control::ABSENT_CORRECTED);
        aur->to_ddff();
        return true;
      }
    return false;
  }

  bool interpolValue(ValueType val_type, const Uroven & down_ur,const Uroven & up_ur, Uroven * aur)
  {
    float gam;
    if(nullptr == aur) return false;
    if( down_ur.isGood(UR_H) && up_ur.isGood(UR_H) && down_ur.isGood(val_type) && up_ur.isGood(val_type) &&  aur->isGood(UR_H)){
        gam =  (up_ur.value(val_type) - down_ur.value(val_type))/(up_ur.value(UR_H) - down_ur.value(UR_H));
        float val_ = down_ur.value(val_type) + gam*(aur->value(UR_H) - down_ur.value(UR_H));
        aur->set(val_type, val_,control::ABSENT_CORRECTED );
        return true;
      }
    return false;
  }



}









