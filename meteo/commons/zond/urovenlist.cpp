#include "urovenlist.h"
#include "diagn_func.h"

//#include <commons/meteo_data/tmeteodescr.h>

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/surface_service.pb.h>

//#include <map>
#include <qdebug.h>

namespace zond{
CloudDkr::CloudDkr(){
  float DkrOblak[14]={1.6f,1.6f,1.5f,1.5f,1.7f,1.8f,1.9f,2.0f,2.2f,2.4f,2.5f,2.8f,3.2f,4.0f};               //критические значения дефицитов
  float PkrOblak[14]={1050.,950.,900.,850.,800.,750.,700.,650.,600.,550.,500.,400.,300.,200.};//уровни с критическими значениями дефицитов
  for(int i = 0; i < 14; ++i){
      DkrP.insert(PkrOblak[i], DkrOblak[i]);
    }
}

bool CloudDkr::isCloud(float p, float d ){
  QMap<float, float>::const_iterator it;
  it = DkrP.upperBound(p);
  float Dup = it.value();
  float Pup = it.key();

  --it;
  float Dlow = it.value();
  float Hlow = it.key();
  float d_kr= 100.;
  if(MnMath::linInterpolY(Hlow, Dlow, Pup, Dup, p, &d_kr)){
      if( d_kr > d ){
          return true;
        }
    }
  return false;
}

bool CloudDkr::getCloudDeficit(float H, float *Dkr) const {
  if(DkrH.isEmpty()) return false;
  if(DkrH.contains(H)){
      *Dkr = DkrH.value(H);
      return true;
    }

  QMap<float, float>::const_iterator it;

  it = DkrH.upperBound(H);
  if ( it == DkrH.end() || it == DkrH.begin()-1 ) {
      //error_log << "INVALID ITERATOR";
      return false;
    }
  float Dup = it.value();
  float Hup = it.key();

  --it;
  if ( it == DkrH.end() || it == DkrH.begin()-1 ) {
      //error_log << "INVALID ITERATOR";
      return false;
    }
  float Dlow = it.value();
  float Hlow = it.key();

  if(!MnMath::linInterpolY(Hlow, Dlow, Hup, Dup, H, Dkr)){
      return false;
    }
  return true;
}



UrovenList::UrovenList()
{
  keyType_ = LAST_UR;
  isData_ = false;
  // _d = TMeteoDescriptor::instance(); //не нужна уже?
}

UrovenList::~UrovenList() {
  clear();
}

void UrovenList::clear(){
  keyType_ = LAST_UR;
  isData_ = false;
  urList_.clear();
  urZ_.reset();//!< данные у земли
  urZinv_.reset();//!< приземная инверсия
  urMaxWind_.reset();//!< данные на уровне максимального ветра
  //  urTropo_.reset();//!< данные на уровне тропопаузы
  urTropo_.clear();//!< данные на уровне тропопаузы
  //cloud_.clear();
}

bool UrovenList::contains(float val, int level_type)const{
  return urList_.contains(Level(val))&&(level_type == urList_.value(Level(val)).levelType());
}


void UrovenList::insert(float level, const Uroven &val){
  urList_.insert(Level(level),val);
}

void UrovenList::insertMulti(float level, const Uroven &val){
  urList_.insertMulti(Level(level),val);
}



int UrovenList::size(){
  return urList_.size();
}

bool UrovenList::setData(const meteo::surf::OneZondValueOnStation& data) //TODO
{
  Uroven ur_;
  for(int i = 0; i < data.ur_size(); i++){
      ur_.reset();
      fromProtoUroven(data.ur(i), &ur_);
      if (!contains(ur_.value(UR_P),ur_.levelType()) ||
          ur_.levelType() != meteodescr::kIsobarLevel) {
          insert(ur_.value(UR_P),ur_);
        }
    }
  if(0 == size()){
      return false;
    }
  isData_ = true;
  keyType_ = UR_P;
  //print();

  preobr();
  return true;
}

void UrovenList::preobr()
{
  preobr(false);
}
void UrovenList::preobr(bool onlyAbsent)
{
  oprH();
  oprD();
  DF2UV();
  UV2DF(onlyAbsent);
  interpolDF();
  checkOsobyPoint();
  resetlevelType();
}

void UrovenList::resetlevelType(){
  QMap <Level, Uroven>::iterator it = urList_.begin();
  for(;it != urList_.end(); ++it){
      if(!it.value().isGood(UR_P)) continue;
      float p =it.value().value(UR_P);
      //if(MnMath::isEqual(p, urZ_.value(UR_P) )) {
      //    it.value().setLevelType(meteodescr::kSurfaceLevel);
      //  }
      for(int i = 0; i < urTropo_.size();++i){
          float tp =urTropo_.at(i).value(UR_P);
          if(MnMath::isEqual(p, tp )) {
              it.value().setLevelType(meteodescr::kTropoLevel);
            }
        }
    }

}


bool UrovenList::setDataNoPreobr(const meteo::surf::OneZondValueOnStation& data) //TODO
{
  Uroven ur_;
  for(int i = 0; i < data.ur_size(); i++){
      ur_.reset();
      fromProtoUroven(data.ur(i), &ur_);
      if(meteodescr::kMaxWindLevel == ur_.levelType()){
          urMaxWind_= ur_;
          continue;
        }
      if(meteodescr::kTropoLevel == ur_.levelType()){
          urTropo_.append(ur_);
          continue;
        }
      if(meteodescr::kSurfaceLevel == ur_.levelType()){
          urZ_ = ur_;
          continue;
        }
      insert(ur_.value(UR_P),ur_);
    }
  if(0 == size()){
      return false;
    }
  isData_ = true;
  keyType_ = UR_P;
  return true;
}

bool UrovenList::setData(const TMeteoData& adata)
{
  setDataNoPreobr( adata);

  DF2UV();

  isData_ = false;

  if(0 == size() ){
      return setDataPoH(adata);
    }

  isData_ = true;
  keyType_ = UR_P;

  preobr();
  return true;
}

void UrovenList::setData(int level_type, float level,const Uroven &ur_){
  keyType_ = UR_P;
//    if(2 > ur_.countValid() ){ //TODO по идее этого случая быть не должно...
//        return;
//      }
  int ilevel = level;
  switch (level_type) {
  case meteodescr::kTropoLevel: //TODO
    urTropo_.append(ur_);
    //insertMulti(level,ur_);
    break;
  case meteodescr::kMaxWindLevel:
    urMaxWind_=ur_;
    //insertMulti(level,ur_);
    break;
  case meteodescr::kSurfaceLevel:
    urZ_ = ur_;
    if (control::NO_OBSERVE == urZ_.quality(UR_H)){
      urZ_.set( UR_H,level+coord_.alt(), control::NO_CONTROL);
      //urZ_.setLevelTypeInt(level_type);
    } //insert(level,ur_);
    break;
  case meteodescr::kIsobarLevel:
    insertMulti(level,ur_);
    break;
  case meteodescr::kHeightLevel:
        ilevel = MnMath::ftoi_norm(level+coord_.alt());
  case meteodescr::kGeopotentialLevel:
  case meteodescr::kMeanSeaLevel:
        urHList_[ilevel] = ur_;
       // urHList_[ilevel].setLevelTypeInt(level_type);
      if (control::NO_OBSERVE == urHList_[ilevel].quality(UR_H)){
          urHList_[ilevel].set( UR_H,ilevel, control::NO_CONTROL);
      }   // level = -9999;
   // insertMulti(level,ur_);
    break;
 default:
    return; //чтоб не попали данные в слое
  }
  //debug_log << "set" << level << ur_;
  isData_ = true;
}

bool UrovenList::getPNaURPoMeanSeaLevel(float *p,const Uroven& urH){
  QList<Uroven> aur;

  if(false == getUrPoLevelType(meteodescr::kMeanSeaLevel,&aur)||
    0==aur.size()){
    return false;
  }
  if(control::NO_OBSERVE == urH.quality(UR_T)&&
    control::NO_OBSERVE == aur.at(0).quality(UR_T)){
    return false;
  }
  float h = coord_.alt();
  *p = aur.at(0).value(UR_P);
  float t = urH.value(UR_T) + h*0.005;
  if(control::NO_OBSERVE != aur.at(0).quality(UR_T)){
     t = aur.at(0).value(UR_T);
  }
  t+=kKelvin;
  oprPt(p,&t,h,0);
  debug_log <<" H=  "<<h<<p<<aur.at(0).value(UR_P);
  return true;
}

void UrovenList::restoreUrList(){
  //QMap <Level, Uroven> list_vs;
  QMap <int, zond::Uroven>::iterator it = urHList_.begin();
  for(;it != urHList_.end(); ++it) {
     if(control::NO_OBSERVE != it.value().quality(UR_P)){
       urList_.insertMulti(it.value().value(UR_P),it.value());
       isData_=true;
     }else {
       float p;
       if(true == getPNaURPoMeanSeaLevel(&p,it.value())){
         it.value().set(UR_P,p,control::ABSENT_CORRECTED);
         urList_.insertMulti(it.value().value(UR_P),it.value());
       } else {
         urList_.insertMulti(9999,it.value());//TODO
       }
     }
   }

  if(control::NO_OBSERVE != urZ_.quality(UR_P)){
     urList_.insert(urZ_.value(UR_P),urZ_);
     } else {
       float p;
       if(true == getPNaURPoMeanSeaLevel(&p,urZ_)){
         urZ_.set(UR_P,p,control::ABSENT_CORRECTED);
         urList_.insertMulti(urZ_.value(UR_P),urZ_);
       }
     }
  if(control::NO_OBSERVE != urMaxWind_.quality(UR_P)){
     urList_.insert(urMaxWind_.value(UR_P),urMaxWind_);
     }
  for(int i=0;i< urTropo_.size();++i){
    if( control::NO_OBSERVE != urTropo_.at(i).quality(UR_P)){
       urList_.insertMulti(urTropo_.at(i).value(UR_P),urTropo_.at(i));
       }
  }

}


void UrovenList::setValue(int level_type, float level,int descr,float value,int quality){
  int ilevel = 0;
  switch (level_type) {
    case meteodescr::kMaxWindLevel:
      return setMaxWindValue( descr, value, quality);
    case meteodescr::kTropoLevel:
      return setTropoValue( descr, value, quality);

    case meteodescr::kSurfaceLevel:
      if (control::NO_OBSERVE == urZ_.quality(UR_H)){
          urZ_.set( UR_H,level+coord_.alt(), control::NO_CONTROL);
          urZ_.setLevelTypeInt(level_type);
      }
      return setUrZValue(descr, value, quality);
    case meteodescr::kIsobarLevel:
      return setIsoBarValue(level_type, level, descr, value, quality);
    case meteodescr::kHeightLevel:
      ilevel = MnMath::ftoi_norm(level+coord_.alt());
    case meteodescr::kMeanSeaLevel:
      if (control::NO_OBSERVE == urHList_[ilevel].quality(UR_H)){
          urHList_[ilevel].set( UR_H,ilevel, control::NO_CONTROL);
          urHList_[ilevel].setLevelTypeInt(level_type);
      }
      return setUrHValue(ilevel, descr, value, quality);

   // case meteodescr::kMeanSeaLevel:
    default:
      return setHeightValue(level_type, level, descr, value, quality);
    }
}

void UrovenList::setTropoValue( int descr, float value, int quality)
{
  for(int i=0; i<urTropo_.size();++i ){
    if(control::NO_OBSERVE == urTropo_.at(i).quality(UrTypePoDescr(descr))){
      urTropo_[i].set(UrTypePoDescr(descr),value, quality);
      break;
    }
  }

}


void UrovenList::setUrHValue(int level, int descr, float value, int quality)
{
  urHList_[level].set(UrTypePoDescr(descr),value, quality);
}



void UrovenList::setUrZValue(int descr, float value, int quality)
{
  urZ_.set(UrTypePoDescr(descr),value, quality);
}

void UrovenList::setMaxWindValue( int descr, float value, int quality)
{
  urMaxWind_.set(UrTypePoDescr(descr),value, quality);
}


void UrovenList::setIsoBarValue(int level_type, float level, int descr, float value, int quality){
  if (!contains(level,level_type)){
    Uroven ur;
    ur.setLevelType(static_cast<meteodescr::LevelType>(level_type));
    if(meteodescr::kIsobarLevel == level_type){
        ur.set( UR_P,level, control::NO_CONTROL);
    }
    ur.set( UrTypePoDescr(descr),value, quality);
    setData(level_type, level,ur);
  } else{
    urList_[ Level(level)].set(UrTypePoDescr(descr),value, quality);
  }
}

void UrovenList::setHeightValue(int level_type, float level, int descr, float value, int quality){
  QMap <Level, Uroven>::iterator it = urList_.begin();
  for(;it != urList_.end(); ++it) {
    if( MnMath::isEqual(it.value().value(UR_H),level)){
      break;
    }
  }
  if (it == urList_.end()){
    Uroven ur;
    ur.setLevelType(static_cast<meteodescr::LevelType>(level_type));
    ur.set( UR_H,level, control::NO_CONTROL);
    ur.set( UrTypePoDescr(descr),value, quality);
    setData(level_type, -9999,ur);
  } else{
    it->set(UrTypePoDescr(descr),value, quality);
  }
}

bool UrovenList::setZDataNoPreobr(const TMeteoData& adata )
{
  const TMeteoData* md = adata.findChild(meteodescr::kSurfaceLevel);
  if(nullptr == md) return false;
  TMeteoParam p = md->getParam(10004);
  urZ_.set(UR_P, p.value(), p.quality());
  p = md->getParam(12101);
  urZ_.set(UR_T, p.value(), p.quality());
  p = md->getParam(12108);
  urZ_.set(UR_D, p.value(), p.quality());
  p = md->getParam(12103);
  urZ_.set(UR_Td, p.value(), p.quality());
  p = md->getParam(11001);
  urZ_.set(UR_dd, p.value(), p.quality());
  p = md->getParam(11002);
  urZ_.set(UR_ff, p.value(), p.quality());

  urZ_.to_uv();
  if(!urZ_.isGood(UR_Td))      { urZ_.to_Td();}
  else if(!urZ_.isGood(UR_D))  { urZ_.to_D(); }

  // md->printData();
  //облака
  const QMap<int, TMeteoParam> p_l = md->getParamList(20012);
  QList< int > pll = p_l.keys();
  for(int i=0;i< pll.count();++i){
      const TMeteoParam mp = p_l.value(pll.at(i));
      cloud_.setC(mp.value(),mp.quality());
    }

  p = md->getParam(20011);
  cloud_.set(Nh,p.value(),p.quality());
  p = md->getParam(20013);
  cloud_.set(h,p.value(),p.quality());
  urZ_.setLevelType(meteodescr::kSurfaceLevel);
  urList_.insert(urZ_.value(UR_P),urZ_);
  keyType_ = UR_P;
  isData_ = true;
  return true;
}


bool UrovenList::setDataNoPreobr(const TMeteoData& adata)
{
  //   setDataNoPreobrPoTypeLevel(adata,meteodescr::kMaxWindLevel ,UR_P);
  //  setDataNoPreobrPoTypeLevel(adata,meteodescr::kTropoLevel ,UR_P);
  //  setDataNoPreobrPoTypeLevel(adata,meteodescr::kIsobarLevel ,UR_P);
  int ades = descrPoUrType(UR_P);

  QList<const TMeteoData*> urlist = adata.findChildsConst(ades);

  int cnt = urlist.count();
  if(1 > cnt ) return false;
  for (int lvl = 0 ; lvl < cnt ; ++lvl) {
      const TMeteoData* md = urlist.at(lvl);
      if(nullptr == md) continue;
      meteodescr::LevelType lt = static_cast< meteodescr::LevelType > (md->getParam(10).value());
      if (meteodescr::kSurfaceLevel == lt) continue;
      Uroven ur;
      TMeteoParam p;
      for ( int urs =0; urs < LAST_UR; ++urs){
          ValueType at = static_cast<ValueType> (urs);
          p = md->getParam(descrPoUrType(at));
          ur.set(at, p.value(), p.quality());
        }
      ur.setLevelType(lt);

      if (!contains(ur.value(UR_P),lt) ||
          lt != meteodescr::kIsobarLevel) {
          insert(ur.value(UR_P),ur);
        }
    }
  setZDataNoPreobr(adata);

  if( 0 < cnt){
      isData_ = true;
      keyType_ = UR_P;
    }

  return true;
}

bool UrovenList::setDataPoH(const TMeteoData& adata)
{

  int ades = descrPoUrType(UR_H);
  QList<const TMeteoData*> urlist = adata.findChildsConst(ades);

  int cnt = urlist.count();
  for (int lvl = 0 ; lvl < cnt ; ++lvl) {
      const TMeteoData* md = urlist.at(lvl);
      if(nullptr == md) continue;
      Uroven ur;
      for ( int urs =0; urs < LAST_UR; ++urs){
          ValueType at = static_cast<ValueType> (urs);
          const TMeteoParam &p = md->getParam(descrPoUrType(at));
          ur.set(at, p.value(), p.quality());
        }
      ur.setLevelType(static_cast< meteodescr::LevelType > (md->getParam(10).value()));
      if (!contains(ur.value(UR_P),ur.levelType()) ||
          ur.levelType() != meteodescr::kIsobarLevel) {
          insert(ur.value(UR_P),ur);
        }
    }

  if(0 == size() ){
      return false;
    }
  isData_ = true;
  keyType_ = UR_H;
  return true;
}



void UrovenList::UV2DF(bool onlyAbsent)
{
  QMap <Level, Uroven>::iterator it = urList_.begin();
  for(;it != urList_.end(); ++it) {
      if(it.value().isGood( UR_u ) && it.value().isGood( UR_v )) {
          if (!onlyAbsent || !it.value().isGood( UR_dd ) || !it.value().isGood( UR_ff )) {
              it.value().to_ddff();
            }
        }
    }
}
void UrovenList::UV2DF()
{
  UV2DF(false);
}


void UrovenList::DF2UV(){
  QMap <Level, Uroven>::iterator it = urList_.begin();
  for(;it != urList_.end(); ++it){
      if(it.value().isGood( UR_dd ) && it.value().isGood( UR_ff)){
          it.value().to_uv();
        }
    }
}

float UrovenList::value(int lev_type, float level, ValueType urType, bool *res)
{
  if (LAST_UR == urType){*res = false; return BAD_METEO_ELEMENT_VAL;}
  Uroven ur;
  if(nullptr != res){*res = false;}

  if(100 == lev_type){
      if(! getUrPoP(level, &ur) ) return BAD_METEO_ELEMENT_VAL;
    }else {
      if(! getUrPoLevelType(lev_type, &ur) ) return BAD_METEO_ELEMENT_VAL;
    }

  if(nullptr != res){*res = ur.isGood(urType);}
  return ur.value(urType);
}

float UrovenList::valueDescr(int lev_type, float level, int descr, bool *res){
  ValueType urType = UrTypePoDescr(descr);
  return value( lev_type,  level,  urType, res);
}

void UrovenList::fromProtoUroven(const meteo::surf::Uroven &ur_, Uroven *res)
{
  if(ur_.has_p()){
      res->set(UR_P, ur_.p().value(), ur_.p().quality());
    }
  if(ur_.has_h()){
      res->set(UR_H, ur_.h().value(), ur_.h().quality());
    }
  if(ur_.has_t()){
      res->set(UR_T, ur_.t().value(), ur_.t().quality());
    }
  if(ur_.has_td()){
      res->set(UR_Td, ur_.td().value(), ur_.td().quality());
    }
  if(ur_.has_d()){
      res->set(UR_D, ur_.d().value(), ur_.d().quality());
    }
  if(ur_.has_dd()){
      res->set(UR_dd, ur_.dd().value(), ur_.dd().quality());
    }
  if(ur_.has_ff()){
      res->set(UR_ff, ur_.ff().value(), ur_.ff().quality());
    }
  if(ur_.has_u()){
      res->set(UR_u, ur_.u().value(), ur_.u().quality());
    }
  if(ur_.has_v()){
      res->set(UR_v, ur_.v().value(), ur_.v().quality());
    }
  if(ur_.has_level_type()){
      res->setLevelTypeInt(ur_.level_type());
    }
}

bool UrovenList::getUrz(Uroven *ur)
{
  if(!isData_) {
    return false;
  }
  bool ok = true;
  if(2 > urZ_.countValid()) {//FIXME
      ok = getUrPoLevelType(1, &urZ_);
    }
  *ur = urZ_;
  return ok;
}

bool UrovenList::getPz(float *p)
{
  *p = 0;
  Uroven ur;
  if(!getUrz(&ur)) return false;
  if(!ur.isGood(UR_P)) return false;
  *p = ur.value(UR_P);
  return true;
}

bool UrovenList::getTz(float *t)
{
  *t = 0;
  Uroven ur;
  if(!getUrz(&ur)) return false;
  if(!ur.isGood(UR_T)) return false;
  *t = ur.value(UR_T);
  return true;
}

bool UrovenList::getUr5001000(Uroven *ur)const
{
  if(!isData_) return false;
  Uroven ur1, ur2;
  if(!getUrPoP(1000., &ur1) || !getUrPoP(500., &ur2)){
      return false;
    }
  ValueType urType;
  ur->setLevelTypeInt(15000); ur->set(UR_P, 500, control::ABSENT_CORRECTED);
  for(int utype = 1; utype < UR_dd; ++utype){
      if(utype == UR_dd || utype == UR_ff){
          continue;
        }
      urType = static_cast<ValueType>(utype);
      if(ur1.isGood(urType) && ur2.isGood(urType)){
          ur->set(urType, ur2.value(urType) - ur1.value(urType), control::ABSENT_CORRECTED);
        }
      else {
          ur->set(urType, BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);
        }
    }
  float dd, ff;
  if (ur->isGood(UR_u) && ur->isGood(UR_v)) {
      MnMath::preobrUVtoDF(ur->value(UR_u), ur->value(UR_v), &dd, &ff);
      ur->set(UR_dd, dd, control::ABSENT_CORRECTED);
      ur->set(UR_ff, ff, control::ABSENT_CORRECTED);
    }
  else {
      ur->set(UR_dd, BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);
      ur->set(UR_ff, BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);
    }

  return true;
}

bool UrovenList::getUrMaxWind(Uroven *ur)
{
  if(!isData_) return false;
  bool ok = true;
  if(1 > urMaxWind_.countValid()) {
      ok = getUrPoLevelType(meteodescr::kMaxWindLevel, &urMaxWind_);
    }
  *ur = urMaxWind_;
  return ok;
}

bool UrovenList::getTropo(Uroven *ur)
{
  if(!isData_) return false;
  bool ok = true;
  if(urTropo_.isEmpty()) {
      ok = getTropo(&urTropo_);
    }
  if (urTropo_.size() == 0) {
      return false;
    }

  //самый нижний
  int loIdx = 0;
  float loP = 0;
  for (int idx = 0; idx < urTropo_.size(); idx++) {
      if (loP < urTropo_.at(idx).value(UR_P)) {
          loP = urTropo_.at(idx).value(UR_P);
          loIdx = idx;
        }
    }

  *ur = urTropo_.at(loIdx);

  return ok;
}

bool UrovenList::getTropo(QList<Uroven> *ur)
{
  if(!isData_) return false;
  bool ok = true;
  urTropo_.clear();
  ok = getUrPoLevelType(meteodescr::kTropoLevel, &urTropo_);
  *ur = urTropo_;

  return ok;
}


bool UrovenList::getCloudDkrH (CloudDkr* Dkr) const
{

  QMap<float, float>::iterator it = Dkr->DkrP.end();

  float H;
  Uroven ur;
  while( it != Dkr->DkrP.begin() ){
      it--;
      if(!getUrPoP(it.key(), &ur)){
          continue;
        }
      H = ur.value(UR_H);
      Dkr->DkrH.insert(H, it.value());
    }
  return true;
}

bool UrovenList::oprD()
{
  if(!isData_) return false;
  QMap <Level, Uroven>::iterator it = urList_.begin();
  QMap <Level, Uroven>::iterator eit = urList_.end();
  for(;it != eit; ++it){

      if(!it.value().isGood(UR_Td)){
          it->to_Td();
        }
      else if(!it.value().isGood( UR_D )){
          it->to_D();
        }
    }
  return true;
}

bool UrovenList::oprH()
{
  if(!isData_) return false;
  if(keyType_ != UR_P) return false;
  // int kol_dan = urList_.count();

  QMap <Level, Uroven>::iterator it = urList_.begin();

  QMap <Level, Uroven>::iterator nit;

  while (it != urList_.end() - 1) {
      nit = it+1;
      if (nit == urList_.end()) break;
      if(it.key() == nit.key() && it.key() != -9999) {
          for ( int urs = 0; urs < LAST_UR; ++urs){
              ValueType at = static_cast<ValueType> (urs);
              if(it.value().quality(at) > nit.value().quality(at)){
                  it.value().set(at,nit.value().value(at),nit.value().quality(at));
                }
            }
      //FIXME убрать этот код, так как уничтожался приземный уровень?
      //it.value().setLevelType(static_cast<meteodescr::LevelType> (nit.value().levelType()));

          urList_.erase(nit);
        } else {
          ++it;
        }
    }

//FIXME убрать этот код, так как уничтожался приземный уровень  ?
  QMap <Level, Uroven>::iterator eit = urList_.end();
  it = urList_.begin();
  eit = urList_.end();
  for(;it != eit; ++it){  //!< удаление из списка повторяющихся уровней с более низким качеством данных
      if(!it.value().isGood(UR_H)) {
          //debug_log << "H do = " << it.value().value(UR_H);
          oprUrHTpoP(it.key().p(), it.operator->());
          //debug_log << "H posle = " << it.value().value(UR_H);
        }
      //qDebug() << "setData():" << "H=" << it.value().value(UR_H);
    }

  return true;
}

bool UrovenList::getUrPoH( float aval,Uroven * aur)const{
  if(nullptr == aur) return false;
  if(!isData_) return false;
  if(keyType_ != UR_P) return false;

  bool okPoint = false;
  QMap <Level, Uroven>::const_iterator it;
  for(it = urList_.begin(); it != urList_.end(); ++it ){
      if(MnMath::isEqual(it.value().value(UR_H),aval)) {
          *aur = it.value();     //если уже есть данная точка в списке точек, то копируем все данные из списка
          okPoint = true;
        }
    }
  bool okTH = oprUrHTpoH(aval, aur);    //дополняем недостающие данные h,t,d,td
  bool okWind = oprUrWindPoH(aval, aur);//дополняем недостающие данные u,v,dd,ff
  if( okPoint || okTH || okWind){  //если есть точки по ветру или температуре, позволяющие определить эти параметры в искомой точке

      return true;
    }

  return false;
}



bool UrovenList::getUrPoLevelType(int lev_type, Uroven * aur) const{
  if(nullptr == aur)  return false;
  if(!isData_) return false;

  QMap <Level, Uroven>::const_iterator it = urList_.begin();
  QMap <Level, Uroven>::const_iterator eit = urList_.end();
  for(;it != eit; ++it){
      //      if(levelTypePoGroupType(gr_type) == it.value().groupType()){
      if(lev_type == it.value().levelType()){
          *aur = it.value();
          return true;
        }
    }
  return false;//почему true если ни одного уровня не найдено?
}

bool UrovenList::getUrPoLevelType(int lev_type, QList<Uroven> * aur)const
{
  if(nullptr == aur)  return false;
  if(!isData_) return false;

  bool ok = false;
  QMap <Level, Uroven>::const_iterator it = urList_.begin();
  QMap <Level, Uroven>::const_iterator eit = urList_.end();
  for(;it != eit; ++it){
      //      if(levelTypePoGroupType(gr_type) == it.value().groupType()){
      if(lev_type == it.value().levelType()){
          if (it.value().countValid() > 1) {
              *aur << it.value();
              ok  = true;
            }
        }
    }
  return ok;
}

bool UrovenList::getUrPoP( float aval,Uroven * aur)const{
  if(nullptr == aur) return false;
  if(!isData_) return false;
  if(keyType_ != UR_P) return false;

  bool okPoint = false;
  bool okTH = false;
  if(contains(aval,meteodescr::kIsobarLevel)){
      *aur = getUrPoKey(aval);        //если точка есть в списке, то копируем все имеющиеся данные по этой точке
      okPoint = true;
    } else {
      okTH = oprUrHTpoP(aval, aur);    //дописываем данные h,t,d,Td
    }

  bool okWind = oprUrWindPoP(aval, aur);//дописываем данные u,v,dd,ff
  if(okPoint || okTH || okWind){  //если есть точки по ветру или температуре, позволяющие определить эти параметры в искомой точке
      return true;
    }
  return false;
}


Uroven UrovenList::getUrPoKey(float key)const{
  Uroven ur;
  return urList_.value(key, ur);
}

int UrovenList::countValid(ValueType atype) const{
  QList<Level> levels = urList_.keys(); //!< данные по уровням
  int count_valid =0;
  for(int i=0; i<standartLevels.size();++i ){
      Uroven aur;
      if(getUrPoP( standartLevels.at(i),&aur)){
          if(aur.isGood(atype)){
              count_valid++;
            }
        }
    }

  return count_valid;
}



void UrovenList::print() const
{
  debug_log << toString();
}

QString UrovenList::toString() const
{
  QString res = QObject::tr("Уровень: ");
  for ( auto it = urList_.begin(), end = urList_.end(); it != end; ++it ) {
    res += QString("%1\n").arg( it.value().toString() );
  }
  return res;
}

bool UrovenList::isEmpty() const {
  return !isData_;
}



int UrovenList::sizeofBuffer()const{
  int asize = 0;
  asize += urZ_.size()*4;
  asize += urZ_.size()*urList_.size();
  asize += sizeof(double)*urList_.size();
  asize += sizeof(isData_);//type_net_
  asize += sizeof(keyType_);//type_net_
  return asize;

}

QList<meteo::surf::Uroven> UrovenList::toProtoUroven() const
{
  QList<meteo::surf::Uroven> list;

  QMapIterator<Level,Uroven> it(urList_);
  while ( it.hasNext() ) {
      it.next();

      const Uroven& u = it.value();

      ::meteo::surf::Uroven ur;
      ur.mutable_p()->set_value(u.value(UR_P));
      ur.mutable_p()->set_quality(u.quality(UR_P));
      ur.mutable_h()->set_value(u.value(UR_H));
      ur.mutable_h()->set_quality(u.quality(UR_H));
      ur.mutable_t()->set_value(u.value(UR_T));
      ur.mutable_t()->set_quality(u.quality(UR_T));
      ur.mutable_td()->set_value(u.value(UR_Td));
      ur.mutable_td()->set_quality(u.quality(UR_Td));
      ur.mutable_d()->set_value(u.value(UR_D));
      ur.mutable_d()->set_quality(u.quality(UR_D));
      ur.mutable_dd()->set_value(u.value(UR_dd));
      ur.mutable_dd()->set_quality(u.quality(UR_dd));
      ur.mutable_ff()->set_value(u.value(UR_ff));
      ur.mutable_ff()->set_quality(u.quality(UR_ff));
      ur.set_level_type(u.levelType());
      list << ur;
    }

  return list;
}

TMeteoData UrovenList::toMeteoData() const
{
  TMeteoData md;

  auto cur = urList_.constBegin();
  auto end = urList_.constEnd();
  for ( ; cur!=end; ++cur ) {
      TMeteoData& lvl = md.addChild();
      lvl = cur.value().toMeteoData();
    }

  // TODO: добавить данные на уровне станции
  // TODO: добавить данные об индексе станции, координатах, времени

  return md;
}

void UrovenList::checkOsobyPoint(){
  if(urList_.size() < 3 ) return;
  //  QMap <double, Uroven>::iterator it  = urList_.begin();
  //  QMap <double, Uroven>::iterator eit = urList_.end();
  QMap <Level, Uroven>::iterator eit  = urList_.begin()+1;
  QMap <Level, Uroven>::iterator it = urList_.end();
  QMap <Level, Uroven> goodList;
  it-=2;
  urList_T_.clear();
  urList_V_.clear();

  bool is_tropo = true;
  for (;it != eit; --it) {
      const Uroven &ur0 = it.value();
      Uroven &ur = (it-1).value();
      const Uroven& ur1 = (it-2).value();
      if( meteodescr::kTropoLevel == ur0.levelType()){
          is_tropo = false;
        }
      if ( !ur0.isGood(UR_P)||!ur.isGood(UR_P)||!ur1.isGood(UR_P)
           ||meteodescr::kSurfaceLevel == ur.levelType()
           ||meteodescr::kMeanSeaLevel == ur.levelType()
           ){
          continue;
        }
      float p0 = ur0.value(UR_P);
      float p = ur.value(UR_P);
      float p1 = ur1.value(UR_P);

      if (ur0.isGood(UR_T)&& ur.isGood(UR_T)&& ur1.isGood(UR_T) )
        {
          float t0 = ur0.value(UR_T);
          float t = ur.value(UR_T);
          float t1 = ur1.value(UR_T);
          float t_i = interpolT(p0,p1,p,t0,t1);
          if(( is_tropo && fabs(t_i-t) > KR_OSOBY_T_TROPO )||
             (!is_tropo && fabs(t_i-t) > KR_OSOBY_T_STRATO ))
            {
              //особая точка
              //    ur.print();
              urList_T_.append(ur);
            }
        }
      if(ur0.isGood(UR_dd)&& ur.isGood(UR_dd)&& ur1.isGood(UR_dd)&&
         ur0.isGood(UR_ff)&& ur.isGood(UR_ff)&& ur1.isGood(UR_ff))
        {
          float dd0 = ur0.value(UR_dd);
          float dd = ur.value(UR_dd);
          float dd1 = ur1.value(UR_dd);
          float ff0 = ur0.value(UR_ff);
          float ff = ur.value(UR_ff);
          float ff1 = ur1.value(UR_ff);
          float dd_i = BAD_METEO_ELEMENT_VAL;
          float ff_i = BAD_METEO_ELEMENT_VAL;
          interpolDDFF(p0, p1, p,dd0,ff0,dd1,ff1,&dd_i,&ff_i);
          if(( fabs(ff_i-ff) > KR_OSOBY_FF )||(fabs(dd_i-dd) > KR_OSOBY_DD ))
            {
              //особая точка
              // debug_log<<"особая точка по ветру"<<ff_i<<ff<<fabs(ff_i-ff)<<dd_i<<dd<<fabs(dd_i-dd);
              urList_V_.append(ur);
              //      ur.print();
            }
        }
    }
}




bool UrovenList::interpolDF()
{
  if(!isData_) return false;

  QMap <Level, Uroven>::iterator it = urList_.begin()+1;
  QMap <Level, Uroven>::iterator eit = urList_.end();
  QMap <Level, Uroven>::iterator next_it, prev_it;;
  for(;it != eit-1 && it != eit; ++it){
      if(it.value().isGood( UR_H ) && !it.value().isGood( UR_dd )
         && !it.value().isGood( UR_ff)){
          next_it = it+1;
          prev_it = it-1;
          if(next_it !=eit ){
              if(next_it.value().isGood( UR_H ) && next_it.value().isGood( UR_dd ) && next_it.value().isGood( UR_ff)&&
                 prev_it.value().isGood( UR_H ) && prev_it.value().isGood( UR_dd ) && prev_it.value().isGood( UR_ff)){
                  Uroven &aur = it.value();
                  interpolWind(next_it.value(), prev_it.value(), &aur);
                }
            }
        }
    }

  //UV2DF();
  return true;
}

//!<определяет ветер на заданном уровне по давлению
bool UrovenList::oprUrWindPoP( float aval,Uroven * aur)const{
  if(nullptr == aur || !isData_ || keyType_ != UR_P) return false;

  QMap<Level, zond::Uroven> aur_list = urList_;

  ValueType val_type = UR_P;
  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit = aur_list.begin();
  QMap <Level, Uroven>::iterator eit = aur_list.end();

  //1 - поиск ближайшей (опорной) точки по P с данными о ветре
  QMap <Level, Uroven>::iterator near_it = eit;
  it = bit;
  float npp = MAX_PRESS;
  for(;it != eit; ++it){
      // debug_log <<" i: "<< it.key()<<it.value().value(val_type)<<it.value().quality(val_type);
      if(it.value().isGood(UR_u) &&
         it.value().isGood(UR_v) &&
         it.value().isGood(val_type) &&
         fabs(it.value().value(val_type) - aval) < npp  ){
          npp = fabs(it.value().value(val_type) - aval);
          near_it = it;
          if(MnMath::isZero(npp)) { //если отличие в уровнях равно нулю, то нет смысла искать дальше:
              //ближайший уровень совпадает с искомой точкой
              *aur = near_it.value();
              aur->to_ddff();
              return true;
            }
        }
    }
  if(near_it == eit) return false;

  //2 - определение направления поиска второй точки по ветру
  bool var1 = (aval <= near_it.value().value(val_type)); //искомый уровень выше опорного
  bool var2 = (aval > near_it.value().value(val_type));  //искомый уровень
  if(var1){
      eit = aur_list.begin();
      bit = aur_list.end()-1;
    }
  if(var2){
      bit = aur_list.begin();
      eit = aur_list.end();
    }
  //3 - поиск второй точки cверху (снизу) aval по ветру
  QMap <Level, Uroven>::iterator up_it = aur_list.end();
  it = near_it;
  if(var2 ) ++it;
  while( it != eit ){
      if(var1)  --it;

      if( it.value().isGood(UR_u)      &&
          it.value().isGood(UR_v)      &&
          it.value().isGood(val_type)  &&
          ((var1 && it.value().value(val_type)  <= aval) ||
           (var2 && it.value().value(val_type)  >= aval) )){
          up_it = it;
          break;
        }
      if( var2 ) ++it;
    }
  if(up_it == aur_list.end()) return false;
  //debug_log <<"Found down i: "<< down_it.key()<<down_it.value().value(UR_H)<<down_it.value().value(UR_T)<<down_it.value().value(UR_dd)<<down_it.value().value(UR_ff);
  float upp = up_it.value().value(val_type);
  if( up_it == near_it || MnMath::isEqual(upp,aval) ){//TODO воможно лишнее при наличии проверки вверху
      //      *aur = up_it.value(); //если следующий за уточненным опорным уровнем совпадает с искомой точкой
      aur->set(UR_P, near_it.value().value(UR_P), near_it.value().quality(UR_P));
      aur->set(UR_u, near_it.value().value(UR_u), near_it.value().quality(UR_u));
      aur->set(UR_v, near_it.value().value(UR_v), near_it.value().quality(UR_v));
      aur->to_ddff();
      return true;
    }

  if(!oprUrHTpoP(near_it.value().value(val_type), near_it.operator->() )) return false;
  if(!oprUrHTpoP(up_it.value().value(val_type),   up_it.operator->()   )) return false;
  //debug_log <<"Found upper i: "<< up_it.key()<<up_it.value().value(UR_H)<<up_it.value().value(UR_T)<<up_it.value().value(UR_dd)<<up_it.value().value(UR_ff);

  Uroven ur_vs = getUrPoKey(aval); //проверка наличия уровня aval в списке среди всех точек (включая неучтенные точки по температуре)

  if(!ur_vs.isGood(UR_P)){
      aur->set(UR_P, aval, control::ABSENT_CORRECTED);
    } else {
      aur->set(UR_P, aval, ur_vs.quality(UR_P));
    }
  interpolWind(near_it.value(), up_it.value(),aur);//TODO возможно необходимо сделать для вариантов var1 и var2
  return true;

}

void UrovenList::addCenterName(const QString& ast){
  if(!center_.contains(ast)){
      center_.append(ast);
    }
}


QDataStream &operator<<(QDataStream &stream, const UrovenList& z)
{
  stream << z.urList_;
  stream << z.urZ_;
  stream << z.urMaxWind_;
  stream << z.urTropo_;
  stream << z.cloud_;
  int kt=z.keyType_;
  stream << kt;
  stream << z.isData_;
  stream << z.dateTime_;
  stream << z.dateTimeAnalyse_;
  stream << z.stIndex_;
  stream << z.coord_;
  stream << z.center_;
  return stream;
}

QDataStream &operator>>(QDataStream &stream,  UrovenList& z)
{
  stream >> z.urList_;
  stream >> z.urZ_;
  stream >> z.urMaxWind_;
  stream >> z.urTropo_;
  stream >> z.cloud_;
  int kt;
  stream >> kt;
  z.keyType_= static_cast< ValueType >( kt);
  stream >> z.isData_;
  stream >> z.dateTime_;
  stream >> z.dateTimeAnalyse_;
  stream >> z.stIndex_;
  stream >> z.coord_;
  stream >> z.center_;
  return stream;
}

/**
 * @brief Заполнение прото-структуры с метеодатой
 * 
 * @param stream 
 * @param z 
 * @return meteo::surf::MeteoDataProto& 
 */
QList <meteo::surf::MeteoDataProto> UrovenList::toMeteoDataProto()
{
  QList<meteo::surf::MeteoDataProto> list;

  // 
  // Вспомогательный список параметров по которым проходимся и заполняем прото-структуру
  // 
  QMap<zond::ValueType, std::string> *urParams = new QMap<zond::ValueType, std::string>();
  urParams->insert( UR_P,  "P"  );
  urParams->insert( UR_H,  "H"  );
  urParams->insert( UR_T,  "T"  );
  urParams->insert( UR_Td, "Td" );
  urParams->insert( UR_D,  "D"  );
  urParams->insert( UR_dd, "dd" );
  urParams->insert( UR_ff, "ff" );
  urParams->insert( UR_u,  "u"  );
  urParams->insert( UR_v,  "v"  );

  // 
  // Проходимся по уровням и заполняем прото структуру новыми данными
  // 
  QMapIterator<Level,Uroven> it(urList_);
  while ( it.hasNext() ) {
    it.next();

    // инициализируем прото
    meteo::surf::MeteoDataProto meteodata_proto;

    const Uroven& u = it.value();
    
    QMap <zond::ValueType, std::string>::const_iterator it_begin  = urParams->constBegin();
    QMap <zond::ValueType, std::string>::const_iterator it_end    = urParams->constEnd();

    for(;it_begin!=it_end;++it_begin){
      if ( u.isGood(it_begin.key()) ){
        meteo::surf::MeteoParamProto *param = meteodata_proto.add_param();
        param->set_value( u.value( it_begin.key() ) );
        param->set_quality( u.quality( it_begin.key() ) );
        param->set_descrname( it_begin.value() );
      } else{
        debug_log<<it_begin.value() << " - not good value!";
      }     
    }
    
    meteodata_proto.set_level_type( u.levelType() );

    // записываем полученную прото в
    list<<meteodata_proto;
  }

  delete urParams; urParams = nullptr;

  return list;
}



const UrovenList& operator>>(const UrovenList& data, QByteArray& out)
{
  QDataStream ds(&out, QIODevice::WriteOnly);
  ds << data;
  return data;
}

UrovenList& operator<<(UrovenList& data, const QByteArray& ba)
{
  QDataStream ds(ba);
  ds >> data;
  return data;
}


//!<определяет высоту и температуру на заданном уровне по высоте
bool UrovenList::oprUrHTpoH( float aval,Uroven * aur)const{
  if(nullptr == aur || !isData_ || keyType_ != UR_P ) return false;
  QMap <Level, Uroven> aurList_ = urList_;
  ValueType val_type = UR_H;
  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit = aurList_.begin();
  QMap <Level, Uroven>::iterator eit = aurList_.end();

  //1 - поиск ближайшей (опорной) точки по P с известной высотой H и температурой Т
  QMap <Level, Uroven>::iterator near_it = eit;
  it = bit;
  float npp = MAX_H;
  for(;it != eit; ++it){
      if(it.value().isGood(UR_P) &&
         it.value().isGood(UR_T) ) {
          if(!it.value().isGood(val_type)) {
              if(! oprUrHTpoP(it.value().value(UR_P), it.operator->())){
                  continue;//return false;
                }
            }
          if( fabs(it.value().value(val_type) - aval) < npp  ){
              npp = fabs(it.value().value(val_type) - aval);
              near_it = it;
              if(MnMath::isZero(npp) ){ //если различие в уровнях нулевое то эта точка ближайшая
                  *aur = near_it.value();
                  aur->to_Td();
                  return true;
                }
            }
        }
    }
  if(near_it == eit) return false;
  //2 - определение направления поиска точек по температуре
  bool var1 = (aval >= near_it.value().value(val_type)); //искомый уровень выше опорного
  bool var2 = (aval < near_it.value().value(val_type));  //искомый уровень
  if(var1){
      eit = aurList_.begin();
      bit = aurList_.end()-1;
    }
  if(var2){
      bit = aurList_.begin();
      eit = aurList_.end();
    }
  //3 - поиск точек между P(H,T) и aval по температуре и восстановление высот H на каждом уровне
  double h;
  it = near_it;
  if(var2 ) ++it;
  QMap <Level, Uroven>::iterator up_it = aurList_.end();
  while( it != eit ){
      if(var1)  --it;
      // debug_log <<" i: "<< it.key()<<it.value().value(val_type)<<it.value().quality(val_type);
      if( it.value().isGood(UR_T)  &&
          it.value().isGood(UR_P)) {
          if(!it.value().isGood(val_type)){
              h = getHpoPinPT(it.value().value(UR_P),
                              near_it.value().value(UR_P),
                              it.value().value(UR_P),
                              near_it.value().value(UR_T),
                              it.value().value(UR_T),
                              near_it.value().value(UR_H) );
              it.value().set(UR_H, h, control::ABSENT_CORRECTED);
            }
          ////первая точка перед aval
          if((var1 && it.value().value(val_type)  < aval) ||
             (var2 && it.value().value(val_type)  > aval) ){
              near_it = it;
            }
          ////вторая точка после aval
          if((var1 && it.value().value(val_type)  >= aval) ||
             (var2 && it.value().value(val_type)  <= aval) ){
              up_it = it;
              break;
            }
        }
      if(var2) ++it;
    }

  if(up_it == aurList_.end()) return false;
  float upp = up_it.value().value(val_type);
  if( MnMath::isEqual(upp,aval) ){
      *aur = up_it.value(); //если следующий за уточненным опорным уровнем совпадает с искомой точкой
      aur->to_Td();
      return true;
    }
  //debug_log <<"Found upper i: "<< up_it.key()<<up_it.value().value(UR_H)<<up_it.value().value(UR_T)<<up_it.value().value(UR_dd)<<up_it.value().value(UR_ff);


  //проверка наличия уровня aval в списке среди всех точек (включая неучтенные точки по ветру)
  for(it = aurList_.begin(); it != aurList_.end(); ++it){
      if(! oprUrHTpoP(it.value().value(UR_P), it.operator->())){
          continue;
        }
      if(MnMath::isEqual(it.value().value(val_type), aval)){
          aur->set(UR_P, it.key().p(), it.value().quality(UR_P));
          aur->set(val_type, aval, it.value().quality(val_type));
          break;
        }
    }
  if( it == aurList_.end()){
      float pval = getPpoHinPT(aval, near_it.key().p(), up_it.key().p(),
                               near_it.value().value(UR_T),
                               up_it.value().value(UR_T),
                               near_it.value().value(UR_H));
      /////////уточнение значения давления
      /////////////////////////////////////////////
      aur->set(UR_P, pval, control::ABSENT_CORRECTED);
      aur->set(val_type, aval, control::ABSENT_CORRECTED);
    }

  interpolValue(UR_T, near_it.value(), up_it.value(),aur);
  interpolValue(UR_D, near_it.value(), up_it.value(),aur);
  aur->to_Td();
  return true;
}




//!<определяет высоту и температуру на заданном уровне по давлению
bool UrovenList::oprUrHTpoP( float aval,Uroven * aur)const {
  //debug_log << "Zond::oprUrHTpoP";
  if(nullptr == aur || !isData_ || keyType_ != UR_P ) return false;
  QMap<Level, zond::Uroven> aur_list = urList_;

  ValueType val_type = UR_P;
  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit = aur_list.begin();
  QMap <Level, Uroven>::iterator eit = aur_list.end();

  //1 - поиск ближайшей (опорной) точки по P с известной высотой H и температурой Т
  QMap <Level, Uroven>::iterator near_it = eit;
  it = bit;
  float npp = MAX_PRESS;
  for(;it != eit; ++it){
    //debug_log <<" i: "<< it.key()<<it.value().value(val_type)<<it.value().quality(val_type);
      if(it.value().isGood(UR_H) &&
         it.value().isGood(UR_T) &&
         it.value().isGood(val_type) &&
         fabsf(it.value().value(val_type) - aval) < npp  ){
          npp = fabsf(it.value().value(val_type) - aval);
          near_it = it;
          if(MnMath::isZero(npp)) { //если отличие в уровнях равно нулю, то нет смысла искать дальше
              *aur = near_it.value();
              aur->to_Td();
              return true;
            }
        }
    }
  if(near_it == eit) return false;

  //2 - определение направления поиска точек по температуре
  bool var1 = (aval <= near_it.value().value(val_type)); //искомый уровень выше опорного
  bool var2 = (aval > near_it.value().value(val_type));  //искомый уровень
  if(var1){
      eit = aur_list.begin();
      bit = aur_list.end()-1;
    }
  if(var2){
      bit = aur_list.begin();
      eit = aur_list.end();
    }

  //3 - поиск точек между P(H,T) и aval по температуре и восстановление высот H на каждом уровне
  double h;
  it = near_it;
  if(var2 ) ++it;

  QMap <Level, Uroven>::iterator up_it = aur_list.end(); //итератор за пределами контейнера
  while( it != eit ){
      if(var1)  --it;
      ////первая точка перед aval
      if( it.value().isGood(UR_T)     &&
          it.value().isGood(val_type) ){
          if(!it.value().isGood(UR_H)){
              h = getHpoPinPT(it.value().value(UR_P),
                              near_it.value().value(UR_P),
                              it.value().value(UR_P),
                              near_it.value().value(UR_T),
                              it.value().value(UR_T),
                              near_it.value().value(UR_H) );
              it.value().set(UR_H, h, control::ABSENT_CORRECTED);
            }

          if((var1 && it.value().value(val_type)  > aval) ||
             (var2 && it.value().value(val_type)  < aval) ){
              near_it = it;
            }
          ////вторая точка после aval
          if((var1 && it.value().value(val_type)  <= aval) ||
             (var2 && it.value().value(val_type)  >= aval) ){
              up_it = it;
              break;
            }
        }
      if(var2) ++it;
    }

  if(up_it == aur_list.end()) return false;
  //debug_log <<"Found down i: "<< down_it.key()<<down_it.value().value(UR_H)<<down_it.value().value(UR_T)<<down_it.value().value(UR_dd)<<down_it.value().value(UR_ff);
  float upp = up_it.value().value(val_type);
  if( MnMath::isEqual(upp,aval) ){
      *aur = up_it.value(); //если следующий за уточненным опорным уровнем совпадает с искомой точкой
      aur->to_Td();
      return true;
    }
  Uroven ur_vs = getUrPoKey(aval); //проверка наличия уровня aval в списке среди всех точек (включая неучтенные точки по ветру)
  if(!ur_vs.isGood(UR_P)){
      aur->set(UR_P, aval, control::ABSENT_CORRECTED);
    } else {
      aur->set(UR_P, aval, ur_vs.quality(UR_P));
    }

  h =  getHpoPinHT(aval, near_it.value().value(UR_H),
                   up_it.value().value(UR_H),
                   near_it.value().value(UR_T),
                   up_it.value().value(UR_T),
                   near_it.value().value(UR_P) );
  if(!ur_vs.isGood(UR_H)){
      aur->set(UR_H, h, control::ABSENT_CORRECTED);
    } else {
      aur->set(UR_H, ur_vs.value(UR_H), ur_vs.quality(UR_H));
    }

  if(!ur_vs.isGood(UR_T)){
      interpolValue(UR_T, near_it.value(), up_it.value(),aur);
    } else {
      aur->set(UR_T, ur_vs.value(UR_T), ur_vs.quality(UR_T));
    }

  if(!ur_vs.isGood(UR_D)){
      interpolValue(UR_D, near_it.value(), up_it.value(),aur);
    } else {
      aur->set(UR_D, ur_vs.value(UR_D), ur_vs.quality(UR_D));
    }

  if(!ur_vs.isGood(UR_Td)){
      aur->to_Td();
    } else {
      aur->set(UR_Td, ur_vs.value(UR_Td), ur_vs.quality(UR_Td));
    }

  return true;
}
//!<определяет ветер на заданном уровне по давлению
bool UrovenList::oprUrWindPoH( float aval,Uroven * aur)const{
  if(nullptr == aur || !isData_ || keyType_ != UR_P) return false;

  ValueType val_type = UR_H;

  QMap <Level, Uroven> aurList_ = urList_;
  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit = aurList_.begin();
  QMap <Level, Uroven>::iterator eit = aurList_.end();

  //1 - поиск ближайшей (опорной) точки по H с данными о ветре
  QMap <Level, Uroven>::iterator near_it = eit;
  it = bit;
  float npp = MAX_PRESS;
  for(;it != eit; ++it){
      // debug_log <<" i: "<< it.key()<<it.value().value(val_type)<<it.value().quality(val_type);
      if(it.value().isGood(UR_u) &&
         it.value().isGood(UR_v) &&
         it.value().isGood(UR_P)) {
          if(!it.value().isGood(val_type)) {
              if(! oprUrHTpoP(it.value().value(UR_P), it.operator->())){
                  continue;//return false;
                }
            }
          if(fabs(it.value().value(val_type) - aval) < npp  ){
              npp = fabs(it.value().value(val_type) - aval);
              near_it = it;
              if( MnMath::isZero(npp) ){ //если различие в уровнях нулевое то эта точка ближайшая
                  aur->set(UR_P, near_it.value().value(UR_P), near_it.value().quality(UR_P));
                  aur->set(UR_H, near_it.value().value(UR_H), near_it.value().quality(UR_H));
                  aur->set(UR_u, near_it.value().value(UR_u), near_it.value().quality(UR_u));
                  aur->set(UR_v, near_it.value().value(UR_v), near_it.value().quality(UR_v));
                  aur->to_ddff();
                  return true;
                }
            }
        }
    }
  if(near_it == eit) return false;
  //2 - определение направления поиска второй точки по ветру
  bool var1 = (aval >= near_it.value().value(val_type)); //искомый уровень выше опорного
  bool var2 = (aval < near_it.value().value(val_type));  //искомый уровень
  if(var1){
      eit = aurList_.begin();
      bit = aurList_.end()-1;
    }
  if(var2){
      bit = aurList_.begin();
      eit = aurList_.end();
    }
  //3 - поиск второй точки cверху (снизу) aval по ветру
  QMap <Level, Uroven>::iterator up_it = aurList_.end();
  it = near_it;
  if( var2 ) ++it;
  while( it != eit ){
      if( var1 )  --it;
      // debug_log <<" i: "<< it.key()<<it.value().quality(val_type)<<it.value().quality(val_type);
      if( it.value().isGood(UR_u)      &&
          it.value().isGood(UR_v)      &&
          it.value().isGood(UR_P)         ) {
          if(!it.value().isGood(UR_H)) {
              if(! oprUrHTpoP(it.value().value(UR_P), it.operator->())){
                  if( var2 ) ++it;
                  continue;//return false;
                }
            }
          if((var1 && it.value().value(val_type)  >= aval) ||
             (var2 && it.value().value(val_type)  <= aval) ) {
              up_it = it;
              break;
            }
        }
      if( var2 ) ++it;
    }
  if(up_it == aurList_.end()) return false;
  //debug_log <<"Found down i: "<< down_it.key()<<down_it.value().value(UR_H)<<down_it.value().value(UR_T)<<down_it.value().value(UR_dd)<<down_it.value().value(UR_ff);
  float upp = up_it.value().value(val_type);
  if( up_it == near_it || MnMath::isEqual(upp,aval) ){//TODO воможно лишнее при наличии проверки вверху
      //      *aur = up_it.value(); //если следующий за уточненным опорным уровнем совпадает с искомой точкой
      aur->set(UR_P, near_it.value().value(UR_P), near_it.value().quality(UR_P));
      aur->set(UR_H, near_it.value().value(UR_H), near_it.value().quality(UR_H));
      aur->set(UR_u, near_it.value().value(UR_u), near_it.value().quality(UR_u));
      aur->set(UR_v, near_it.value().value(UR_v), near_it.value().quality(UR_v));
      aur->to_ddff();
      return true;
    }

  //проверка наличия уровня aval в списке среди всех точек (включая неучтенные точки по температуре)
  for(it = aurList_.begin(); it != aurList_.end(); ++it){
      if(! oprUrHTpoP(it.value().value(UR_P), it.operator->())){
          continue;
        }
      if(MnMath::isEqual( it.value().value(val_type),aval)){
          aur->set(UR_P, it.key().p(), it.value().quality(UR_P));
          aur->set(val_type, aval, it.value().quality(val_type));
          break;
        }
    }

  if( it == aurList_.end()){
      float pval = getPpoHinPT(aval, near_it.key().p(), up_it.key().p(),
                               near_it.value().value(UR_T),
                               up_it.value().value(UR_T),
                               near_it.value().value(UR_H));

      aur->set(UR_P,     pval, control::ABSENT_CORRECTED);
      aur->set(val_type, aval, control::ABSENT_CORRECTED);
    }

  interpolWind(near_it.value(), up_it.value(),aur);//TODO возможно необходимо сделать для вариантов var1 и var2

  return true;

}
///все параметры, кроме ветра
bool UrovenList::oprPPoParam(ValueType urType, float value, QList<float> *P)
{
  if(!isData_) return false;
  double gam, H, g=G, R=Rc;
  double tK=kKelvin;
  double Pres;

  QMap <Level, Uroven>::iterator it  = urList_.begin();
  QMap <Level, Uroven>::iterator eit = urList_.end();

  QMap <Level, Uroven> goodList;

  Uroven ur;

  for (;it != eit; ++it) {
      if(!oprUrHTpoP(it.value().value(UR_P), &ur)){
          ur = it.value();
        }
      if (  ur.isGood(urType)   &&
            ur.isGood(UR_P)     &&
            ur.isGood(UR_H)     &&
            ur.isGood(UR_T) ){
          goodList.insert(it.key(),ur);
          //debug_log << it.key() << ur.value(UR_T) <<  ur.value(UR_P) << ur.value(UR_H);
        }
    }
  if(goodList.isEmpty()) {
      return false;
    }

  it  = goodList.begin();
  eit = goodList.end()-1;

  float val_0, val_1;
  double H_0, H_1;
  double T_0, T_1;
  double P_0;//, P_1;
  QMap <Level, Uroven>::iterator prev;
  while (it != eit) {
      prev = it; ++it;
      val_0 = prev.value().value(urType); //предыдущий - нижний уровень

      if(MnMath::isEqual(val_0, value) ){
          P->append(prev.value().value(UR_P));
          continue;
        }


      val_1 = it.value().value(urType);   //последующий - верхний уровень



      if((val_0 <= value && value < val_1) ||
         (val_0 >= value && value > val_1) ){

          H_0 = prev.value().value(UR_H);
          H_1 = it.value().value(UR_H);
          T_0 = prev.value().value(UR_T)+tK;
          T_1 = it.value().value(UR_T)+tK;
          P_0 = prev.value().value(UR_P);
          //        P_1 = it.value().value(UR_P);

          if(!MnMath::linInterpolY(val_0, H_0, val_1, H_1, value, &H)){    //определение высоты H путем интерполяции по параметру
              continue; //попавшая в интервал точка не обрабатывается
            }

          H-= H_0;

          gam=-(T_1 - T_0)/(H_1 - H_0); //расчет градиента температуры T
          if( gam == 0. ){
              Pres = P_0*exp( -g*H/R/T_0 );
            }
          else {
              Pres = P_0*pow((T_0 - gam*H)/T_0, g/R/gam);
            }
          P->append(Pres);
        }
    }
  return (!P->isEmpty());
}

bool UrovenList::oprHPoParam(ValueType urType, float value, QList<float>* H)
{
  if(!isData_) return false;


  QMap <Level, Uroven>::iterator it  = urList_.begin();
  QMap <Level, Uroven>::iterator eit = urList_.end();

  QMap <Level, Uroven> goodList;

  Uroven ur;

  for (;it != eit; ++it) {
      if(!oprUrHTpoP(it.value().value(UR_P), &ur)){
          ur = it.value();
        }
      if (  ur.isGood(urType)   &&
            //           it.value().isGood(UR_P)     &&
            ur.isGood(UR_H)     &&
            ur.isGood(UR_T) ){
          goodList.insert(it.key(),ur);
        }
    }
  if(goodList.isEmpty()) {
      return false;
    }

  it  = goodList.begin();
  eit = goodList.end()-1;

  float val_0, val_1;
  double H_0, H_1;
  double Hvs;

  QMap <Level, Uroven>::iterator prev;
  while (it != eit) {
      prev = it; ++it;
      val_0 = prev.value().value(urType);
      if(MnMath::isEqual(val_0, value) ){
          H->append(prev.value().value(UR_H));
          continue;
        }

      val_1 = it.value().value(urType);

      if((val_0 < value && value < val_1) ||
         (val_0 > value && value > val_1) ){

          H_0 = prev.value().value(UR_H);
          H_1 = it.value().value(UR_H);
          //        T_0 = prev.value().value(UR_T)+tK;
          //        T_1 = it.value().value(UR_T)+tK;
          //        P_0 = prev.value().value(UR_P);
          //        P_1 = it.value().value(UR_P);

          if(!MnMath::linInterpolY(val_0, H_0, val_1, H_1, value, &Hvs)){    //определение высоты H путем интерполяции по параметру
              continue; //попавшая в интервал точка не обрабатывается
            }
          H->append(Hvs);
        }
    }
  return (!H->isEmpty());
}

bool UrovenList::getUrPoHH( float aval,Uroven * aur)const{
  if(nullptr == aur) return false;
  if(!isData_) return false;

  ValueType val_type = UR_H;
  QMap <Level, Uroven>::const_iterator it = urList_.begin();
  QMap <Level, Uroven>::const_iterator eit = urList_.end();
  QMap <Level, Uroven>::const_iterator up_it = eit;

  //  debug_log <<"-----------search val i: "<< aval;
  for(;it != eit; ++it){
      //    debug_log <<" i: "<< it.key()<<it.value().value(val_type)<<it.value().quality(val_type);
      if( (it.value().isGood(UR_T)      &&
           it.value().isGood(val_type))  &&
          (it.value().value(val_type) > aval ||
           qFuzzyCompare(it.value().value(val_type), aval)) ){
          up_it = it;
        }
    }

  if(up_it == eit) return false;

  //debug_log <<"Found upper i: ";
  //  up_it.value().print();
  eit = urList_.begin();
  it = urList_.end();

  QMap <Level, Uroven>::const_iterator down_it = eit;
  while( it != eit ){
      --it;
      // debug_log <<" i: "<< it.key()<<it.value().quality(val_type)<<it.value().quality(val_type);
      if( (it.value().isGood(UR_T)      &&
           it.value().isGood(val_type)) &&
          (it.value().value(val_type) < aval ||
           qFuzzyCompare(it.value().value(val_type), aval)) ){
          down_it = it;
        }
    }
  if(down_it == eit) return false;
  //debug_log <<"Found down i: ";
  //  down_it.value().print();
  if(down_it == up_it){
      *aur = down_it.value();
      return true;
    }
  aur->set(UR_H, aval,control::ABSENT_CORRECTED);

  if(!interpolValue(UR_T, down_it.value(),up_it.value(),aur)){
      return false;
    }

  interpolValue(UR_D, down_it.value(), up_it.value(), aur);
  interpolWind(down_it.value(), up_it.value(), aur);
  return true;

}
/**
* @brief Расчет высоты по значению атмосферного давления
* если нет данных зондирования вообще, то расчет ведется по стандартной (политропной) атмофере
* если нет данных зондирования до этой высоты,  то расчет ведется по стандартной (политропной) атмофере начиная с максимального уровня
* @param p атмосферное давление, гПа на требуемой высоте
* @param z_p давление у пов. Земли, гПа (если не указано, берем из зонда)
* @param z_t температура у пов. Земли, C (если не указано, берем из зонда)
* @return double высота, м
**/

float UrovenList::oprHpoP( float p, float z_p, float z_t )
{

  float new_p = z_p;
  float h =0.;
  float t = z_t;

  if((keyType_ != UR_P && keyType_ != UR_H) ){

      if(z_p < 0.f && !getPz(&z_p)){
          return 0.;
        }
      if(z_t < 999.f && !getTz(&z_t)) {
          return 0.;
        }

      float new_p = z_p;
      new_p = z_p;
      z_t += kKelvin;
      t = z_t;

      //если данных зондирования нет, определение по стандартной модели атмосферы  (можно было бы обратную модель построить h(p))
      while(h < MAX_H ) {
          ISA(h, &t,&new_p, 0.);
          if(new_p <= p){
              break;
            }
          new_p = z_p;
          t = z_t;
          h += 10.f;
        }
      return h;
    }
  if( !isData_){ return 0.;}
  //если есть данные зондирования для этой высоты
  Uroven ur;
  if( getUrPoP(p, &ur)) {
      return ur.value(UR_H);
    }

  //если есть данные зондирования для более низких высот, берем самый верхний уровень

  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit;
  QMap <Level, Uroven>::iterator eit;

  if(keyType_ == UR_H){
      bit = urList_.begin();
      eit = urList_.end();
    }
  if(keyType_ == UR_P){
      bit = urList_.end();
      eit = urList_.begin();
    }

  it = eit;
  while( it != bit ) { //поиск сверху самого верхнего уровня с хорошими данными
      if(keyType_ == UR_H) --it;
      if(it.value().isGood(UR_P) &&
         it.value().isGood(UR_H) &&
         it.value().isGood(UR_T)){
          break;
        }
      if(keyType_ == UR_P) ++it;
    }

  if(it == bit)   { return h; }

  ur = it.value();
  h = it.value().value(UR_H);
  new_p = it.value().value(UR_P);
  t = it.value().value(UR_T) + kKelvin;

  while(h < MAX_H ) {
      ISA( h, &t, &new_p, it.value().value(UR_H) );
      if (new_p <= p) {
          break;
        }
      h += 10.f;
      new_p = it.value().value(UR_P);
      t = it.value().value(UR_T) + kKelvin;
    }
  return h;
}

/**
* @brief возвращает значение атм. давления и температуры воздуха на высоте h
*
* @param h высота, м
* @param p давление у пов. Земли, гПа (если <0, берем из зонда)
* @param t температура у пов. Земли, C (если <0, берем из зонда)
* @return bool true, если все ок
**/
bool UrovenList::p_tPoH( float h, float *p, float *t )
{
  if( (*p < 0 || *t < 0) && !isData_){
      return false;
    }
  if(*p < 0 && !getPz(p)){
      return false;
    }
  if(*t < 0 && !getTz(t)) {
      return false;
    }
  *t += kKelvin;

  ISA( h, t, p, 0. );

  if(!isData_ || (keyType_ != UR_P && keyType_ != UR_H)) {
      return true;
    }

  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit;
  QMap <Level, Uroven>::iterator eit;

  if(keyType_ == UR_H){
      bit = urList_.begin();
      eit = urList_.end();
    }
  if(keyType_ == UR_P){
      bit = urList_.end();
      eit = urList_.begin();
    }

  it = eit;
  while( it != bit ) { //поиск сверху самого верхнего уровня с хорошими данными
      if(keyType_ == UR_H) --it;
      if(it.value().isGood(UR_P) &&
         it.value().isGood(UR_H) &&
         it.value().isGood(UR_T)){
          break;
        }
      if(keyType_ == UR_P) ++it;
    }

  if(it == bit){
      return true;
    }

  *p = it.value().value(UR_P);            //давление на начальном уровне
  *t = it.value().value(UR_T) + kKelvin;   //температура на начальном уровне
  ISA(h, t, p, it.value().value(UR_H) ); //возвращает температуру и давление на заданном уровне
  return true;
}


//!< наличие приземной инверсии и параметры P, H, T, Td на ее верхнем уровне
bool UrovenList::getPprizInvHi(Uroven *ur)
{
  int cnt = urZinv_.size();
  if (cnt < 2) return false;
  if(1 < urZinv_.countValid()) {
      *ur = urZinv_;
      return true;
    }

  if(!isData_ ) return false;
  if( UR_P == keyType_ ){
      return getPprizInvHi_P(ur);
    }

  if( UR_H == keyType_ ){
      return getPprizInvHi_H(ur);
    }
  return false;
}


bool UrovenList::getPprizInvHi_P(Uroven *ur)
{
  if ( 0 == urList_.size() ) {
    warning_log << QObject::tr("Нет данных в зонде за % (%) ")
                   .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"))
                   .arg(stIndex_);
    return false;
  }
  QMap <Level, Uroven>::iterator bit = urList_.end()-1;
  QMap <Level, Uroven>::iterator eit = urList_.begin();
  QMap <Level, Uroven>::iterator it = bit;
  if ( bit == eit ) {
    warning_log <<  QObject::tr("Нет данных в зонде за %1 (%2) ")
                    .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"))
                    .arg(stIndex_);
    return false;
  }
  for( ;it != eit; --it){
      if(meteodescr::kSurfaceLevel == it.value().levelType()){ break; }
    }

  if( it == eit ) {  //!нет данных на уровне станции либо некачественные данные на уровне станции
      it = bit-1;
      warning_log << QObject::tr("Данные на уровне станции %1 за %2 некачественные либо отсутствуют!")
                   .arg(stIndex_)
                   .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"));
    }
  for( ;it != eit; --it){
      if(it.value().isGood(UR_T)) {  break; }
    }
  QMap <Level, Uroven>::iterator prev;
  prev = it;
  if ( it != urList_.begin() ) {
    --it;
    for( ;it != eit; --it) {
      if ( it.value().isGood(UR_T)
        && prev.value().isGood(UR_T)
        && it.value().value(UR_T) < prev.value().value(UR_T)) { //если не соблюдаются условия инверсии или изотермии
        break;
      }
      if ( true == it.value().isGood(UR_T) ) {
        prev = it;
      }
    }
  }

  if(!prev.value().isGood(UR_P) ||
     !prev.value().isGood(UR_H) ||
     !prev.value().isGood(UR_Td)){
      return false; //!<некачественные данные
    }

  if(prev == bit || it == eit){
      return false;
    }

  *ur = prev.value();
  urZinv_ = prev.value();
  return true;
}


//!< наличие приземной инверсии и параметры P, H, T, Td на ее верхнем уровне
bool UrovenList::getPprizInvHi_H(Uroven * ur)
{
  QMap <Level, Uroven>::iterator bit = urList_.begin();
  QMap <Level, Uroven>::iterator eit = urList_.end();
  QMap <Level, Uroven>::iterator it = bit;
  for( ;it != eit; ++it){
      if(1 == it.value().levelType()){ break; }
    }
  if( it == eit ) {  //!нет данных на уровне станции либо некачественные данные на уровне станции
      it = bit-1;
      // print();
      warning_log << QObject::tr("Данные на уровне станции %1 за %2 некачественные либо отсутствуют!")
                   .arg(stIndex_)
                   .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"));
    }
  for( ;it != eit; --it){
      if(it.value().isGood(UR_T)) {  break; }
    }
  QMap <Level, Uroven>::iterator prev;
  prev = it;
  for( ;it != eit; ++it){
      if(it.value().isGood(UR_T) && prev.value().isGood(UR_T) &&
         it.value().value(UR_T) < prev.value().value(UR_T)){ //если не соблюдаются условия инверсии или изотермии
          break;
        }
      if(it.value().isGood(UR_T)) prev = it;
    }
  //  prev.value().print();
  if(prev == bit || it == eit){
      return false;
    }

  if(!prev.value().isGood(UR_P) ||
     !prev.value().isGood(UR_H) ||
     !prev.value().isGood(UR_Td)){

      //prev.value().print();
      return false; //!<некачественные данные
    }

  *ur = prev.value();
  urZinv_ = prev.value();
  return true;
}



//! Наличие приземной инверсии и параметры P, H, T, Td на ее нижней границе
bool UrovenList::getPprizInvLow(Uroven * ur)
{
  if(!isData_ || (keyType_ != UR_P && keyType_ != UR_H)) return false;

  bool ground_level = false;
  int cnt = urList_.size();
  if (cnt < 2) return false;

  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit;
  QMap <Level, Uroven>::iterator eit;
  QMap <Level, Uroven>::iterator prev;

  if(keyType_ == UR_H){
      bit = urList_.begin();
      eit = urList_.end();
    }
  if(keyType_ == UR_P){
      bit = urList_.end();
      eit = urList_.begin();
    }
  it = bit;
  while( it != eit ){
      if(keyType_ == UR_P) --it;
      if(1 == it.value().levelType()){
          ground_level = true;
          break;
        }
      if(keyType_ == UR_H) ++it;
    }

  if( it == eit || (1 == it.value().levelType() && !it.value().isGood(UR_T)) ) {  //!нет данных на уровне станции либо некачественные данные на уровне станции
      ground_level = false;
      warning_log << QObject::tr("Данные на уровне станции %1 за %2 некачественные либо отсутствуют!")
                   .arg(stIndex_)
                   .arg(dateTime().toString("yyyy-MM-ddThh:mm:ss"));
    }

  if(keyType_ == UR_H){
      eit = urList_.end()-1;
    }
  if(keyType_ == UR_P){
      bit = urList_.end()-1;
    }

  if(!ground_level){ //TODO взять данные анализа
      it = bit; //пока в it передается нижний уровень, если данные по станции плохие или отсутствуют
    }


  prev = it;//инициализация prev найденным уровнем станции или нижним уровнем из списка (если данные по станции плохие или их нет)

  while(it != eit) {
      if(keyType_ == UR_H) {
          ++it;
        }
      if(keyType_ == UR_P) {
          --it;
        }
      //     debug_log << it.value().value(UR_H)<< it.value().value(UR_P) << it.value().value(UR_T);

      if(it.value().isGood(UR_T) && prev.value().isGood(UR_T) &&
         it.value().value(UR_T) < prev.value().value(UR_T)){ //если не соблюдаются условия инверсии или изотермии
          //    debug_log << "not inver conditions" <<it.value().value(UR_H);
          continue;
        }
      if(it.value().isGood(UR_T)){
          //       debug_log << it.value().value(UR_T);
          //     debug_log << prev.value().value(UR_T);
          // prev = it; //Нашли нижнюю границу инверсии/изотермии
          break;
          //it=eit; // Это норм?
        }

    }

  if ( !prev.value().isGood(UR_P) ||
       !prev.value().isGood(UR_H) ||
       !prev.value().isGood(UR_Td)){
      return false; //!<некачественные данные
    }

  *ur = prev.value();

  return true;
}

//!< h1 - высота верхней границы слоя, где скорость ветра V = 0.8 Vmax, в км
//!< h2 - высота нижней границы слоя, где скорость ветра V = 0.8 Vmax, в км
bool UrovenList::oprH1Fast(float *h1, float *h2)
{
  if(!isData_ || (keyType_ != UR_P && keyType_ != UR_H) ) return false;

  float v;
  float max_v = BAD_METEO_ELEMENT_VAL; //, h_max_v = 0;
  float V_V,V_N,H_V,H_N;
  QMap <Level, Uroven>::iterator it;
  QMap <Level, Uroven>::iterator bit;
  QMap <Level, Uroven>::iterator eit;
  QMap <Level, Uroven>::iterator max_wind_it;

  if(keyType_ == UR_P){
      bit = urList_.begin();
      eit = urList_.end();
    }

  if(keyType_ == UR_H){
      bit = urList_.end();
      eit = urList_.begin();
    }

  //поиск уровня с максимальным ветром Vmax от земли
  it = eit;
  while( it != bit ) {
      if(keyType_ == UR_P) --it;
      if(it.value().isGood(UR_dd) && it.value().isGood(UR_ff) &&
         it.value().value(UR_ff) > max_v) {
          max_v = it.value().value(UR_ff);
          max_wind_it = it;
        }

      if(keyType_ == UR_H) ++it;
    }
  if(MnMath::isEqual(max_v, BAD_METEO_ELEMENT_VAL)) return false;

  v = max_v;

  //поиск нижней границы слоя с V=0.8Vmax (оси струйного течения) от уровня с максимальным ветром
  //поиск нижнего ближайшего уровня к 0.8Vmax
  it = max_wind_it;
  while( it != eit ) {
      if(keyType_ == UR_H) --it;
      if(it.value().isGood(UR_H) && it.value().isGood(UR_dd) && it.value().isGood(UR_ff) &&
         it.value().value(UR_ff) < v*0.8f) {
          break;
        }
      if(keyType_ == UR_P) ++it;

    }
  //if(!it.value().isGood(UR_dd) || !it.value().isGood(UR_ff)) return false;

  H_N = it.value().value(UR_H)/1000.f;
  V_N = it.value().value(UR_ff);

  //поиск верхнего ближайшего уровня к 0.8Vmax и интерполяция
  while( it != max_wind_it ) {
      if(it.value().isGood(UR_H) && it.value().isGood(UR_dd) && it.value().isGood(UR_ff) &&
         it.value().value(UR_ff) >= v*0.8f) {
          break;
        }
      if(keyType_ == UR_P) --it;
      if(keyType_ == UR_H) ++it;
    }

  H_V = it.value().value(UR_H)/1000.f;
  V_V = it.value().value(UR_ff);
  *h1 = H_N + ((0.8f*v - V_N)/(V_V - V_N))*(H_V - H_N);

  //поиск верхней границы слоя с V=0.8Vmax (оси струйного течения) от уровня с максимальным ветром
  //поиск верхнего ближайшего уровня к 0.8Vmax
  it = max_wind_it;
  while( it != bit) {
      if(keyType_ == UR_P) --it;
      if(it.value().isGood(UR_H) && it.value().isGood(UR_dd) && it.value().isGood(UR_ff) &&
         it.value().value(UR_ff) < v*0.8f) {
          break;
        }

      if(keyType_ == UR_H) ++it;
    }

  H_V = it.value().value(UR_H)/1000.f;
  V_V = it.value().value(UR_ff);

  //поиск нижнего ближайшего уровня к 0.8Vmax и интерполяция
  while( it != max_wind_it ) {
      if(it.value().isGood(UR_H) && it.value().isGood(UR_dd) && it.value().isGood(UR_ff) &&
         it.value().value(UR_ff) >= v*0.8f) {
          break;
        }
      if(keyType_ == UR_P) ++it;
      if(keyType_ == UR_H) --it;
    }

  H_N = it.value().value(UR_H)/1000.f;
  V_N = it.value().value(UR_ff);
  *h2 = H_N + ((0.8f*v - V_N)/(V_V - V_N))*(H_V - H_N);

  return true;
}


}
