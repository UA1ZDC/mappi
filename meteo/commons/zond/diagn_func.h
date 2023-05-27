#ifndef FUNKCIIDIAGNOZA_H
#define FUNKCIIDIAGNOZA_H

#include <qregexp.h>
#include <qmap.h>

#include "turoven.h"

namespace zond {

const double R =  8.31441E+3;//J/(kmol K)   универсальная газовая постоянная
const double Rc = 287.05; //J/(kg K)  Удельная газовая постоянная
const double Rp = 461.5;//J/(kg K)
const double GamTDryAdiab = 0.0098; //K/m  (сухоадиабатический градиент)
const double Lwater = 2.3E+6; //J/kg       (удельная теплота парообразования воды)
const double Cp = 1006.; //J/(kg K)        (удельная теплоемкость при постоянном давлении)
const float  kKelvin = 273.15;
const double kPars = 0.622;   // парциальное давление водяного пара
const double G = 9.8065;
//const double kKap =  0.286;
  const double kKap =  0.2854; //такое значение используется при построении влажных адиабат


  float oprdHpoPT(double p, double p0, double t0, double t1, float gam);

  bool oprTsm(float, float, float, float *);
  double EP(double);
  bool oprTet(float , float , float *);
  bool oprTPoTet(float, float, float *);
  bool oprTPoTPot(float P,float tet, float *res);
  bool oprTPot(float, float, float *);
  bool oprTPoSP(float,float, float *);
  bool oprSPoTP(float,float, float *);
  bool oprPkondens(float, float, float, float *, float *);
  double oprTvirt(double T, double P);

  void sdvigVdf(int d1, int f1,int d2, int f2, int &d, int &f);
  void ISA(float h, float *t, float *p, float h_start );
  void SA81_H( float p, float* t, float* h);
  void SA81_P( float h, float *t, float *p );

  void SA81_H_ext( float p, float* t, float* h);
  void SA81_P_ext( float h, float *t, float *p );
  void p2h(double p, double *h);
  void h2p(double h, double *p);

  void oprPt(float *p, float *t, float h, float gam);

  double getHpoPinPT(double p, double p0, double p1, double t0, double t1, double h0);
  double getHpoPinHT(double p, double h0, double h1, double t0, double t1, double p0);

  double getPpoHinPT(double h, double p0, double p1, double t0, double t1, double h0);
  double getPpoHinHT(double h, double h0, double h1, double t0, double t1, double p0);

  double getGamPoPT(double p0, double p1, double t0, double t1);
  double getGamPoHT(double h0, double h1, double t0, double t1);

  bool getObled(double T, double D);
  bool getNoObledCloud(double T, double D);


  bool getBoltan(double gam_ff, double gam_dd, double gam_t);
  bool getBoltan(double gam_u, double gam_v, double gam_t, double t_sr);


  double getGamTWetAdiab(double p, double t);
  double interpolT(double p0,double p1,double p,double t0,double t1);
  float interpolT(float p0,float p1,float p,float t0,float t1);
  void interpolDDFF(float p0,float p1,float p,float dd0,float ff0,float dd1,float ff1,float *dd,float *ff);

  bool interpolWind(const Uroven & down_ur,const Uroven & up_ur, Uroven * aur);
  bool interpolValue(ValueType val_type, const Uroven & down_ur,const Uroven & up_ur, Uroven * aur);
  //double Y2(double yy);


}

#endif
