#include "tdataprocessor.h"
#include "meteo/commons/zond/diagn_func.h"

#include <qscriptvalue.h>
#include <qtimezone.h>
#include <commons/obanal/haosmain.h>

enum ParamType{
  Pressure     = 0,
  Height       = 1,
  Temperature  = 2,
  DewPoint     = 3,
  Inversion    = 4,
  WindVelocity = 5,
  WindSpeed = 6,     //скорость ветра
  WindDirection = 7  //направление ветра
};

//для работы с getPrizInvData
enum ParamInver{
  h_lo     = 0,
  h_hi     = 1,
  T_lo     = 2,
  T_hi     = 3,
  w_min    = 4
};


//!ПНС - потенциально неустойчивый слой
enum CalcPNSParamType{
  kPv      = 0, //!< Верхняя граница, ГПа
  kPn      = 1, //!< Нижняя граница, ГПа
  kTpv     = 2, //!< Температура псевдопотенциальная на нижней границе ПНС, С
  kTpn     = 3, //!< Температура псевдопотенциальная на верхней границе ПНС, С
  kTv      = 4, //!< Температура на нижней границе ПНС, С
  kTn      = 5  //!< Температура на нижней границе ПНС, С
};



bool TDataProcessor::resetData(){
 //debug_log<<" resetData()";
//  dataProvider_->getAeroData();
 /*meteo::surf::OneZondValueOnStation data;
 dataSource src =   cur_station_src;// брать данные по текущей станции
 // near_station_src = 1,   // брать данные по ближайшей станции
 int dist;
 StationData stData;
  dataProvider_->getZond(st)->clear();
  */
 //if(!_dataProvider->getAeroData(src, &stData,&dist, &data)) return false;
 //if(!dataProvider_->getZond(st)->setData(data)) return false;
 return true;
}

double TDataProcessor::getValue(const StationData &ast,int level, int typelev, const QString & descr,int srok){
  //float resval = kBadValue;
  QMap<int, float> mreslist;
  QMultiMap<int, FMetaData> metalist;
  FMetaData md;
  md.station = ast;
  md.level = level;
  md.type_level = typelev;
  md.srok = srok;

  if(dataProvider_->getValue(md, descr, &metalist, &mreslist) && 0 < mreslist.size()) {  
    return mreslist.values().first();
  }
  
  return kBadValue;
}

float TDataProcessor::getHpoT(const StationData &st, double valT, int srok){
  if(nullptr == dataProvider_->getZond(st,srok)){
      return kBadValue;
    }
    //ValueType urType, double value,
    QList<float> h;
    dataProvider_->getZond(st,srok)->oprHPoParam(zond::UR_T,valT,&h);
    if(1 > h.count() ){
      return kBadValue;
    }
    return h.at(0);

  return kBadValue;
}


float TDataProcessor::getZondValuePoDescr(const StationData &st,int lev_type, int level, int descr){
  if(nullptr == dataProvider_->getZond(st)){
      return kBadValue;
    }
  return dataProvider_->getZond(st)->valueDescr( lev_type,  level,  descr);
}

float TDataProcessor::getZondValuePoUrType(const StationData &st,int lev_type, int level, int type){

  if(nullptr != dataProvider_->getZond(st))
    return dataProvider_->getZond(st)->value( lev_type,  level,  static_cast< zond::ValueType>(type));
  else return kBadValue;
}

float TDataProcessor::getUr(const StationData &st,int level, int type) //!< Возвращает данные на заданном уровне давления
{
  if(nullptr != dataProvider_->getZond(st))
    return dataProvider_->getZond(st)->value(100, level,  static_cast< zond::ValueType>(type));
  else return kBadValue;

}

float TDataProcessor::getDataFromHeight(const StationData &st,int H,int t,const QScriptValue& urValueType)
{
 // debug_log<<st.coord.toString();
  if(nullptr == dataProvider_->getZond(st,t))
  {
    return kBadValue;
  }

  zond::Uroven ur;
//   dataProvider_->getZond(st,t)->print();
  //  if (false ==dataProvider_->getZond(st)->getUrPoH(H,&ur))
  if(0 == H){
    if (false == dataProvider_->getZond(st,t)->getUrz(&ur))
    {
      return kBadValue;
    }
  }else {
    if (false == dataProvider_->getZond(st,t)->getUrPoH(H,&ur))
    {
     // ur.print();
      return kBadValue;
    }
  }


  switch(static_cast<ParamType>(urValueType.toInt32())){
    case Temperature:    return ur.isGood(zond::UR_T)?  ur.value(zond::UR_T): kBadValue;
    case DewPoint:       return ur.isGood(zond::UR_Td)?  ur.value(zond::UR_Td): kBadValue;
    case WindSpeed:      return ur.isGood(zond::UR_ff)?  ur.value(zond::UR_ff): kBadValue;
    case WindDirection:  return ur.isGood(zond::UR_dd)?  ur.value(zond::UR_dd): kBadValue;
    case Pressure:  return ur.isGood(zond::UR_P)?  ur.value(zond::UR_P): kBadValue;

    default : return kBadValue;
  }
  //  trc;
  return kBadValue;

}

float TDataProcessor::getDataFromP(const StationData &st,int P,int t,const QScriptValue& urValueType)
{
  if(nullptr == dataProvider_->getZond(st,t))
  {
    return kBadValue;
  }

  zond::Uroven ur;
  // dataProvider_->getZond(st,t)->print();
  //  if (false ==dataProvider_->getZond(st)->getUrPoH(H,&ur))
  if (false == dataProvider_->getZond(st,t)->getUrPoP(P,&ur))
  {
    //    trc;
    return kBadValue;
  }


  switch(static_cast<ParamType>(urValueType.toInt32())){
    case Temperature:    return ur.isGood(zond::UR_T)?  ur.value(zond::UR_T): kBadValue;
    case DewPoint:       return ur.isGood(zond::UR_Td)?  ur.value(zond::UR_Td): kBadValue;
    case WindSpeed:      return ur.isGood(zond::UR_ff)?  ur.value(zond::UR_ff): kBadValue;
    case WindDirection:  return ur.isGood(zond::UR_dd)?  ur.value(zond::UR_dd): kBadValue;

    default : return kBadValue;
  }
  //  trc;
  return kBadValue;

}


  float TDataProcessor::calcPNS(const StationData &st,const QScriptValue &paramType)
{
  float val = kBadValue;

  if (true == paramType.isNull()) { return false; }

  double dTp = 0.0,Tpn = 0.0,Th0 = 0.0,Tph0 = 0.0,Th1 = 0.0,Tph1 = 0.0;
  double P = 910., Ph0 = 0.0, Ph1 = 0.0;
  float tpot;
  bool pokNal = false;
  do
  {
    P=P-10.0;
    if(P<=300.0) return false;
    float Td = getUr(st,P, zond::UR_Td);
    float T = getUr(st,P, zond::UR_T);
    if (MnMath::isEqual( kBadValue, Td)) { /*debug_log << "Td" << Td << descrTd.toString();*/ return val; }
    if (MnMath::isEqual(kBadValue, T)) { /*debug_log << "T" << T << descrT.toString();*/ return val; }
    if(zond::oprTPot(P,Td,&tpot))
    {
      dTp=tpot-Tpn;
      Tpn=tpot;
      Ph0=P;Th0=T;Tph0=tpot;
      if(dTp<0)
      {
        Ph0=P;Th0=T;Tph0=tpot;pokNal=true;
      }
    }
  }
  while(dTp>=0);

  if(!pokNal)
  {
    return val;
  }
  do
  {
    P=P-10.;
    float Td = getUr(st,P, zond::UR_Td);
    float T = getUr(st,P, zond::UR_T);
    if (MnMath::isEqual(kBadValue, Td)) {/* debug_log << "Td";*/ return val; }
    if (MnMath::isEqual(kBadValue, T)) { /*debug_log << "T";*/ return val; }
    if(zond::oprTPot(P,Td,&tpot))
    {
     dTp=tpot-Tpn; Tpn=tpot;
     Ph1=P;Th1=T;Tph1=tpot;
     if(P<=300.) break;
    }
  } while(dTp<0);

  switch(static_cast<CalcPNSParamType>(paramType.toInt32())){
    case kPv:    return Ph1;
    case kPn:    return Ph0;
    case kTn:    return Th0;
    case kTpn:   return Tph0;
    case kTv:    return Th1;
    case kTpv:   return Tph1;
    default :    return kBadValue;
  }
  return kBadValue;
}


//!getUrOsobPoTemp - метод, возвращающий данные (T, Td) в особых точках по температуре
//!N - номер особой точки,  urValueType - тип данных, запрашиваемый с этого уровня (T - это "2"; Td - это "3")
float TDataProcessor::getUrOsobPoTemp(const StationData &st,int N, const QScriptValue& urValueType)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  const QList<zond::Uroven> &urList = dataProvider_->getZond(st)->getUrOsobyT();
  if( urList.isEmpty() ){
    return kBadValue;
  }
  if( N > (urList.length()-1) || N < 0){
    return kBadValue;
  }
  switch(static_cast<ParamType>(urValueType.toInt32())){
    case Height:    return urList.at(N).isGood(zond::UR_H)?  urList.at(N).value(zond::UR_H): kBadValue;
    case Temperature: return urList.at(N).isGood(zond::UR_T) ?  urList.at(N).value(zond::UR_T) : kBadValue;
    case DewPoint:    return urList.at(N).isGood(zond::UR_Td)?  urList.at(N).value(zond::UR_Td): kBadValue;
    default : return kBadValue;
  }
  return kBadValue;
}

float TDataProcessor::getPrizInvData(const StationData &st,const QScriptValue &urValueType)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  zond::Uroven ur;
  dataProvider_->getZond(st)->getPprizInvHi(&ur);

  switch(static_cast<ParamType>(urValueType.toInt32())){
    case Height:    return ur.isGood(zond::UR_H)?  ur.value(zond::UR_H): kBadValue;
    case Temperature:    return ur.isGood(zond::UR_T)?  ur.value(zond::UR_T): kBadValue;
    case DewPoint:    return ur.isGood(zond::UR_Td)?  ur.value(zond::UR_Td): kBadValue;
    default : return kBadValue;
  }
  return kBadValue;
}

float TDataProcessor::getH_obled(const StationData& st,int t ) //!< Высота нижней границы обледенения,м
{
  if(nullptr == dataProvider_->getZond(st,t)) return kBadValue;
  QList <float > p_obled;
  dataProvider_->getZond(st,t)->oprGranObled(&p_obled);
  if(p_obled.size() < 1 ) return kBadValue;
  qSort(p_obled);
  float p = p_obled.last();
  zond::Uroven ur;
  dataProvider_->getZond(st,t)->getUrPoP(p,&ur);
  if(ur.quality(zond::UR_H) >= control::MISTAKEN){
    return kBadValue;
  }

  return ur.value(zond::UR_H);

}

float TDataProcessor::getNalOblak(const StationData& st,int t , double h){ //!< Наличие облачности на высоте
  zond::Zond* znd = dataProvider_->getZond(st,t);
  if(nullptr == znd) return kBadValue;
  QList<float> p_oblak;
  if(!znd->oprGranOblak(&p_oblak)|| p_oblak.size() < 1 ){
    return 0.;
  }
  zond::Uroven ur;
  if(!znd->getUrPoH( h,&ur) || ur.quality(zond::UR_P) >=control::MISTAKEN ){
    return kBadValue;
  }
  float p = ur.value(zond::UR_P);

  for (int i=0, sz = p_oblak.size()-1; i < sz; ++i){
    if(p_oblak.at(i) >= p && p_oblak.at(i+1) <= p  ){
      return 1;
    }
  }
  return 0.;

}


float TDataProcessor::getT_obled(const StationData& st,int t ) //!< Температура воздуха на нижней границе обледенения,°С
{
  if(nullptr == dataProvider_->getZond(st,t)) return kBadValue;
  QList <float > p_obled;
  dataProvider_->getZond(st,t)->oprGranObled(&p_obled);
  if(p_obled.size() < 1 ) return kBadValue;
  qSort(p_obled);
  float p = p_obled.last();
  zond::Uroven ur;
  dataProvider_->getZond(st,t)->getUrPoP(p,&ur);
  if(ur.quality(zond::UR_T) >= control::MISTAKEN){
    return kBadValue;
  }

  return ur.value(zond::UR_T);
}



float TDataProcessor::getInvData(const StationData &st,const QScriptValue &urValueType)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  QVector<zond::InvProp> l;
  dataProvider_->getZond(st)->getSloiInver(&l);


  switch(static_cast<ParamInver>(urValueType.toInt32())){
    case h_lo:    return (l.isEmpty()==false)?  l.value(l.size()-1).h_lo: kBadValue;
    case h_hi:    return (l.isEmpty()==false)?  l.value(l.size()-1).h_hi: kBadValue;
    case T_lo:    return (l.isEmpty()==false)?  l.value(l.size()-1).T_lo: kBadValue;
    case T_hi:    return (l.isEmpty()==false)?  l.value(l.size()-1).T_hi: kBadValue;
    case w_min:   return (l.isEmpty()==false)?  l.value(l.size()-1).w_min: kBadValue;
    default : return kBadValue;
  }
  return kBadValue;
}



  float TDataProcessor::getUrzDataAdv(const StationData &st,const QScriptValue &dataType)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  zond::Uroven urZem;
  dataProvider_->getZond(st)->getUrz(&urZem);

  if(!dataProvider_->getZond(st)->getUrz(&urZem)) return kBadValue;
  switch(static_cast<ParamType>(dataType.toInt32())){
    case Temperature: return urZem.isGood(zond::UR_T) ?  urZem.value(zond::UR_T) : kBadValue;
    case DewPoint:    return urZem.isGood(zond::UR_Td)?  urZem.value(zond::UR_Td): kBadValue;
    case Pressure:    return urZem.isGood(zond::UR_P)?  urZem.value(zond::UR_P): kBadValue;
    case Height:      return urZem.isGood(zond::UR_H)?  urZem.value(zond::UR_H): kBadValue;

    default : return kBadValue;
  }
  return kBadValue;

}

float TDataProcessor::oprTsmUrzem0(const StationData &st)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  float val;
  QList<float> tlist;
  float p_z;
  zond::Uroven ur;

  if (dataProvider_->getZond(st)->getPprizInvHi(&ur) && ur.isGood(zond::UR_P)) {
    p_z = ur.value(zond::UR_P);
  } else {
    if (false ==dataProvider_->getZond(st)->getPz(&p_z)) {
      return kBadValue;
    }
  }

  if (false ==dataProvider_->getZond(st)->oprHPoParam(zond::UR_T,0.,&tlist)) {
    return kBadValue;
  }

/*  if (false == dataProvider_->getZond(st)->oprHPoParam(zond::UR_Td,0.,tdlist)) {
    return kBadValue;
  }*/
  if (tlist.size() < 1) {}
  float Ht = tlist[0];

  if (false ==dataProvider_->getZond(st)->getUrPoH(Ht, &ur)|| !ur.isGood(zond::UR_Td)) {return kBadValue;}
  float t_nach = (0. + ur.value(zond::UR_Td))*.5;

  float Psm = ur.value(zond::UR_P);
//  return (float) Psm;
  float p_nach = Psm;

  zond::oprTsm(p_nach,p_z,t_nach,&val);
  return val;
}

float TDataProcessor::opredHPoT(const StationData &st,double t,int srok)
{
  return getHpoT(st,t, srok);
}

float TDataProcessor::advParam(const StationData &st,float level, int type)
{
 if(nullptr == dataProvider_->getZond(st)) return kBadValue;
 return dataProvider_->getZond(st)->value(100, level,  static_cast< zond::ValueType>(type));
}

double TDataProcessor::oprTPot(const QScriptValue &PVal, const QScriptValue &TdVal)
{
  float P = PVal.toVariant().toFloat();
  float Td = TdVal.toVariant().toFloat();
  float tpot;
  if (!zond::oprTPot(P,Td,&tpot)) { return (double) kBadValue; }
  return tpot;
}

double TDataProcessor::oprTsost(const StationData &st,const QScriptValue &PVal)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  float P = PVal.toVariant().toFloat();
  float tsost;
  if (!dataProvider_->getZond(st)->oprTsost(P,&tsost)) { return (double) kBadValue; }
  return tsost;
}


double TDataProcessor::oprTsostAdv(const StationData &st,const QScriptValue &PVal)
{

  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  float P = PVal.toVariant().toFloat();
  float tsost;
  if (!dataProvider_->getZond(st)->oprTsost(P,&tsost)) { return (double) kBadValue; }
  return tsost;
}

void TDataProcessor::advData(const StationData & st)
{
  dataProvider_->getZond(st);
}

float TDataProcessor::oprPsostRaznAllFastDelta(const StationData & st)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  float p, delta;
  if (!dataProvider_->getZond(st)->oprPsostRaznAllFast(&p,&delta)) { return kBadValue; }
  return delta;
}
float TDataProcessor::oprPsostRaznAllFastP(const StationData & st)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;

  float p, delta;
  if (!dataProvider_->getZond(st)->oprPsostRaznAllFast(&p,&delta)) { return (double) kBadValue; }
  return  p;
}

float TDataProcessor::oprHmaxVFastH(const StationData & st, int srok) //!<Высота максимального ветра
{
  if(nullptr == dataProvider_->getZond(st,srok)) return kBadValue;
  float h, v;
  if (!dataProvider_->getZond(st,srok)->oprHmaxVFast(&h,&v)) { return (double) kBadValue; }
  return  h;
}

float TDataProcessor::oprHmaxWind(const StationData & st, int min_h, int max_h, int srok) //!<Высота максимального ветра
{
  if(nullptr == dataProvider_->getZond(st,srok)) return kBadValue;
  float h, v;
  if (!dataProvider_->getZond(st,srok)->oprHmaxV(&h,&v,min_h,max_h )) { return (double) kBadValue; }
  return  h;
}

float TDataProcessor::oprHSloyWind(const StationData & st,double max_v,  int min_h, int max_h, int srok) //!<слой максимального ветра
{
  if(nullptr == dataProvider_->getZond(st,srok)) return kBadValue;
  float h;
  if (!dataProvider_->getZond(st,srok)->oprHSloymaxV(&h,max_v,min_h,max_h )) { return (double) kBadValue; }
  return  h;
}

float TDataProcessor::oprMaxWind(const StationData & st, int min_h, int max_h, int srok) //!<скорость максимального ветра
{
  if(nullptr == dataProvider_->getZond(st,srok)) return kBadValue;
  float h, v;
  if (!dataProvider_->getZond(st,srok)->oprHmaxV(&h,&v,min_h,max_h )) { return (double) kBadValue; }
  return v;
}


float TDataProcessor::oprHmaxVFastV(const StationData & st, int srok) //!<Скорость максимального ветра
{
  if(nullptr == dataProvider_->getZond(st,srok)) return kBadValue;
  float h, v;
  if (!dataProvider_->getZond(st,srok)->oprHmaxVFast(&h,&v)) { return (double) kBadValue; }
  return v;
}

float TDataProcessor::oprHmaxVFastVprog(const StationData &st,int srok)
{
  if(nullptr == dataProvider_->getZond(st,srok)) return kBadValue;
  float h, v;
  if (!dataProvider_->getZond(st,srok)->oprHmaxVFast(&h,&v)) { return (double) kBadValue; }
  return v;
}

float TDataProcessor::oprHmaxVFastH1(const StationData & st) //!<Высота нижней границы слоя, где скорость ветра превышает 0.8*Vmax
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;
  float h1, h2;
  if (!dataProvider_->getZond(st)->oprH1Fast(&h1,&h2)) { return (double) kBadValue; }
  return  h1;
}

float TDataProcessor::oprHmaxVFastH2(const StationData & st) //!<Высота верхней границы слоя, где скорость ветра превышает 0.8*Vmax, км
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;
  float h1, h2;
  if (!dataProvider_->getZond(st)->oprH1Fast(&h1,&h2)) { return (double) kBadValue; }
  return  h2;
}

float TDataProcessor::oprIntLivnOrl(const StationData & st)
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;
  float intens;
  if (!dataProvider_->getZond(st)->oprIntLivnOrl(&intens)) { return (double) kBadValue; }
  return intens;
}

//! Определение средней скорости перемещения конвективных облаков, км/ч
float TDataProcessor::oprSrKonvSkor(const StationData &st,const QScriptValue &P1Val, const QScriptValue &P2Val )
{
  if(nullptr == dataProvider_->getZond(st)) return kBadValue;
  double p1 = P1Val.toVariant().toDouble();
  double p2 = P2Val.toVariant().toDouble();
  float w_konv;
  if (!dataProvider_->getZond(st)->oprSrKonvSkor(p1,p2,&w_konv)) { return (double) kBadValue; }
  return w_konv;
}


double TDataProcessor::pic( const QScriptValue &arr1, const QScriptValue &arr2, const QScriptValue &arr3, const QScriptValue &val1, const QScriptValue &val2)
{
  double v1 = val1.toVariant().toDouble();
  double v2 = val2.toVariant().toDouble();
  QVariantList arrX = arr1.toVariant().toList();
  QVariantList arrY = arr2.toVariant().toList();
  QVariantList arrZ = arr3.toVariant().toList();

  int kol_pointX = arrX.size();
  int kol_pointY = arrY.size();
  int kol_pointZ = arrZ.size();

  if ((kol_pointX != kol_pointY) || (kol_pointX != kol_pointZ) || (kol_pointY != kol_pointZ)) return kBadValue;

  int kol_point = kol_pointX;

  double x[kol_pointZ];
  double y[kol_pointZ];
  double z[kol_pointZ];

  for(int i=0; i< kol_point; ++i){
    x[i] = arrX.at(i).toDouble();
    y[i] = arrY.at(i).toDouble();
    z[i] = arrZ.at(i).toDouble();
  }
  float rezult;
//  debug_log << kol_point;
  int res = obanal::interpolHaosOnePoint(kol_point,x,y,z,v1,v2,&rezult);
  if (res != 0) { return kBadValue; }
  return rezult;
}

enum SunType{
  Sunrise    = 0,
  Sunset     = 1,
};

double TDataProcessor::calcDist(const meteo::GeoPoint& p1,const meteo::GeoPoint& p2){

  return p1.calcDistance(p2);
}

int TDataProcessor::offsetFromUtc(){
  QDateTime dt=QDateTime::currentDateTime();
 return dt.offsetFromUtc()/3600;
}


