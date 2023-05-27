#include "turoven.h"

#include "diagn_func.h"

#include <commons/meteo_data/tmeteodescr.h>

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>


namespace zond {

  Uroven::Uroven()
  {
    reset();
  }

  void Uroven::reset(){
    for(int type = UR_P; type<LAST_UR;++type)
      set( static_cast<ValueType> (type), BAD_METEO_ELEMENT_VAL, control::NO_OBSERVE);
    level_type = meteodescr::kUnknownLevel;

  }


  bool Uroven::isGood(ValueType atype) const{
    return  (quality(atype) <=  control::DOUBTFUL );
  }

  void Uroven::to_ddff(){
    if(!isGood(UR_u) ||!isGood(UR_v)) return;
    MnMath::preobrUVtoDF(u.first, v.first, &dd.first, &ff.first);
    dd.second =  control::AUTO_CORRECTED;
    ff.second =  control::AUTO_CORRECTED;
  }

  void Uroven::to_uv(){
    if(!isGood(UR_dd) ||!isGood(UR_ff)) return;

    MnMath::convertDFtoUV(dd.first, ff.first, &u.first, &v.first);
    u.second =  control::AUTO_CORRECTED;
    v.second =  control::AUTO_CORRECTED;
  }

  void Uroven::to_Td(){
    if(!isGood(UR_T) ||!isGood(UR_D)) return;
    if( Td.second > control::HAND_CORRECTED ){
      Td.first = T.first - D.first;
      Td.second = control::AUTO_CORRECTED;
    }
  }

  void Uroven::to_D(){
    if(!isGood(UR_T) ||!isGood(UR_Td)) return;
    if((D.second > control::HAND_CORRECTED)){
      D.first = T.first - Td.first;
      D.second = control::AUTO_CORRECTED;
    }
  }



QPair<float, int> Uroven::pair(ValueType type) const
{
  switch(type) {
  case UR_P: return P;
  case UR_H: return H;
  case UR_T: return T;
  case UR_D: return D;
  case UR_Td: return Td;
  case UR_dd: return dd;
  case UR_ff: return ff;
  case UR_u: return u;
  case UR_v: return v;
  default: break;
  }
  return QPair<float, int>();
}

void Uroven::print() const
{
  debug_log << toString();
}

QString Uroven::toString() const
{
  QString res = QString("%1. %2. %3. %4. %5. %6. %7. %8. %9. %10. %11")
    .arg( QString("type = %1").arg( levelType() ) )
    .arg( QString( "P = %1 [%2]").arg( value(UR_P) ).arg( quality(UR_P) ) )
    .arg( QString( "h = %1 [%2]").arg( value(UR_H) ).arg( quality(UR_H) ) )
    .arg( QString( "T = %1 [%2]").arg( value(UR_T) ).arg( quality(UR_T) ) )
    .arg( QString( "D = %1 [%2]").arg( value(UR_D) ).arg( quality(UR_D) ) )
    .arg( QString( "Td = %1 [%2]").arg( value(UR_Td) ).arg( quality(UR_Td) ) )
    .arg( QString( "dd = %1 [%2]").arg( value(UR_dd) ).arg( quality(UR_dd) ) )
    .arg( QString( "ff = %1 [%2]").arg( value(UR_ff) ).arg( quality(UR_ff) ) )
    .arg( QString( "u = %1 [%2]").arg( value(UR_u) ).arg( quality(UR_u) ) )
    .arg( QString( "v = %1 [%2]").arg( value(UR_v) ).arg( quality(UR_v) ) )
    .arg( QString( "valid = %1").arg( countValid() ) );
  return res;
}

int Uroven::countValid()const{
  int k=0;
  for ( int urs =0; urs < LAST_UR; ++urs){
    ValueType at = static_cast<ValueType> (urs);
    if(isGood(at)){
        k++;
        }
  }
  return k;
}

void zond::Uroven::setLevelTypeInt( int lvl_type)
{
  level_type =static_cast< meteodescr::LevelType > (lvl_type);
}

void zond::Uroven::setLevelType( meteodescr::LevelType lvl_type)
{
  level_type = lvl_type;
}


void Uroven::set(ValueType type, float val, int qual)
{
  QPair<float, int>* pair;
  switch(type) {
    case UR_P: pair = &P; break;
    case UR_H: pair = &H; break;
    case UR_T: pair = &T; break;
    case UR_Td: pair = &Td; break;
    case UR_D: pair = &D; break;
    case UR_dd: pair = &dd; break;
    case UR_ff: pair = &ff; break;
    case UR_u: pair = &u; break;
    case UR_v: pair = &v; break;
    default : return;
  }

  pair->first = val;
  pair->second = qual;

}
/*
int levelTypePoGroupType(GroupType type)
{

  int ad_=100;
  switch(type) {
    case Gr_Station:
      ad_= 1;
      break;
    case Gr_Tropo:
      ad_= 7;
      break;
    case Gr_MaxWind_D:  //TODO
    case Gr_MaxWind:
      ad_= 6;
      break;
    case Gr_Tempr_D:   //TODO одинаковые дескрипторы для ветра
    case Gr_Wind_D:   //TODO одинаковые дескрипторы для ветра
    case Gr_Tempr:
    case Gr_Wind:
      ad_= 101;
      break;
    case Gr_Lidar:
      ad_= 105;
      break;
    case Gr_Standart:
    case Gr_Standart_C:
    default:
      ad_= 100; break;//TODO одинаковые дескрипторы для ветра
  }
  return ad_;
}*/

int descrPoUrType(ValueType type)
{

  int ad_=0;
  switch(type) {
    case UR_P:    ad_= 7004;  break;
    case UR_H:   ad_= 10009; break;
    case UR_T:   ad_= 12101; break;
    case UR_D:   ad_= 12108; break;
    case UR_Td:   ad_= 12103; break;
    case UR_dd: ad_= 11001; break;
    case UR_ff:   ad_= 11002; break;
    case UR_u: ad_= 11003; break; //TODO
    //case UR_u: ad_= 9911001; break; //TODO
    case UR_v:   ad_= 11004; break;//TODO одинаковые дескрипторы для ветра
    //case UR_v:   ad_= 9911002; break;//TODO одинаковые дескрипторы для ветра
    default : return 0;
  }
  return ad_;
}

ValueType UrTypePoDescr( int descr)
{

  ValueType ad_=LAST_UR;
  switch(descr) {
    case  7004:    ad_= UR_P;  break;
    case  10004:    ad_= UR_P;  break;
    case  10051:    ad_= UR_P;  break;
    case 10009:   ad_= UR_H; break;
    case 12101:   ad_= UR_T; break;
    case 12108:   ad_= UR_D; break;
    case 12103:   ad_= UR_Td; break;
    case 11001: ad_= UR_dd; break;
    case 11002:   ad_= UR_ff; break;
    case 9911001:
    case 11003:
      ad_= UR_u; break; //TODO
    case 11004:
    case 9911002:
      ad_= UR_v; break;//TODO одинаковые дескрипторы для ветра

    case 20012: ad_= UR_CLOUD_type; break;//тип облачности
    case 20011: ad_= UR_CLOUD_Nh; break;//тип облачности
    case 20013: ad_= UR_CLOUD_h; break;//тип облачности


    default : return LAST_UR;
  }
  return ad_;
}


int Uroven::size() const{
  int asize = 0;
  asize += sizeof(float);
  asize += sizeof(int);
  asize *= 9;
  asize += sizeof(level_type);
  return asize;
}

TMeteoData Uroven::toMeteoData() const
{
  TMeteoData md;

  for ( int type=UR_P; type<LAST_UR; ++type ) {
    if ( !isGood(ValueType(type)) ) { continue; }

    md.add(descrPoUrType(ValueType(type)), "", value(ValueType(type)), control::QualityControl(quality(ValueType(type))));
  }

  return md;
}


QDataStream &operator>>(QDataStream &stream, Uroven &u)
{
  stream >> u.P;
  stream >> u.H;
  stream >> u.D;
  stream >> u.T;
  stream >> u.Td;
  stream >> u.dd;
  stream >> u.ff;
  stream >> u.u;
  stream >> u.v;
  int lt;
  stream >> lt;
  u.setLevelTypeInt(lt);

  return stream;
}

QDataStream &operator<<(QDataStream &stream, const Uroven &u)
{
  stream << u.P;
  stream << u.H;
  stream << u.D;
  stream << u.T;
  stream << u.Td;
  stream << u.dd;
  stream << u.ff;
  stream << u.u;
  stream << u.v;
  stream << u.levelType();
  return stream;
}

Uroven& Uroven::operator=( const Uroven& src ){
  P = src.P;
  H = src.H;
  D = src.D;
  T = src.T;
  Td = src.Td;
  dd = src.dd;
  ff = src.ff;
  u = src.u;
  v = src.v;
  level_type = src.level_type;
  return *this;
}

TLog& operator<<( TLog& log, const Uroven& ur )
{
  return log << QString("uroven[%1] = [P=(%2:%3), H=(%4,%5), T=(%6,%7), D=(%8,%9), Td=(%10,%11), dd=(%12,%13), ff=(%14,%15), u=(%16,%17), v=(%18,%19) ]")
    .arg( QString::number( ur.level_type) )
    .arg( QString::number( ur.P.first,'f',1) ).arg( QString::number( ur.P.second) )
    .arg( QString::number( ur.H.first,'f',1) ).arg( QString::number( ur.H.second) )
    .arg( QString::number( ur.T.first,'f',1) ).arg( QString::number( ur.T.second) )
    .arg( QString::number( ur.D.first,'f',1) ).arg( QString::number( ur.D.second) )
    .arg( QString::number( ur.Td.first,'f',1) ).arg( QString::number( ur.Td.second) )
    .arg( QString::number( ur.dd.first,'f',1) ).arg( QString::number( ur.dd.second) )
    .arg( QString::number( ur.ff.first,'f',1) ).arg( QString::number( ur.ff.second) )
    .arg( QString::number( ur.u.first,'f',1) ).arg( QString::number( ur.u.second) )
    .arg( QString::number( ur.v.first,'f',1) ).arg( QString::number( ur.v.second) );
}

} // zond
