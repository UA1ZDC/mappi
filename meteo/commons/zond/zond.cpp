#include "zond.h"
#include "diagn_func.h"

//#include <commons/meteo_data/tmeteodescr.h>

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/surface_service.pb.h>

//#include <map>
#include <qdebug.h>

namespace zond{


Zond::Zond()
{
  keyType_ = LAST_UR;
  isData_ = false;
}

Zond::~Zond() {
  clear();
}

void Zond::test()
{
  preobr(true);
}

bool Zond::oprPkondens(float *p_kondens, float *t_kondens)
{
  Uroven ur;
  if(!getPprizInvHi(&ur)){
    if(!getUrz( &ur)){
        warning_log << QObject::tr("Нет данных о давлениии у поверхности за %1 (%2) ")
                              .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"))
                              .arg(stIndex_);
      return false;
    }
  }
  if(!ur.isGood(UR_T) ||!ur.isGood(UR_Td) ||!ur.isGood(UR_P) ){
    return false;
  }
  return zond::oprPkondens(ur.value(UR_T) , ur.value(UR_Td) ,ur.value(UR_P),  p_kondens, t_kondens);

}

bool Zond::oprTsost(float P_lev, float *val)
{
  float p_kondens, tet, tpot, t_kondens;
  Uroven ur;
  if(!getPprizInvHi(&ur)){
    if(!getUrz( &ur)){
      warning_log << QObject::tr("Нет данных о давлениии у поверхности за %1 (%2) ")
                            .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"))
                            .arg(stIndex_);
      return false;
    }

  }
  if(!ur.isGood(UR_T) ||!ur.isGood(UR_Td) ||!ur.isGood(UR_P) ){
    return false;
  }

  if(P_lev > ur.value(UR_P)) return false;
  if(!zond::oprPkondens(ur.value(UR_T) , ur.value(UR_Td) ,ur.value(UR_P),  &p_kondens, &t_kondens)) return false;

  if(!oprTet(ur.value(UR_P), ur.value(UR_T), &tet)) return false;

  if(!oprTPot(p_kondens, t_kondens, &tpot)) return false;

  return p_kondens < P_lev ? oprTPoTet(P_lev, tet, val) : oprTPoTPot(P_lev, tpot, val);
}

//!< максимальная разность между кривой состояния и кривой стратификации
bool Zond::oprPsostRaznAll(float *p, float *deltaT, float P_lev_end, float P_lev_start, float step)
{
  int P_lev;
  int kol = 0;
  Uroven ur;
  float p_kondens, t_kondens, t_vsSost, max_razn = BAD_METEO_ELEMENT_VAL,P_max_razn = 0.;

  //если начальный уровень давления указан отрицательный, то начало - уровень конденсации
  if(P_lev_start < 0){
    if(!oprPkondens(&p_kondens,&t_kondens)) return false;
    P_lev_start = p_kondens;
  }
  //поиск максимальной разности на всех уровнях с заданным шагом
  for(P_lev = P_lev_start; P_lev > P_lev_end; P_lev -= step)
  {
    if(!oprUrHTpoP(P_lev,&ur)) continue;
    if(!ur.isGood(UR_T)) continue;
    if(!oprTsost(P_lev,&t_vsSost)) continue;
    if( t_vsSost - ur.value(UR_T) > max_razn ) {
      P_max_razn = P_lev;
      max_razn = t_vsSost - ur.value(UR_T);
    }
    kol++;
  }
  if(kol == 0) return false;
  *p = P_max_razn;
  *deltaT = max_razn;
  return true;
}
//!< максимальная разность между кривой состояния и кривой стратификации
bool Zond::oprPsostRaznAllFast(float *p, float *deltaT, float p_beg)
{
  if(keyType_ != UR_P && keyType_ != UR_H) return false;

  Uroven ur;
  float t_kondens, t_vsSost;
  float max_razn = BAD_METEO_ELEMENT_VAL, P_max_razn = 0.;
  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator eit;
  //начальный уровень - уровень конденсации
  if(p_beg < 0.f ){
    if(!oprPkondens(&p_beg,&t_kondens)) return false;
  }
  if(!oprTsost(p_beg,&t_vsSost) || !oprUrHTpoP(p_beg,&ur)) return false;
  if(ur.isGood(UR_T)) {
    P_max_razn = p_beg;
    max_razn = t_vsSost - ur.value(UR_T);
  }

  //поиск следующего уровня из числа имеющихся точек
  if(keyType_ == UR_H) { //по возрастанию ключа H
    for(it = urList_.begin(); it != urList_.end(); ++it){
      if(it.value().isGood(UR_P) && it.value().value(UR_P) <= p_beg) break;
    }
    eit = urList_.end();
  }

  if(keyType_ == UR_P){
    it = urList_.lowerBound(p_beg); //итератор на следующее за p_kondens значение по убыванию ключа P (P <= p_beg)
    eit = urList_.begin();
  }

  //поиск по всем остальным уровням
  while( it != eit ){
    if(keyType_ == UR_P)  --it;
    if(!it.value().isGood(UR_P)) continue;
    if(!oprUrHTpoP(it.value().value(UR_P),&ur)) continue;
    if(!oprTsost(it.value().value(UR_P),&t_vsSost)) continue;
    if((t_vsSost - ur.value(UR_T)) > max_razn) {
      P_max_razn = ur.value(UR_P);
      max_razn = t_vsSost - ur.value(UR_T);
    }
    if(keyType_ == UR_H)  ++it;
  }
  if(MnMath::isEqual(BAD_METEO_ELEMENT_VAL, max_razn)) return false;
  *p = P_max_razn;
  *deltaT = max_razn;
  return true;
}


//!< v - максимальная скорость ветра, в м/с
//!< h - высота, в м
bool Zond::oprHmaxV( float *h, float *v, float H_min, float H_max)
{
  float max_v = BAD_METEO_ELEMENT_VAL, h_max_v = 0.;
  QMap <Level, Uroven>::iterator it;
  for(it = urList_.end(); it != urList_.begin(); --it){
    zond::Uroven& ur = it.value();
    if(!ur.isGood(UR_H) || !ur.isGood(UR_dd) || !ur.isGood(UR_ff)) continue;
    if(ur.value(UR_H) > H_max || ur.value(UR_H) < H_min )continue;
    if(ur.value(UR_ff) >= max_v) {
      max_v = ur.value(UR_ff);
      h_max_v = ur.value(UR_H);
  //    debug_log<<max_v<<h_max_v;
    }
  }
  if(MnMath::isEqual(max_v,BAD_METEO_ELEMENT_VAL)) return false;
  *h = h_max_v;
  *v = max_v;
  return true;
}
//!< v - скорость ветра, в м/с
//!< h - нижняя граница слоя в котором скорость ветра выше V, в м
bool Zond::oprNizHpoV( float *h, int *kol_prop, float max_v, float H_min, float H_max)//TODO  поправить нахождение уровня скорости
{
  float h_max_v = BAD_METEO_ELEMENT_VAL;
  *kol_prop = 0;
  QMap <Level, Uroven>::iterator it;
  for(it = urList_.end(); it != urList_.begin(); --it){
    zond::Uroven& ur = it.value();
    if(!ur.isGood(UR_H) || !ur.isGood(UR_dd) || !ur.isGood(UR_ff)) continue;
    if(ur.value(UR_H) > H_max || ur.value(UR_H) < H_min )continue;
    if(ur.value(UR_ff) >= max_v) {
      //max_v = ur.value(UR_ff);
      h_max_v = ur.value(UR_H);
      break;
    } else {
      ++(*kol_prop);
    }
  }
  if(MnMath::isEqual( h_max_v,BAD_METEO_ELEMENT_VAL)) return false;
  *h = h_max_v;
  return true;
}



//!< v - максимальная скорость ветра, в м/с
//!< h - толщина слоя в котором скорость ветра выше V, в м
bool Zond::oprHSloymaxV( float *h, float max_v, float H_min, float H_max)
{
  float h_max_v = BAD_METEO_ELEMENT_VAL;
  float h_start = BAD_METEO_ELEMENT_VAL;
  float h_sloy = 0;
  int kol_prop = 0;
  if(!oprNizHpoV(&h_start, &kol_prop,max_v, H_min, H_max)){
    return false;
  }
  H_min = h_start+1.f;
  do{
    if(!oprNizHpoV(&h_max_v, &kol_prop, max_v, H_min, H_max)){
      break;
    }
    if(0 == kol_prop){
      h_sloy += h_max_v-h_start;
    }
    h_start = h_max_v;
    H_min = h_max_v+1.f;

    //debug_log <<h_max_v<<h_start<<h_sloy;

  }while(true);
  *h = h_sloy;
  return true;
}


//!< v - максимальная скорость ветра, в км/ч
//!< h - высота, в км
bool Zond::oprHmaxVFast( float *h, float *v)
{
  float max_v = BAD_METEO_ELEMENT_VAL, h_max_v = 0.;
  QMap <Level, Uroven>::iterator it;
  for(it = urList_.begin(); it != urList_.end(); ++it){
    if(!it.value().isGood(UR_H) || !it.value().isGood(UR_dd) || !it.value().isGood(UR_ff)) continue;
    if(it.value().value(UR_ff) >= max_v) {
      max_v = it.value().value(UR_ff);
      h_max_v = it.value().value(UR_H);
    }
  }
  if(MnMath::isEqual( max_v, BAD_METEO_ELEMENT_VAL)) return false;
  *h = h_max_v/1000.f;
  *v = max_v*3.6f;
  return true;
}


//!< v - максимальная скорость ветра, в м/с
//!< p - давление на уровне, гПа
bool Zond::oprIlina( float *p, float *v, float P_lev_end, float P_lev_start, float step)
{
  float P_lev;
  int kol = 0;
  Uroven ur;
  float max_v = BAD_METEO_ELEMENT_VAL, p_max_v = 0.;

  for(P_lev = P_lev_start; P_lev > P_lev_end; P_lev -= step)
  {
    if(!getUrPoP(P_lev,&ur)) continue;
    if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)) continue;
    if(ur.value(UR_ff) > max_v) {
      max_v = ur.value(UR_ff);
      p_max_v = ur.value(UR_P);
    }
    kol++;
  }
  if(kol == 0) return false;
  *p = p_max_v;
  *v = max_v;
  return true;
}


//!< h1 - высота верхней границы слоя, где скорость ветра V = 0.8 Vmax, в км
//!< h2 - высота нижней границы слоя, где скорость ветра V = 0.8 Vmax, в км
bool Zond::oprH1(double &h1, double &h2, int P_lev_end, int P_lev_start, int step)
{
  if(!isData_) return false;
  int P_lev;
  int P_levl = 0;
  int kol = 0;
  double v; //, h;
  Uroven ur;
  double max_v = BAD_METEO_ELEMENT_VAL; //, h_max_v = 0;
  double V_V,V_N,H_V,H_N;
  //поиск уровня с максимальным ветром от земли
  for(P_lev = P_lev_start; P_lev > P_lev_end; P_lev -= step)
  {
    if(!getUrPoP(P_lev, &ur)) continue;
    if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)) continue;
    if(ur.value(UR_ff) > max_v) {
      max_v = ur.value(UR_ff);
      //        h_max_v = ur.value(UR_H);
      P_levl = P_lev;
    }
    kol++;
  }
  if(kol==0) return false;

  //h = h_max_v;
  v = max_v;

  //поиск нижней границы слоя с V=0.8Vmax (оси струйного течения) от уровня с максимальным ветром
  //поиск нижнего ближайшего уровня к 0.8Vmax
  for(P_lev = P_levl; P_lev < P_lev_start; P_lev += step)
  {
    if(!getUrPoP(P_lev, &ur)) continue;
    if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)) continue;
    if(ur.value(UR_ff) < v*0.8f) {
      break;
    }
  }
  if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)) return false;

  H_N = ur.value(UR_H)/1000.f;
  V_N = ur.value(UR_ff);

  //поиск вернего ближайшего уровня к 0.8Vmax и интерполяция
  if(getUrPoP(P_lev - step, &ur)) {
    if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)){
      h1 = H_N;
    }
    else {
      H_V = ur.value(UR_H)/1000.;
      V_V = ur.value(UR_ff);
      h1 = H_N+((0.8*v-V_N)/(V_V-V_N))*(H_V-H_N);
    }
  }

  //поиск верхней границы слоя с V=0.8Vmax (оси струйного течения) от уровня с максимальным ветром
  //поиск верхнего ближайшего уровня к 0.8Vmax
  for(P_lev = P_levl; P_lev > P_lev_end; P_lev -= step)
  {
    if(!getUrPoP(P_lev, &ur)) continue;
    if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)) continue;
    if(ur.value(UR_ff) < v*0.8) {
      break;
    }
  }
  if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)) return false;

  H_V = ur.value(UR_H)/1000.;
  V_V = ur.value(UR_ff);

  //поиск нижнего ближайшего уровня к 0.8Vmax и интерполяция
  if(getUrPoP(P_lev + step, &ur)) {
    if(!ur.isGood(UR_dd) || !ur.isGood(UR_ff)) {
      h2 = H_V;
    }
    else {
      H_N = ur.value(UR_H)/1000.;
      V_N = ur.value(UR_ff);
      h2 = H_N + ((0.8*v - V_N)/(V_V - V_N))*(H_V - H_N);
    }
  }

  return true;
}


// Определение границ КНС
// Входные данные : список уровней urList_
// Выходные данные : список границ уровней КНС p_kns
bool  Zond::oprGranKNS(QList<float> *p_kns)
{
  float tmax = BAD_METEO_ELEMENT_VAL;
  if( false == getTmaxTitov(&tmax) ) {
    tmax = BAD_METEO_ELEMENT_VAL;
  }
  float p = BAD_METEO_ELEMENT_VAL;
  if ( false == oprKNS( &p, tmax ) ) {
    return false;
  }
  zond::Uroven ur;
  if ( false == getUrPoP( p, &ur ) ) {
    return false;
  }

  zond::Uroven urZ;
  if ( false == getUrz(&urZ) ) {
    return false;
  }
  float pZ = urZ.value(zond::UR_P);

  if (MnMath::isEqual(p, pZ) ) {
    return false;
  }
  p_kns->append(pZ);
  p_kns->append(p);
  return true;
}

bool  Zond::oprSkorDpKNS(QList<float> *w, QList<float> *dp)
{
  if(!isData_) return false;

  Uroven ur;

  QList<float> p_kns;
  QList<float> t_vs;
  QList<float> p_vs;

  float p_step, t_sr;
  float P, T, deltaT, deltaP;
  QList<float>::iterator it;
  QList<float>::iterator prev;
  int j, kol_vs;

  kol_vs = 100; //количество элементарных слоев в КНС для интегрирования

  if(!oprGranKNS(&p_kns) ) return false;



  it = p_kns.begin();
  while( it != p_kns.end() ) {
    prev = it; ++it;
    p_step = (*it - *prev)/kol_vs;//разделение каждого КНС на 100 частей определение шага по давлению
    if(MnMath::isZero(p_step)) return false;
    for(j = 0; j < kol_vs; j++) {
      P = *prev + p_step*j;   //определение давления на каждой из частей КНС
      p_vs.append(P);
      if(!oprTsost(P, &T)) return false; //определение температуры кривой состояния на каждой из частей КНС
      if(!oprUrHTpoP(P, &ur)) return false;//определение температуры на каждой из частей КНС
      if(!ur.isGood(UR_T)) return false;
      deltaT = T - ur.value(UR_T);      //определения разности кривой состояния и стратификации
      t_vs.append(deltaT);
    }
    if(!MnMath::integrTrapec(t_vs,p_vs,&t_sr)) return false;
    deltaP = (*it - *prev);
    t_sr /= deltaP; //среднее значение разности кривой состояния и стратификации в пределах КНС
    float W_vs = sqrtf(Rc*(t_sr+kKelvin)*log((*prev)/(*it)));
    w->append(W_vs);
    dp->append(-deltaP);
    ++it;
  }

  return true;
}

bool Zond::oprSrKonvSkor( float p1, float p2, float *w_konv)
{
  if(!isData_) return false;
  int i;
  float m;
  QList<float> w;
  QList<float> dp;


  if(p1 < p2) return false;
  *w_konv = 0.;
  if(!oprSkorDpKNS(&w, &dp)) return false;
  m = 0.;
  for(i = 0; i < dp.count(); i++) {
    m += dp[i];
  }
  if(m < 100.f) return true;

  for(i = 0;i < w.count(); i++) {
    *w_konv += (w[i]*dp[i]);
  }
  *w_konv = (*w_konv)*0.3f/(p1 - p2);

  return true;
}

bool Zond::oprGranOblak(QList<float> *p_oblak) const//Анализ облачности Ns, As, Cs
{
  if(!isData_) return false;

  Uroven ur;
  float deltaDup, deltaDdown = 0.0, Hup, Hdown = 0.0; //дефицит на границах анализируемых элементарных слоев
  float Dkrit, t, h_vs;

  CloudDkr Dkr;
  if(!getCloudDkrH(&Dkr)) return false; //определение высот уровней P с критическими значениями дефицитов для облачности



  p_oblak->clear();

  QMap <Level, Uroven>::const_iterator it = urList_.end()-1;
  QMap <Level, Uroven>::const_iterator eit = urList_.begin();
  QMap <Level, Uroven>::const_iterator prev;

  //!< поиск первой точки с корректными данными и определение разности дефицитов на нижней границе элементарного слоя
  for(;it != eit; --it) {
    if(!it.value().isGood(UR_P) || !it.value().isGood(UR_H) || !it.value().isGood(UR_D)) continue;
    Hdown = it.value().value(UR_H);
    if(!Dkr.getCloudDeficit(Hdown, &Dkrit)) continue;
    deltaDdown = Dkrit - it.value().value(UR_D);
    ++it; break;
  }
  //!< поиск последующих точек с корректными данными и определение разности дефицитов на верхних (нижних) границах элементарных слоев
  for(;it != eit; --it ){
    if(!it.value().isGood(UR_P) || !it.value().isGood(UR_H) || !it.value().isGood(UR_D)) continue;
    Hup = it.value().value(UR_H);
    if(!Dkr.getCloudDeficit(Hup, &Dkrit)) continue;
    deltaDup = Dkrit - it.value().value(UR_D);
    if(deltaDup*deltaDdown < 0.f ||
       (deltaDup > 0.f && MnMath::isZero( deltaDdown)) ||
       (MnMath::isZero(deltaDup) && deltaDdown > 0.f)) {
      t = deltaDdown/(deltaDdown - deltaDup);
      h_vs = (1.f - t) * Hdown + t*Hup;
      if (!oprUrHTpoH(h_vs, &ur)) {
        return false;
      }
      p_oblak->append(ur.value(UR_P));
    }
    deltaDdown = deltaDup;
    Hdown = Hup;
  }

  //!< поиск верхней границы последнего слоя
  it = urList_.begin(); //it - первый (самый нижний) элемент, по завершении цикла
  eit = urList_.end()-1;
  while( it != eit ){
    prev = it; ++it;
    if(!prev.value().isGood(UR_P) || !prev.value().isGood(UR_H) || !prev.value().isGood(UR_D)) continue;
    if(!Dkr.getCloudDeficit(it.value().value(UR_H), &Dkrit)) continue;
    deltaDdown = Dkrit - it.value().value(UR_D);
    if(!Dkr.getCloudDeficit(prev.value().value(UR_H), &Dkrit)) continue;
    deltaDup = Dkrit - prev.value().value(UR_D);
    if(deltaDdown >= 0.f && deltaDup > 0.f) {
      p_oblak->append(prev.value().value(UR_P)); //именно prev (верхняя граница элементарного слоя)
    }
    break;
  }

  if(p_oblak->isEmpty()) return false;
  if(p_oblak->size()%2 != 0){ //если нечетное число точек, то нижняя граница первого слоя облачности - у земли
    //eit--;
    //p_oblak->insert(0, eit.value().value(UR_P)); //может не быть данных по темп-ре и может быть ниже уровня земли
    if (urZ_.isGood(UR_P) && urZ_.isGood(UR_T)) {
      p_oblak->insert(0, urZ_.value(UR_P));
    }
  }

  return true;
}



bool Zond::oprGranObled(QList<float> *p_obled) const//анализ обледенения
{
  if(!isData_) return false;

  Uroven ur, ur1, ur2;
  float deltaDup, deltaDdown = 0.0, Hup, Hdown = 0.0; //, Pup, Pdown, Pvs; //дефицит на границах анализируемых элементарных слоев
  float Tnl; //температура насыщения надо льдом
  float t, h_vs;

  p_obled->clear();

  QMap <Level, Uroven>::const_iterator it  = urList_.constEnd()-1;
  QMap <Level, Uroven>::const_iterator eit = urList_.constBegin();
  QMap <Level, Uroven>::const_iterator prev;

  //!< поиск первой точки с корректными данными и определение разности дефицитов на нижней границе элементарного слоя
  for(;it != eit; --it) {
    if(!it.value().isGood(UR_P) || !it.value().isGood(UR_H) ||
       !it.value().isGood(UR_D) || !it.value().isGood(UR_T)) continue;
    Tnl = -8.f*it.value().value(UR_D);
    deltaDdown =  Tnl - it.value().value(UR_T);
    Hdown = it.value().value(UR_H);
//    Pdown = it.value().value(UR_P);
    //   it.value().print();
    //  debug_log<<"D "<<it.value().value(UR_D)<<"T "<<it.value().value(UR_T)<<"Tnl "<<Tnl<<"deltaDdown "<<deltaDdown<<"Hdown "<<Hdown;
    ++it; break;
  }
  //!< поиск последующих точек с корректными данными и определение разности температур на кривых состояния и стратификации на верхних границах элементарных слоев
  for(;it != eit; --it ){
    if(!it.value().isGood(UR_P) || !it.value().isGood(UR_H) ||
       !it.value().isGood(UR_D) || !it.value().isGood(UR_T)) continue;
    Tnl = -8.f*it.value().value(UR_D);
    deltaDup = Tnl - it.value().value(UR_T);
    Hup = it.value().value(UR_H);
//    Pup = it.value().value(UR_P);
    //  it.value().print();
    //  debug_log<<"D "<<it.value().value(UR_D)<<"T "<<it.value().value(UR_T)<<"Tnl "<<Tnl<<"deltaDdown "<<deltaDdown<<"Hdown "<<Hup;

    if(deltaDup*deltaDdown < 0.f ||
       (deltaDup > 0.f && MnMath::isZero(deltaDdown)) ||
       (MnMath::isZero(deltaDup) && deltaDdown > 0.f)) {
      t = deltaDdown/(deltaDdown - deltaDup);
      h_vs = (1.f - t) * Hdown + t*Hup;

//      if(h_vs > 2000.) break; //TODO метод справедлив только до 2 км
      if (!oprUrHTpoH(h_vs, &ur)) {
        return false;
      }
      // debug_log << "p_obled!"<<h_vs<<ur.value(UR_P);
      p_obled->append(ur.value(UR_P));
    }
    deltaDdown = deltaDup;
    Hdown = Hup;
  }

  //!< поиск верхней границы последнего слоя
  //it = urList_.begin(); //it - первый (самый нижний) элемент, по завершении цикла
  //TODO начальный элемент it - последний элемент предыдущего цикла
  eit = urList_.end()-1;
  while( it != eit ){
    prev = it; ++it;
    if(!prev.value().isGood(UR_P) || !prev.value().isGood(UR_H) ||
       !prev.value().isGood(UR_D) || !prev.value().isGood(UR_T)) continue;

//    if(prev.value().value(UR_H) > 2000.) continue; //TODO метод справедлив только до 2 км
    Tnl = -8.f*it.value().value(UR_D);
    deltaDdown = Tnl - it.value().value(UR_T);
    Tnl = -8.f*prev.value().value(UR_D);
    deltaDup = Tnl - prev.value().value(UR_T);
    if(deltaDdown >= 0.f && deltaDup > 0.f) {
      p_obled->append(prev.value().value(UR_P)); //именно prev (верхняя граница элементарного слоя)
    }
    break;
  }

  if(p_obled->size()%2 != 0){ //если число точек нечетное, значит нижняя граница первого слоя лежит на уровне Земли
    eit--;
    p_obled->insert(0, eit.value().value(UR_P));
  }



///поиск слоев с отсутствием обледенения внутри слоев, найденных по формуле Годске
  double Pvs, Pup, Pdown;
  double lastLayer, firstLayer;
  bool noObledCloudUp, noObledCloudDown;
  bool noDesigion;
  int point_count_in_layer;

  point_count_in_layer = 2;
  //qDebug() << *p_obled;

  for(int i = 0; i < p_obled->count()-1; i += 2){ //поиск по слоям (от земли) с обледенением по Годске
    if(!oprUrHTpoP(p_obled->at(i),&ur1)){ //уровень НГ слоя с обледенением
      return false; //невозможно узнать высоту и температуру на границе - ошибка данных
    }
    if(!oprUrHTpoP(p_obled->at(i+1),&ur2)){ //уровень ВГ слоя с обледенением
      return false; //невозможно узнать высоту и температуру на границе - ошибка данных
    }

    firstLayer = p_obled->at(i);
    lastLayer = p_obled->at(i+1);

    // поиск в слое первой (нижней) точки от земли с отсутствием обледенения
    noObledCloudDown = getNoObledCloud(ur1.value(UR_T), ur1.value(UR_D));
    Pdown = ur1.value(UR_P);

    it  = urList_.lowerBound(p_obled->at(i));
    eit = urList_.upperBound(p_obled->at(i+1));
    if ( it == urList_.end() ) {
      return false;
    }
    while( it != eit ){ //поиск (от земли) точек внутри слоя с обледенением, где не выполняются условия отсутствия обледенения
      --it;
      if ( it == urList_.end() || it == urList_.begin() - 1 ) {
        return false;
      }
      if(!oprUrHTpoP(it.key(),&ur)){
        continue; //невозможно узнать высоту и температуру - переход на следующую точку
      }
      noObledCloudUp = getNoObledCloud(ur.value(UR_T), ur.value(UR_D));
      Pup = ur.value(UR_P);
      if(noObledCloudDown != noObledCloudUp){
        //поиск границы слоя обледенения методом половинного деления
        Pvs = bisectBorderObledCloud(Pup, Pdown, 1e-6, &noDesigion);
        // qDebug() << "Pvs=" << Pvs << "Pup=" << Pup << "Pdown=" << Pdown << noDesigion;
        if(!noDesigion){//////TODO алгоритм сдвига точки
          p_obled->insert(i+1, Pvs);
          ++point_count_in_layer;
          ++i;
        }
      }
      Pdown = Pup;
      noObledCloudDown = noObledCloudUp;
    }
    if(getNoObledCloud(ur1.value(UR_T), ur1.value(UR_D))){//////TODO алгоритм сдвига точки remove
     //(p_obled->at(i));
       if(!p_obled->removeOne(firstLayer)){
         return false;
       }
      --point_count_in_layer;
      --i;
    }

    if(getNoObledCloud(ur2.value(UR_T), ur2.value(UR_D)) ){//////TODO алгоритм сдвига точки remove
      if(!p_obled->removeOne(lastLayer)){//append(p_obled->at(i+1));
        return false;
      }
      --point_count_in_layer;
      --i;

    }

    if(point_count_in_layer%2 != 0) { //число точек в слое нечетное, значит ошибка в расчетах
      return false;
//      for(int  i = 0; i < point_count_in_layer; i++) { //удаление всех точек обледенения в слое
//        p_obled->removeLast();
//      }
    }

  }//окончание поиска слоев с обледенением по Годске


/////////////////////////////////////////////////////////////////////////////////////////////////

  if(p_obled->isEmpty()) return false;

  return true;
}


bool Zond::oprGranBoltan(QList<float> *p_boltan) const//анализ болтанки
{
  if(!isData_) return false;

  Uroven urDown, urUp;
  float deltaU, deltaV; //, delta_dd, delta_ff;
  double gam_t, gam_u, gam_v, t_sr; //модуль вертикального градиента температуры, скорости и направления ветра

  bool boltan = false;

  p_boltan->clear();

  QMap <Level, Uroven>::const_iterator it =   urList_.end();
  QMap <Level, Uroven>::const_iterator eit = urList_.begin();


  //первая нижняя граница слоя
  while( it != eit ) {
    --it;
    if(!oprUrHTpoP(  it.value().value(UR_P), &urDown)) continue;
    if(!oprUrWindPoP(it.value().value(UR_P), &urDown)) continue;
    break;
  }

  //верхняя граница слоя - нижняя граница следующего слоя
  while( it != eit ) {
    --it;
    if(!oprUrHTpoP(  it.value().value(UR_P),   &urUp  )) continue;
    if(!oprUrWindPoP(it.value().value(UR_P),   &urUp  )) continue;

    deltaU = urUp.value(UR_u) - urDown.value(UR_u);
    deltaV = urUp.value(UR_v) - urDown.value(UR_v);
    //   MnMath::preobrUVtoDF(deltaU, deltaV, &delta_dd, &delta_ff);

    //   gam_ff = fabs(delta_ff)/(urUp.value(UR_H) - urDown.value(UR_H));//*1000.;  // градиент на 1 км

    //   gam_dd = fabs(delta_dd)/(urUp.value(UR_H) - urDown.value(UR_H));//*1000.;  // градиент на 1 км

    gam_t  = -(urUp.value(UR_T) - urDown.value(UR_T))/
             (urUp.value(UR_H) - urDown.value(UR_H));//*1000.;             // градиент на 1 км

    t_sr = 0.5f*(urUp.value(UR_T) + urDown.value(UR_T));

    gam_u = fabs(deltaU)/(urUp.value(UR_H) - urDown.value(UR_H));//*1000.;  // градиент на 1 км

    gam_v = fabs(deltaV)/(urUp.value(UR_H) - urDown.value(UR_H));//*1000.;  // градиент на 1 км

    boltan = getBoltan(gam_u, gam_v, gam_t, t_sr);
    //   qDebug() << "Tsr ="<< t_sr << "deltaWind:" << deltaU << deltaV;
    //   qDebug() << "gradT ="<< gam_t;
    //qDebug() << "P ="<< urUp.value(UR_P) << "Boltan=" << boltan;


    if(boltan){
      if(p_boltan->size()> 0 && MnMath::isEqual(p_boltan->last(), urDown.value(UR_P)) ){
        p_boltan->removeLast();
      } else {
        p_boltan->append(urDown.value(UR_P));
      }
      p_boltan->append(urUp.value(UR_P));
    }
    urDown = urUp;
  }

  if(p_boltan->isEmpty())     return false;
  if(p_boltan->size()%2 != 0) return false; //если число точек нечетное, значит нижняя граница первого слоя лежит на уровне Земли

  return true;
}


//вертикальный градиент температуры воздуха м-1;
bool Zond::getGamm(float lvl, float *gam) const{
  QList<Level>levl = urList_.uniqueKeys();
  qSort(levl);
  int kol_lev = levl.count();
  Uroven urDown;
  if(!getUrPoP(lvl,&urDown)) return false;
  if(!urDown.isGood(UR_T) || !urDown.isGood(UR_H) ) return false;
  for(int i = kol_lev-1; i >-1;  --i ){
    double p = levl.at(i).p();
    if(p >= lvl) continue;
    Uroven urUp;
    if(!getUrPoP(p,&urUp)) continue;
    if(!urUp.isGood(UR_T) || !urUp.isGood(UR_H) ) continue;
    *gam = -(urUp.value(UR_T) - urDown.value(UR_T))/
           (urUp.value(UR_H) - urDown.value(UR_H));
    return true;
  }
  return false;
}


bool Zond::oprGranBoltan_Td(QList<float> *p_boltan, float fi) const//анализ болтанки
{
  if(!isData_ || 1 > gradTxList_.size() ) return false;

  p_boltan->clear();
  QList<double> levl = gradTxList_.uniqueKeys();
  int kol_grad = levl.count();
  float w=1.4584e-4f;
  float teta = fi +M_PI_2f;
  float f = w*cosf(teta);
  f = f*f/G;

  for(int i =0; i < kol_grad; ++i ){
    Uroven ur;
    float p = levl.at(i);
    if(!getUrPoP(p,&ur)) continue;
    float gam;
    if(!getGamm(p, &gam)) continue;
    float tx =gradTxList_.value(p);
    float ty =gradTyList_.value(p);
    float gradt = tx*tx+ty*ty;
    float t = ur.value(UR_T)+kKelvin ;
    float Ri = t*f*(GamTDryAdiab - gam)/gradt;
    //debug_log << "ri "<<Ri;
    if(1.f >= Ri ) p_boltan->append(p);
  }

  return p_boltan->isEmpty();
}

bool Zond::oprGranTrace(QList<float> *p_trace)const
{
  if(!isData_ || keyType_ != UR_P ) return false;

  Uroven urUp, urDown, ur;
  double deltaSup1 = 0.0, deltaSdown1, deltaSup2 = 0.0, deltaSdown2;
  float sUp, sDown, Hup=0, Hdown;
  double t, h_vs;

  const double s1 = 0.00010;
  const double s2 = 0.00015;

  QMap <Level, Uroven>::const_iterator it =   urList_.begin();



  //первая верхняя граница элементарного слоя, начиная от точки B (уровень pB)
  for(;it != urList_.end(); ++it){
    if(!oprUrHTpoP( it.key(), &urUp)) continue;
    if(!oprSPoTP(it.key(),urUp.value(UR_T),&sUp)) continue; //массовая доля вод. пара для изограммы
    deltaSup1 = sUp - s1; //для нижней границы
    deltaSup2 = s2 - sUp; //для верхней границы
    Hup = urUp.value(UR_H);
    if(deltaSup1 >= 0. && deltaSup2 >= 0.){ //если самая верхняя точка лежит в пределах s1 и s2
      p_trace->append(urUp.value(UR_P));
    };
    ++it;
    break;
  }


  //вторая нижняя граница элементарного слоя
  for( ;it != urList_.end(); ++it) {
    if(!oprUrHTpoP(it.key(), &urDown)) continue;
    if(!oprSPoTP(it.key(),urDown.value(UR_T),&sDown)) continue;
    deltaSdown1 = sDown - s1;//для нижней границы
    deltaSdown2 = s2 - sDown;//для верхней границы
    Hdown = urDown.value(UR_H);

    //проверка наличия нижней границы
    if(deltaSup1*deltaSdown1 < 0. ||
       (  deltaSup1 > 0. && qFuzzyCompare(1. + deltaSdown1,  1.)) ||
       (deltaSdown1 > 0. && qFuzzyCompare(1. + deltaSup1,    1.))) { //TODO возможно это (второе) условие лишнее
      t = deltaSdown1/(deltaSdown1 - deltaSup1);
      h_vs = (1. - t) * Hdown + t*Hup;
      if (!oprUrHTpoH(h_vs, &ur)) {
        return false;
      }
      p_trace->append(ur.value(UR_P));
    }
    //проверка наличия верхней границы
    if(deltaSup2*deltaSdown2 < 0. ||
       (  deltaSup2 > 0. && qFuzzyCompare(1. + deltaSdown2,  1.)) ||
       (deltaSdown2 > 0. && qFuzzyCompare(1. + deltaSup2,    1.))) { //TODO возможно это (второе) условие лишнее
      t = deltaSdown2/(deltaSdown2 - deltaSup2);
      h_vs = (1. - t) * Hdown + t*Hup;
      if (!oprUrHTpoH(h_vs, &ur)) {
        return false;
      }
      p_trace->append(ur.value(UR_P));
    }


    deltaSup1 = deltaSdown1 ;
    deltaSup2 = deltaSdown2 ;
    Hup = Hdown;

  }

  if(p_trace->isEmpty())     return false;
  if(p_trace->size()%2 != 0) return false; //если число точек нечетное, значит нижняя граница первого слоя лежит на уровне Земли

  return true;

}

bool Zond::getTmaxTitov(float *t)
{
  float T700, Td700, T850, Td850, T900;
  Uroven ur700, ur850, ur900;
  if(!oprUrHTpoP(700.,&ur700) || !oprUrHTpoP(850.,&ur850) || !oprUrHTpoP(900.,&ur900)) return false;
  T700 = ur700.value(UR_T);
  Td700 = ur700.value(UR_Td);
  T850 = ur850.value(UR_T);
  Td850 = ur850.value(UR_Td);
  T900 = ur900.value(UR_T);

  float D850, D700;
  float Ds, Tmax;
  float T1,T2;
  T1 = 8.;
  T2 = 12.; // Для неадаптированных методов
  D850 = T850 - Td850;
  D700 = T700 - Td700;

  Ds = D850 + D700;

  if(Ds < 3.5f) {
    Tmax = T900 + T1;
  } else {
    Tmax = T900 + T2;
  }
  *t=Tmax;
  return true;
}

bool Zond::oprKNS(float *p, float Tmax) //граница конвективно-неустойчивого слоя
{
  if(!isData_ || keyType_ != UR_P ) return false;
  if(MnMath::isEqual(Tmax,BAD_METEO_ELEMENT_VAL )){
    if(!getTz(&Tmax)) return false;
  }

  double gamT     = BAD_METEO_ELEMENT_VAL;
  double gamTPrev = BAD_METEO_ELEMENT_VAL;
  Uroven urDown, urUp, ur;
  bool interceptingLayer = false;

  float Pz = BAD_METEO_ELEMENT_VAL;
   double gamWet   = BAD_METEO_ELEMENT_VAL;
  if(!getPz(&Pz)) return false;

  *p = Pz; // во всех случаях, когда в атмосфере КНС нет, его граница совпадает с уровнем земли

  QMap <Level, Uroven>::iterator it  = urList_.end();
  QMap <Level, Uroven>::iterator eit = urList_.begin();
  QMap <Level, Uroven>::iterator bit = urList_.upperBound(850);//поиск слоя,ближайшего к 850 гПа снизу

  QMap <Level, Uroven>::iterator it1  = urList_.begin();

  it = bit;
  //первая нижняя граница элементарного слоя
  while( it != eit ) {
    --it;
    if(!oprUrHTpoP(  it.key(), &urDown)) continue;
    break;
  }

  //верхняя граница элементарного слоя - нижняя граница следующего элементарного слоя

  while( it != eit ) {
    --it;
    if(!oprUrHTpoP(  it.key(),   &urUp  )) continue;
    gamT = -(urUp.value(UR_T) - urDown.value(UR_T))/
           (urUp.value(UR_H) - urDown.value(UR_H));                 // градиент T на 1 м в слое
    gamWet = getGamTWetAdiab(urUp.value(UR_P), urUp.value(UR_T)); // влажноадиабатический градиент на верхней границе слоя
    if(gamT >= gamWet && gamWet > gamTPrev){
      interceptingLayer = true; // определение наличия задерживающего слоя при ниличии ниже него неустойчивого слоя
      break;
    }
    gamTPrev = gamT;
    urDown = urUp;
  }
  if(MnMath::isEqual(gamT,BAD_METEO_ELEMENT_VAL)) return false; //недостаточно данных

  if(interceptingLayer){ //при наличии задерживающего слоя
    float tet,tpot, s;
    float p1, p2, tDry1, tDry2, tDry, tWet1, tWet2, tWet;
    float pB = 0.0, tB;
    float t, h_vs;
    float deltaTup, deltaTdown, Hup, Hdown; //температура на границах анализируемых элементарных слоев

    //определение точки B (пересечение влажной адиабаты и сухой адиабаты)
    if(!oprTet( Pz, Tmax, &tet )) return false;                            //потенциальная температура для сухой адиабаты от Tmax
    if(!oprTPot(urUp.value(UR_P), urUp.value(UR_T), &tpot)) return false;  //псевдопотенциальная температура для влажной адиабаты от B

    p1=1000.;
    p2=100.;

    if(!oprTPoTet(p1, tet, &tDry1)) return false;
    if(!oprTPoTPot(p1,tpot,&tWet1)) return false;
    if(!oprTPoTet(p2, tet, &tDry2)) return false;
    if(!oprTPoTPot(p2,tpot,&tWet2)) return false;
    if((tDry1-tWet1)*(tDry2-tWet2) > 0.) return false; //кривые не пересекаются
    tDry = tDry1;
    tWet = tWet1;
    float oldpB = p1;
//    while(fabs(tDry-tWet) > LITTLE_VALUE) {
    while(false == MnMath::isEqual(tDry,tWet) &&
          false == MnMath::isEqual(oldpB, pB)) {
      oldpB = pB;
      pB = (p1 + p2)*0.5;

      if(!oprTPoTet( pB,tet, &tDry)) return false;
      if(!oprTPoTPot(pB,tpot,&tWet)) return false;
      if((tDry1-tWet1)*(tDry-tWet) > 0.) p1 = pB; else p2 = pB;
    }
    tB = tDry;

    //определение точки Д (пересечение изограммы и кривой точек росы)
    if(!oprSPoTP(pB,tB,&s)) return false; //массовая доля вод. пара для изограммы

    urDown.reset();
    urUp.reset();

    //первая верхняя граница элементарного слоя, начиная от точки B (уровень pB)
    if(!oprUrHTpoP( pB, &urUp)) return false;
    if(!oprTPoSP(pB,s,&tWet1)) return false;
    deltaTup = tWet1 - urUp.value(UR_Td);
    Hup = urUp.value(UR_H);
    if( deltaTup <= 0. ) { // если кривая точек росы лежит правее чем изограмма (температура на изограмме ниже точки росы)
      *p = pB;
      return true;
    }

    it1 = urList_.upperBound(pB);


    //вторая нижняя граница элементарного слоя
    for( ;it1 != urList_.end(); ++it1) {
      if(!oprUrHTpoP(it1.key(), &urDown)) continue;
      if( !oprTPoSP(it1.key(), s, &tWet2)) continue;
      deltaTdown = tWet2 - urDown.value(UR_Td);
      Hdown = urDown.value(UR_H);


      if(deltaTup*deltaTdown < 0.f ||
         (  deltaTup > 0.f && MnMath::isZero(deltaTdown)) ||
         (deltaTdown > 0.f && MnMath::isZero(deltaTup))) { //TODO возможно это (второе) условие лишнее
        t = deltaTdown/(deltaTdown - deltaTup);
        h_vs = (1.f - t) * Hdown + t*Hup;
        if (!oprUrHTpoH(h_vs, &ur)) {
          return false;
        }
        *p = ur.value(UR_P);
        return true;
      }
      deltaTdown = deltaTup;
      Hdown = Hup;
    }

    return false;
  }


  //Если атмосфера абсолютно устойчива

  if( it == eit && gamT < gamWet ){// при абсолютно устойчивой атмосфере (gamT < gamWet на верх. границе) без отдельного задерживающего слоя
    float tet,tpot, s;
    float p1, p2, tDry1, tDry2, tDry, tWet1, tWet2, tWet, tSost1, tSost2;
    double pB = 0.0, tB, pZ = 0.0;
    float p_kondens, t_kondens;
    double deltaTup, deltaTdown = 0.0, Hup = 0.0, Hdown = 0.0, t, h_vs;

    bool success;

    urUp.reset();

    //определение точки Z (пересечение кривой состояния от Tmax с кривой стратификации)
    if(!oprUrHTpoP( Pz,   &urUp )) return false; //определение параметров на уровне земли
    if(!zond::oprPkondens(Tmax, urUp.value(UR_Td), Pz, &p_kondens, &t_kondens)) return false; //определение уровня конденсации от Tmax
    if(!oprTet( Pz, Tmax, &tet)) return false;             //потенциальная температура для сухой адиабаты от Tmax
    if(!oprTPot(p_kondens, t_kondens, &tpot)) return false;//псевдопотенциальная температура для влажной адиабаты от УК при Tmax у земли


    urDown.reset();
    urUp.reset();

    it1 = urList_.upperBound(Pz);

    //первая нижняя граница элементарного слоя - поиск с самого нижнего уровня
    while( it1 != urList_.begin() ){
      --it1;
      if(!oprUrHTpoP( it1.key(), &urDown)) continue;
      success = p_kondens <  it1.key() ? oprTPoTet(it1.key(), tet, &tSost1) : oprTPoTPot(it1.key(), tpot, &tSost1);
      if(!success) continue;
      deltaTdown = tSost1 - urDown.value(UR_T);
      Hdown = urDown.value(UR_H);
      if( deltaTdown <= 0. ){ // если кривая стратификации лежит правее чем кривая состояния (температура частицы ниже температуры воздуха)
        return false; //энергия неустойчивости отрицательная
      }
      break;
    }

    //вторая верхняя граница элементарного слоя, поиск точки Z по слоям
    while( it1 != urList_.begin() ){
      --it1;
      if(!oprUrHTpoP(it1.key(), &urUp)) continue;
      success = p_kondens <  it1.key() ?
            oprTPoTet(it1.key(), tet, &tSost2) : oprTPoTPot(it1.key(), tpot, &tSost2);
      if(!success) continue;
      deltaTup = tSost2 - urUp.value(UR_T);
      Hup = urUp.value(UR_H);

      if(deltaTup*deltaTdown < 0. ||
         (  deltaTup > 0. && MnMath::isZero(deltaTdown)) ||  //TODO возможно это (первое) условие лишнее
         (deltaTdown > 0. && MnMath::isZero(deltaTup))) {
        t = deltaTdown/(deltaTdown - deltaTup);
        h_vs = (1. - t) * Hdown + t*Hup;
        if (!oprUrHTpoH(h_vs, &ur)) {
          return false;
        }
        pZ = ur.value(UR_P);
        break;
      }
      //
      deltaTup = deltaTdown;
      Hup = Hdown;
    }
    if(it1 == urList_.begin() && MnMath::isEqual( Hup, Hdown)) return false;


    //определение точки M (спуск по кривой стратификации на 100-50 гПа)
    ur.reset();
    if (!oprUrHTpoP(pZ - 50, &ur)) {
      return false;
    }

    //определение точки B (пересечение кривой состояния от точки М с сухой адиабатой от Tmax у земли)

    if(!oprTet( Pz, Tmax,&tet)) return false; //потенциальная температура для сухой адиабаты при Tmax у земли
    if(!oprTPot(ur.value(UR_P), ur.value(UR_T), &tpot)) return false;  //псевдопотенциальная температура для влажной адиабаты от M


    p1=1000.;
    p2=100.;

    if(!oprTPoTet(p1, tet, &tDry1)) return false;
    if(!oprTPoTet(p2, tet, &tDry2)) return false;
    if(!oprTPoTPot(p1, tpot, &tWet1)) return false;
    if(!oprTPoTPot(p2, tpot, &tWet2)) return false;
    if((tDry1-tWet1)*(tDry2-tWet2) > 0.f) return false;
    tDry = tDry1;
    tWet = tWet1;
    double oldpB = p1;
    //while(fabs(tDry - tWet) > LITTLE_VALUE) {
    while(false == MnMath::isEqual(tDry,tWet) &&
          false == MnMath::isEqual(oldpB, pB)) {
      oldpB = pB;
      pB = (p1 + p2)*0.5f;
      if(!oprTPoTet(pB, tet, &tDry)) return false;
      if(!oprTPoTPot(pB, tpot, &tWet)) return false;
      if((tDry1-tWet1)*(tDry-tWet) > 0.f) p1 = pB; else p2 = pB;
    }
    tB = tDry;
    //определение точки Д (пересечение изограммы с кривой точек росы)

    if(!oprSPoTP(pB,tB,&s)) return false; //массовая доля вод. пара для изограммы

    urDown.reset();
    urUp.reset();

    //первая верхняя граница элементарного слоя, начиная от точки B (уровень pB)
    if(!oprUrHTpoP( pB, &urUp)) return false;
    if(!oprTPoSP(pB,s,&tWet1)) return false;
    deltaTup = tWet1 - urUp.value(UR_Td);
    Hup = urUp.value(UR_H);
    if( deltaTup <= 0. ) { // если кривая точек росы лежит правее чем изограмма (температура на изограмме ниже точки росы)
      *p = pB;             //точка Д совпадает с точкой B
      return true;
    }

    it1 = urList_.upperBound(pB);


    //вторая нижняя граница элементарного слоя
    for( ;it1 != urList_.end(); ++it1) {
      if(!oprUrHTpoP(it1.key(), &urDown)) continue;
      if( !oprTPoSP(it1.key(), s, &tWet2)) continue;
      deltaTdown = tWet2 - urDown.value(UR_Td);
      Hdown = urDown.value(UR_H);


      if(deltaTup*deltaTdown < 0. ||
         (  deltaTup > 0. && MnMath::isZero(deltaTdown)) ||
         (deltaTdown > 0. && MnMath::isZero(deltaTup))) { //TODO возможно это (второе) условие лишнее
        t = deltaTdown/(deltaTdown - deltaTup);
        h_vs = (1. - t) * Hdown + t*Hup;
        if (!oprUrHTpoH(h_vs, &ur)) {
          return false;
        }
        *p = ur.value(UR_P); // точка Д
        return true;
      }

      deltaTdown = deltaTup;
      Hdown = Hup;

    }
    return false;
  }

  if( it == eit && gamT >= gamWet){   // при устойчивой атмосфере (gamT >= gamWet на верх. границе),
    // если атмосфера абсолютно неустойчива, то КНС невозможно определить gamT > gamDry
    float tet, s;
    float tSost1, tSost2, tWet1, tWet2;
    double deltaTdown = 0.0, deltaTup, Hdown = 0.0, Hup = 0.0;
    double h_vs, t;
    double pB = 0.0, tB = 0.0;


    //определение точки B
    if(!oprTet( Pz, Tmax, &tet)) return false;             //потенциальная температура для сухой адиабаты от Tmax

    urDown.reset();
    urUp.reset();

    it1 = urList_.upperBound(Pz);

    //первая нижняя граница элементарного слоя
    while( it1 != urList_.begin() ){
      --it1;
      if(!oprUrHTpoP( it1.key(), &urDown)) continue;
      if(!oprTPoTet(  it1.key(), tet, &tSost1)) continue;
      deltaTdown = tSost1 - urDown.value(UR_T);
      Hdown = urDown.value(UR_H);
      if( deltaTdown <= 0. ){ // если кривая стратификации лежит правее чем кривая состояния (температура частицы ниже температуры воздуха)
        return false; //энергия неустойчивости отрицательная
      }
      break;
    }

    //вторая верхняя граница элементарного слоя, поиск точки B по слоям
    while( it1 != urList_.begin() ){
      --it1;
      if(!oprUrHTpoP(it1.key(), &urUp)) continue;
      if(!oprTPoTet(  it1.key(), tet, &tSost2)) continue;
      deltaTup = tSost2 - urUp.value(UR_T);
      Hup = urUp.value(UR_H);

      if(deltaTup*deltaTdown < 0. ||
         (  deltaTup > 0. && MnMath::isZero(deltaTdown)) || //TODO возможно это (первое) условие лишнее
         (deltaTdown > 0. && MnMath::isZero( deltaTup))) {
        t = deltaTdown/(deltaTdown - deltaTup);
        h_vs = (1. - t) * Hdown + t*Hup;
        if (!oprUrHTpoH(h_vs, &ur)) {
          return false;
        }
        pB = ur.value(UR_P);
        tB = ur.value(UR_T);
        break;
      }
      deltaTup = deltaTdown;
      Hup = Hdown;
    }
    if(it1 == urList_.begin() && MnMath::isEqual( Hup, Hdown)) return false;


    //определение точки Д
    if(!oprSPoTP(pB,tB,&s)) return false; //массовая доля вод. пара для изограммы

    urDown.reset();
    urUp.reset();

    //первая верхняя граница элементарного слоя, начиная от точки B (уровень pB)
    if(!oprUrHTpoP( pB, &urUp)) return false;
    if(!oprTPoSP(pB,s,&tWet1)) return false;
    deltaTup = tWet1 - urUp.value(UR_Td);
    Hup = urUp.value(UR_H);
    if( deltaTup <= 0. ) { // если кривая точек росы лежит правее чем изограмма (температура на изограмме ниже точки росы)
      *p = pB;              //точка Д совпадает с точкой B
      return true;
    }

    it1 = urList_.upperBound(pB);


    //вторая нижняя граница элементарного слоя
    for( ;it1 != urList_.end(); ++it1) {
      if(!oprUrHTpoP(it1.key(), &urDown)) continue;
      if( !oprTPoSP(it1.key(), s, &tWet2)) continue;
      deltaTdown = tWet2 - urDown.value(UR_Td);
      Hdown = urDown.value(UR_H);


      if(deltaTup*deltaTdown < 0. ||
         (  deltaTup > 0. && MnMath::isZero( deltaTdown)) ||
         (deltaTdown > 0. && MnMath::isZero( deltaTup))) {
        t = deltaTdown/(deltaTdown - deltaTup);
        h_vs = (1. - t) * Hdown + t*Hup;
        if (!oprUrHTpoH(h_vs, &ur)) {
          return false;
        }
        *p = ur.value(UR_P); // точка Д
        return true;
      }
      deltaTdown = deltaTup;
      Hdown = Hup;
    }
    return false;
  }

  return true;
}

bool Zond::averConvecLevel(float *p, float Tmax)//средний уровнеь конвекции
{
  if(!isData_) return false;
  if(MnMath::isEqual( Tmax, BAD_METEO_ELEMENT_VAL)){
    if(!getTz(&Tmax)) return false;
  }

  Uroven urUp, urDown, ur;
  double deltaTup=0, deltaTdown, Hup = 0.0, Hdown; //температура на границах анализируемых элементарных слоев
  float Tsost, t, h_vs;

  QMap <Level, Uroven>::iterator it = urList_.begin();

  //!< поиск верхней границы последнего КНС
  //первая верхняя граница элементарного слоя
  for( ;it != urList_.end(); ++it) {
      urUp = *it;
    //if(!oprUrHTpoP( it.key(), &urUp)) continue;
    if(!oprTsost(it.key(),&Tsost)) continue;
    deltaTup = Tsost - urUp.value(UR_T);
    Hup = urUp.value(UR_H);
    if( deltaTup >= 0. ) { // если кривая состояния лежит правее чем стратификации (температура воздуха ниже температуры частицы)
      *p = urUp.value(UR_P);
      return true;
    }
    break;
  }


  //вторая нижняя граница элементарного слоя
  for( ;it != urList_.end(); ++it) {
    if(!oprUrHTpoP(it.key(), &urDown)) continue;
    if(!oprTsost(it.key(),&Tsost)) continue;
    deltaTdown = Tsost - urDown.value(UR_Td);
    Hdown = urDown.value(UR_H);


    if(deltaTup*deltaTdown < 0. ||
       (  deltaTup < 0. && MnMath::isZero( deltaTdown)) ) { //TODO возможно это (второе) условие лишнее
      t = deltaTdown/(deltaTdown - deltaTup);
      h_vs = (1. - t) * Hdown + t*Hup;
      if (!oprUrHTpoH(h_vs, &ur)) {
        return false;
      }
      *p = ur.value(UR_P);
      return true;
    }

    deltaTdown = deltaTup;
    Hdown = Hup;

  }
  return false;
}


bool Zond::averCondensLevel(float *p, float *t, float Tmax)//средний уровень конденсации
{
  if(!isData_) return false;
  if(MnMath::isEqual( Tmax, BAD_METEO_ELEMENT_VAL)){
    if(!getTz(&Tmax)) return false;
  }
  Uroven ur;
  float Pkns, Pz;
  float Hz, Hkns;
  float t_nach,td_nach,p_nach;

  if(!oprKNS(&Pkns, Tmax)) return false;

  //с середины КНС
  if(!getPz(&Pz)) return false;
  if(!oprUrHTpoP(Pz, &ur)) return false;
  Hz = ur.value(UR_H);
  ur.reset();

  if(!oprUrHTpoP(Pkns, &ur)) return false;
  Hkns = ur.value(UR_H);
  ur.reset();

  if(!oprUrHTpoH( 0.5f*(Hkns + Hz), &ur )) return false; //середина КНС

  // p_nach, t_nach, td_nach, inv
  p_nach = ur.value(UR_P);
  t_nach = ur.value(UR_T);
  td_nach = ur.value(UR_Td);

  if(!zond::oprPkondens(t_nach, td_nach, p_nach, p, t)) return false;

  return true;
}





bool Zond::getAverWindUV(float h1, float h2, float *u, float *v)
{
  if(!isData_ || (keyType_ != UR_P && keyType_ != UR_H) ) return false;
  if(h1 > h2)  return false;
  if(MnMath::isEqual(h1, h2)) {
    Uroven ur;
    oprUrWindPoH(h1,&ur);
    *u = ur.value(UR_u);
    *v = ur.value(UR_v);
    return true;
  }

  Uroven ur, ur1, ur2;

  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator prev_it;
  QMap <Level, Uroven>::iterator bit;
  QMap <Level, Uroven>::iterator eit;

  //поиск значений в граничных точках

  if ( !oprUrWindPoH(h1, &ur1) ) { return false; }
  if ( !oprUrWindPoH(h2, &ur2) ) { return false; }

  //поиск начальной и конечной точек в диапазоне [h1,h2] из имеющегося массива urList_
  if(keyType_ == UR_H){
    bit = urList_.upperBound(h1); //итератор на следующее за h1 значение по возрастанию ключа H (высоты)
    eit = urList_.upperBound(h2)-1; //итератор на следующее за h2 значение по убыванию ключа H (высоты)

    if ( bit.key() > eit.key() ) { return false; }
  }
  if(keyType_ == UR_P){
    bit = urList_.lowerBound(ur1.value(UR_P))-1; //итератор на следующее за P1 значение по убыванию ключа P (давления)
    eit = urList_.lowerBound(ur2.value(UR_P)); //итератор на следующее за P2 значение по возрастанию ключа P (давления)
     if ( bit.key() < eit.key() ) { return false; }
  }

  it = bit;
  *u = 0;
  *v = 0;
  float prev_u = ur1.value(UR_u);
  float prev_v = ur1.value(UR_v);
  float tolsh = h2 - h1; //расчет толщины H всего слоя между h1 и h2
  float prev_h = h1;
  float dh;
  //численное интегрирование VdH с переменным шагом dH
  while( it != eit ) {
    ur.reset();
    if ( !it.value().isGood(UR_u) || !it.value().isGood(UR_v) || !it.value().isGood(UR_H) ) {
//    if( false == it.value().isGood(UR_H) /*|| !oprUrWindPoH(it.value().value(UR_H), &ur) TODO слишком долго считается высота, когда уровней много */ ){
      if(keyType_ == UR_H) it++;
      if(keyType_ == UR_P) it--;
      continue;
    }
    ur = *it; 

    dh = ur.value(UR_H) - prev_h;          //толщина элементарного слоя dH

    (*u) += 0.5f*(ur.value(UR_u) + prev_u)*dh; //значение u в элементарном слое dH (среднее между значениями на границах dH), умноженное на dH
    (*v) += 0.5f*(ur.value(UR_v) + prev_v)*dh; //значение v в элементарном слое dH (среднее между значениями на границах dH), умноженное на dH

    prev_h = ur.value(UR_H);
    prev_u = ur.value(UR_u);
    prev_v = ur.value(UR_v);
    prev_it = it;
    if(keyType_ == UR_H) it++;
    if(keyType_ == UR_P) it--;
  }
  //последний элементарный слой dH и завершение интегрирования VdH
  dh = h2 - prev_h;          //толщина элементарного слоя dH
  (*u) += 0.5f*(ur2.value(UR_u) + prev_u)*dh; //значение u в элементарном слое dH (среднее между значениями на границах dH), умноженное на dH
  (*v) += 0.5f*(ur2.value(UR_v) + prev_v)*dh; //значение v в элементарном слое dH (среднее между значениями на границах dH), умноженное на dH
  //определение составляющих u, v среднего ветра
  (*u) /= tolsh;
  (*v) /= tolsh;

  return true;
}

bool Zond::getAverWindDF(float h1, float h2, float *dd, float *ff)
{
  float u,v;
  if(!getAverWindUV(h1, h2, &u, &v)) return false;
  MnMath::preobrUVtoDF(u, v, dd, ff);
  return true;
}

/**
 * @brief Определение среднего значения параметра на высоте или уровне
 * @param type_par тип определяемого параметра
 * @param type_ur  тип задаваемого уровня (высота, давление) на границах слоя
 * @param ph1 нижняя граница слоя, м или гПа
 * @param ph2 верхняя граница слоя. м или гПа
 * @param sr_par среднее значение определяемого параметра
 * @return bool успешность определения среднего значения
 **/
bool Zond::oprSrParamPoPH(ValueType type_par, ValueType type_ur, float ph1, float ph2, float *sr_par)
{

  if(!isData_ || (keyType_ != UR_P && keyType_ != UR_H)) return false;


  Uroven ur, ur1, ur2;
  QList<float> vsX;
  QList<float> vsY;


  if(type_ur == UR_P && ph1<ph2) { return false; }
  if(type_ur == UR_H && ph2<ph1) { return false; }


  if(type_ur == UR_P){
    //нахождение значения параметра на уровне ph1 (проверка попадания в область, охваченную данными проводится в функциях getUrPoP)
    if(!getUrPoP(ph1, &ur1)) { return false; }
    //нахождение значения параметра на уровне ph2
    if(!getUrPoP(ph2, &ur2)) { return false; }
  }
  if(type_ur == UR_H){
    //нахождение значения параметра на уровне ph2
    if(!getUrPoH(ph2, &ur2)) { return false; }
    //нахождение значения параметра на уровне ph1 (проверка попадания в область, охваченную данными проводится в функциях getUrPoP)
    if(!getUrPoH(ph1, &ur1)) { return false; }

  }

  if(!ur1.isGood(type_par)) { return false; }
  if(!ur2.isGood(type_par)) { return false; }

  //заполнение массивов значениями на уровне ph1
  vsX.append(ur1.value(type_ur));
  vsY.append(ur1.value(type_par));

  //заполнение массивов значениями между уровнями ph1 и ph2
  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator prev_it;
  QMap <Level, Uroven>::iterator bit;
  QMap <Level, Uroven>::iterator eit;

  if(type_ur == UR_H && keyType_ == UR_H){
    bit = urList_.upperBound(ph1); //итератор на следующее за h1 значение по возрастанию ключа H (высоты)
    eit = urList_.lowerBound(ph2); //итератор на следующее за h2 значение по убыванию ключа H (высоты)
  }
  if(type_ur == UR_P && keyType_ == UR_P){
    bit = urList_.lowerBound(ph1); //итератор на следующее за P1 значение по убыванию ключа P (давления)
    eit = urList_.upperBound(ph2); //итератор на следующее за P2 значение по возрастанию ключа P (давления)
  }
  if(type_ur == UR_H && keyType_ == UR_P){
    bit = urList_.lowerBound(ur1.value(UR_P)); //итератор на следующее за P1 значение по убыванию ключа P (давления)
    eit = urList_.upperBound(ur2.value(UR_P)); //итератор на следующее за P2 значение по возрастанию ключа P (давления)
  }
  if(type_ur == UR_P && keyType_ == UR_H){
    bit = urList_.upperBound(ur1.value(UR_H)); //итератор на следующее за h1 значение по возрастанию ключа H (высоты)
    eit = urList_.lowerBound(ur2.value(UR_H)); //итератор на следующее за h2 значение по убыванию ключа H (высоты)
  }

  it = bit;
  while( it != eit ){
    if(keyType_ == UR_P) it--;

    if(type_ur == UR_H){
      if ( UR_u == type_par || UR_v == type_par ) {
        if(!oprUrWindPoH(it.value().value(type_ur),&ur)){  return false; }
      }
      else {
        if(!oprUrHTpoH(it.value().value(type_ur),&ur))  {  return false; }
      }
    }
    if(type_ur == UR_P){
      if ( UR_u == type_par || UR_v == type_par ) {
        if(!oprUrWindPoP(it.value().value(type_ur),&ur)){  return false; }
      }
      else {
        if(!oprUrHTpoP(it.value().value(type_ur),&ur))  {  return false; }
      }
    }
    if( (type_ur == UR_H && ph1 < ur.value(type_ur) && ur.value(type_ur) < ph2) ||
        (type_ur == UR_P && ph1 > ur.value(type_ur) && ur.value(type_ur) > ph2) ){
      vsX.append(ur.value(type_ur));
      vsY.append(ur.value(type_par));
    }
    if(keyType_ == UR_H) it++;
  }

  //заполнение массивов значениями на уровне ph2
  vsX.append(ur2.value(type_ur));
  vsY.append(ur2.value(type_par));

  //нахождение среднего значения (справедливо для составляющих среднего ветра u, v)
  if(!MnMath::integrTrapec(vsY, vsX, sr_par)) {  return false; } //TODO уточнить эту функцию в новом репозитории
  (*sr_par) /= (ph2 - ph1);

  return true;
}


bool Zond::oprIntLivnOrl(float *intens)
{
  Uroven ur;
  float  w_konv, p1, p2, t_sr, p_sr, q_850;

  *intens = 0.;
  if( !isData_ ) return false;

  if( !getUrz(&ur) ) return false;
  if( !ur.isGood(UR_P) || !ur.isGood((UR_T)) ) return false;
  p1 = ur.value(UR_P);

  if( !getTropo(&ur) ) return false;
  if( !ur.isGood(UR_P) || !ur.isGood((UR_T)) ) return false;
  p2 = ur.value(UR_P);

  if(p1 < p2) return false;

  if( !oprUrHTpoP(850.,&ur) )  return false;
  if( !ur.isGood(UR_T) || !ur.isGood(UR_Td) ) return false;
  if( !oprSPoTP(850.,ur.value(UR_Td),&q_850) ) return false;

  if( !oprSrParamPoPH(UR_T, UR_P, p1, p2, &t_sr) ) return false;
  if( !oprSrKonvSkor(p1, p2, &w_konv) ) return false;

  p_sr=(p1 + p2) * 0.5f;
  (*intens) = p_sr / (t_sr + kKelvin) / Rc * q_850 * w_konv * 3600.f;
  if((*intens) < 5.f)  (*intens) = 0.;
  return true;
}


int Zond::oprZnakEnergy(double p)
{
  float val;
  double p_nach;

  Uroven ur;
  if(!getPprizInvHi(&ur)){
    if(!getUrz( &ur)){
        warning_log << QObject::tr("Нет данных о давлениии у поверхности за %1 (%2) ")
                              .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"))
                              .arg(stIndex_);
      return 0;
    }

  }
  if(!ur.isGood(UR_T) ||!ur.isGood(UR_Td) ||!ur.isGood(UR_P) ){
    return 0;
  }

  p_nach = ur.value(UR_P);

  if(p > p_nach) p = p_nach - 5.;
  if(!oprTsost(p, &val)) return 0;
  if(!oprUrHTpoP(p, &ur)) return 0;
  if(MnMath::isEqual( ur.value(UR_T),val)) return 0; //TODO
  if(ur.value(UR_T) < val) return 1;  //TODO
  else return -1;
}



//Слои инверсии и изотермии
bool Zond::getSloiInver(QVector<InvProp> *layers)
{
  int Sign = 1, LastSign = 1;
  QVector<float>T;//Массив температур
  QVector<float>P;//Массив давлений
  QVector<float>H;//Массив высот

  //    bool u_zemli=false;

  QMap <Level, Uroven>::iterator it;
  Uroven ur;

  for (it = urList_.begin(); it != urList_.end(); it++) {
    if (!oprUrHTpoP(it.key(), &ur)) {
      ur = it.value();
    }
    if (!ur.isGood(UR_P) || !ur.isGood(UR_H) || !ur.isGood(UR_T)) {
      continue;
    }

    T.append(ur.value(UR_T)); //упорядочение по возрастанию P (сверху вниз)
    P.append(ur.value(UR_P));
    H.append(ur.value(UR_H));
  }
  if (T.count() == 0) {
    return false;
  }

  InvProp vs_inv;
  double dTdP;
  double dT=0.;
  for (int i=0; i<T.count()-1; i++) { //поиск по возрастанию P (сверху вниз)
    dT = T[i+1]-T[i];
    dTdP = dT/(P[i+1]-P[i]);
    /////////////////////////////////////*верхняя граница*/////////////////////////////////////////////////////////
    if (dTdP <= 0.) { //верхняя граница
      Sign = -1;
      if (Sign != LastSign) {
        vs_inv.h_hi = H[i];
        vs_inv.T_hi = T[i];
        vs_inv.sign = Sign;
        vs_inv.invType = (qFuzzyCompare(dT + 1., 1.))? InvProp::IZOTERMIA : InvProp::INVERSIA;
        //                if (i==0) u_zemli=true;
      }
      if (i == T.count()-2) {//у земли нижняя граница - самая нижняя точка
        vs_inv.sign = 1;
        vs_inv.h_lo = H[i+1];
        vs_inv.T_lo = T[i+1];
        vs_inv.dT = vs_inv.T_hi - vs_inv.T_lo;
        layers->append(vs_inv);
      }
    }
    ////////////////////////////////*нижняя граница*////////////////////////////////////////////////////
    else {//нижняя граница
      Sign = 1;
      if (Sign != LastSign) {
        vs_inv.h_lo = H[i];
        vs_inv.T_lo = T[i];
        vs_inv.sign = Sign;
        vs_inv.dT = vs_inv.T_hi - vs_inv.T_lo;
        layers->append(vs_inv);
      }
    }
    LastSign=Sign;
  }
  return true;
}

//Средний ветер в слоях
bool Zond::getSrednWinds(QVector<WindAverage> *winds )
{
  WindAverage vs;
  int h_prev=0;

  QMap <Level, Uroven>::iterator it;
  Uroven ur;

  for (it = urList_.begin(); it != urList_.end(); it++) {
    //  if (!(it.value().isGood(UR_u) && it.value().isGood(UR_v) && it.value().isGood(UR_H))) {
    if(!oprUrWindPoP(it.key(), &ur)) {
      ur = it.value();
    }
    if (!(it.value().isGood(UR_u) && it.value().isGood(UR_v) && it.value().isGood(UR_H))) {
      continue;
    }
    vs.h_lo = h_prev;
    vs.h_hi = ur.value(UR_H);
    vs.u = ur.value(UR_u);
    vs.v = ur.value(UR_v);
    winds->append(vs);
    h_prev = int(vs.h_hi);
  }

  if (winds->count() < 2) {
    return false;
  }

  for (int i = 1; i < winds->count(); ++i) {
    (*winds)[i].u = (winds->at(i-1).u + winds->at(i).u)*0.5; // * 0.005; //почему умножение на 0.005?? Откуда множитель, это же сумма векторов U на двух соседних уровнях
    (*winds)[i].v = (winds->at(i-1).v + winds->at(i).v)*0.5; // * 0.005;
  }
  return true;
}



/**
 * @brief возвращает значение относительной плотности воздуха
 * на высоте h
 * @param h высота, м
 * @param p давление у пов. Земли, Па (если не указано, берем из зонда)
 * @param t температура у пов. Земли, C (если не указано, берем из зонда)
 * @return double
 **/
double Zond::dp_func( float h, float p, float t )
{
  if((p < 0.f || t < 0.f) && !isData_){
    return -1.;
  }
  if(p < 0 && !getPz(&p)){
    return -1.;
  }
  if(p < 0 && !getTz(&t)){
    return -1.;
  }

  t +=kKelvin;;

  float M_R = 0.0289644f / 8.31447f;
  float ro_z = M_R*p/t;

  if(!p_tPoH(h,&p,&t)) return -1;

  float ro_h = M_R*p/t;

  return ro_h/ro_z;
}

bool Zond::oprNoObledCloud(double P) const { //для поиска уровня без обледенения методом половинного деления
  Uroven ur;
  if(!oprUrHTpoP(P, &ur)) {
    return true; //обледенения нет
  }
  return getNoObledCloud(ur.value(UR_T), ur.value(UR_D));
}

//! Поиск точки изменения значения (скачка) логической функции func(x) на участке [a,b] методом двойного сечения
/*! можно использовать в том числе для поиска экстремумов при соответствующем виде функции func(x)
  \param a   начало отрезка
  \param b   конец отрезка
  \param e   заданная точность
  \param noNull возвращаемое значение ошибки true если нулей у функции нет или их более 1 на отрезке
  \return    точка изменения значения (скачка) функции func(x)
*/
double Zond::bisectBorderObledCloud( double a, double b, double e, bool* noNull ) const
{
  double res = 0.5*(a + b);
  *noNull = false;

  if( oprNoObledCloud(a) == oprNoObledCloud(b)){
    *noNull = true;
    return res;
  }

  if( oprNoObledCloud(res) == oprNoObledCloud(a)) {
    a = res;
  }
  else {
    b = res;
  }

  return (fabs(b-a) < e)?  0.5*(a + b) : bisectBorderObledCloud(a, b, e, noNull );
}

}
