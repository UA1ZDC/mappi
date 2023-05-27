#include "turoven.h"

#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>


namespace ocean {
  
  Uroven::Uroven()
  { 
    reset();
  }

  void Uroven::reset()
  {
    for(int type = UR_H; type<LAST_UR; ++type) {
      set( static_cast<ValueType> (type), BAD_METEO_ELEMENT_VAL, control::MISTAKEN);
    }

    group_type = LastGroup;
  }
  
  
  bool Uroven::isGood(ValueType atype) const
  {
    return  (quality(atype) <=  control::DOUBTFUL );
  }
  
QPair<float, int> Uroven::pair(ValueType type) const
{
  switch(type) {
  case UR_H: return H;
  case UR_T: return T;
  case UR_S: return S;
  default: break; 
  }
  return QPair<float, int>();
}

void Uroven::print() const
{
  debug_log << " H = "<<value(UR_H)<<"("<<quality(UR_H) <<")"
	    << " T = "<<value(UR_T)<<"("<<quality(UR_T) <<")"
	    << " S = "<<value(UR_S)<<"("<<quality(UR_S) <<")"
	    <<" countValid"<<countValid();
}

int Uroven::countValid() const
{
  int k=0;
  for ( int urs =0; urs < LAST_UR; ++urs){
    ValueType at = static_cast<ValueType> (urs);
    if(isGood(at)){
      k++;
    }
  }
  return k;
}

void Uroven::setGroupType(GroupType gr_type)
{
  group_type = gr_type;
}

void Uroven::setLevelType(int lvl_type)
{
  level_type = lvl_type;
}


void Uroven::set(ValueType type, float val, int qual)
{
  QPair<float, int>* pair;
  switch(type) {
    case UR_H: pair = &H; break;
    case UR_T: pair = &T; break;
    case UR_S: pair = &S; break;
    default : return;
  }
  
  pair->first = val;
  pair->second = qual;
  
}

int descrPoUrType(ValueType type) 
{
  int ad_=0;

  switch(type) {
  case UR_H:   ad_= TMeteoDescriptor::instance()->descriptor("zn"); break; //07062
  case UR_T:   ad_= TMeteoDescriptor::instance()->descriptor("Tw"); break; //22043
  case UR_S:   ad_= TMeteoDescriptor::instance()->descriptor("Sn"); break; //22062
  default : return 0;
  }
  return ad_;
}

QString& operator<<( QString& str, const Uroven& ur )
{
  str += QString("Группа = %1. Тип уровня = %2. H = [%3,%4], T = [%5,%6], S = [%7,%8]")
    .arg(ur.group_type)
    .arg(ur.level_type)
    .arg(ur.H.first)
    .arg(ur.H.second)
    .arg(ur.T.first)
    .arg(ur.T.second)
    .arg(ur.S.first)
    .arg(ur.S.second);
  return str;
}

TLog& operator<<( TLog& log, const Uroven& ur )
{
  QString str;
  str << ur;
  return log << str;
}

}
