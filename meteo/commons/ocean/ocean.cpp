#include "ocean.h"

#include <cross-commons/debug/tlog.h>

#include <qdebug.h>

namespace ocean{
  
  
  Ocean::Ocean() 
  {
    keyType_ = LAST_UR;
    isData_ = false;
    clear();
  }
  
  Ocean::~Ocean() {
   clear();    
  }

  void Ocean::clear(){
    keyType_ = LAST_UR;
    isData_ = false;
    urList_.clear();
  }
  
  bool Ocean::setData(TMeteoData& adata)
  {
    descr_t ades = descrPoUrType(UR_H);
    
    QList<TMeteoData*> urList = adata.findChilds(ades);
    int cnt = urList.count();
    //    debug_log << "urList__.count() = " << urList__.count();
    if(1 > cnt ) return false;    
    for (int lvl = 0 ; lvl < cnt ; ++lvl) {
      const TMeteoData* md = urList.at(lvl);
      if(nullptr == md) continue;
      meteodescr::LevelType lt = static_cast< meteodescr::LevelType > (md->meteoParam("level_type").value());
      if (meteodescr::kSurfaceLevel == lt) continue;
      Uroven ur;
      TMeteoParam p; 
      for ( int urs =0; urs < LAST_UR; ++urs){
        ValueType at = static_cast<ValueType> (urs);
        p = md->getParam(descrPoUrType(at)); 
        ur.set(at, p.value(), p.quality());
      }
      ur.setLevelType(lt);

      urList_.insert(ur.value(UR_H),ur);
    }
    
    if( 0 < cnt){     
     isData_ = true;
     keyType_ = UR_H;
    }
 
    return true;  
  }

  Uroven Ocean::getUrPoKey(float key)const{
    Uroven ur;
    return urList_.value(key, ur);
  }
  
  
  bool Ocean::isEmpty(){
    return !isData_;
  }
  
}
