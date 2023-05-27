#include "obanal_struct.h"
#include <commons/mathtools/mnmath.h>
#include <QtDebug>

#include <cross-commons/debug/tlog.h>

#include <qtextstream.h>
//#include <map_specification.h>

bool RegionParam::isIn(float afi,float ala)const{
  meteo::GeoPoint astart = start; //!< координаты
  meteo::GeoPoint aend = end; //!< координаты
  if( astart.fi() <= afi && aend.fi() >=afi 
    && astart.la() <= ala && aend.la() >=ala){
    return true;
  } 
   return false;
  
}


bool RegionParam::operator==(const RegionParam &right) const
{
  return ( start == right.start && end == right.end); 
}

bool RegionParam::operator!=(const RegionParam &right) const
{
  return !( start == right.start && end == right.end); 
}

QDataStream& operator<<(QDataStream& in, const RegionParam& gc){
  in<<gc.start;
  in<<gc.end;
  //TODO
  /*in<<gc.start_i;
  in<<gc.start_j;
  in<<gc.kol_i;
  in<<gc.kol_j;*/
  
  return in;
}

QDataStream& operator>>(QDataStream& out, RegionParam& gc){
  out>>gc.start;
  out>>gc.end;
  //TODO
/*  out>>gc.start_i;
  out>>gc.start_j;
  out>>gc.kol_i;
  out>>gc.kol_j;
  */
return out;
}

IsoParam::IsoParam(const obanalParam & param, int svaluelevel)
{
  set(param, svaluelevel);
}

void IsoParam::fillIsotherm(int svaluelevel)
{
  int amount = 0;
  switch (svaluelevel) {
  case (0):
  case (10000):
    min = -80.;
    max = 50.;
    amount = 66;
    break;
  case (9250):
    min = -80.;
    max = 45.;
    amount = 64;
    break;
  case (8500):
    min = -50.;
    max = 35.;
    amount = 44;
    break;
  case (7000):
    min = -20.;
    max = 15.;
    amount = 18;
    break;
  case (5000):
    min = -45.;
    max = -5.;
    amount = 21;
    break;
  case (4000):
    min = -50.;
    max = -10.;
    amount = 21;
    break;
  case (3000):
    min = -70.;
    max = -25.;
    amount = 24;
    break;
  case (2500):
  case (2000):
  case (1500):
  case (1000):
  case (700):
  case (500):
  case (300):
  case (200):
  case (100):
    min = -90.;
    max = -35.;
    amount = 29;
    break;

  }
  step = 2.;
  amount = 29;
  for (int i = 0; i < amount; i++)
    fiso.append(min + step * i);
}

void IsoParam::fillIsohybse(int svaluelevel)
{
  int amount = 0;
  switch (svaluelevel) {
  default:
  case (10000):
    min = -12.;
    max = 52.;
    amount = 17;
    step = 4.;
    break;

  case (9250):
    min = 24.;
    max = 100.;
    amount = 20;
    step = 4.;
    break;
  case (8500):
    min = 112.;
    max = 192.;
    amount = 21;
    step = 4.;
    break;
  case (7000):
    min = 252.;
    max = 352.;
    amount = 26;
    step = 4.;
    break;
  case (5000):
    min = 500.;
    max = 600.;
    amount = 26;
    step = 4.;
    break;
  case (4000):
    min = 652.;
    max = 772.;
    amount = 31;
    step = 4.;
    break;
  case (3000):
    min = 856.;
    max = 976.;
    amount = 16;
    step = 8.;
    break;
  case (2500):
    min = 1000.;
    max = 1104.;
    amount = 14;
    step = 8.;
    break;
  case (2000):
    min = 1104.;
    max = 1248.;
    amount = 19;
    step = 8.;
    break;
  case (1500):
    min = 1304.;
    max = 1448.;
    amount = 19;
    step = 8.;
    break;
  case (1000):
    min = 1504.;
    max = 1704.;
    amount = 26;
    step = 8.;
    break;
  case (700):
    min = 1696.;
    max = 2000.;
    amount = 20;
    step = 16.;
    break;
  case (500):
    min = 1904.;
    max = 2208.;
    amount = 20;
    step = 16.;
    break;
  case (300):
    min = 2208.;
    max = 2608.;
    amount = 26;
    step = 16.;
    break;
  case (200):
    min = 2608.;
    max = 3008.;
    amount = 26;
    step = 16.;
    break;
  case (100):
    min = 3008.;
    max = 3408.;
    amount = 26;
    step = 16.;
    break;
  case (15000):
    min = 480.;
    max = 600.;
    step = 4.;
    amount = 31;
    break;

  }
  for (int i = 0; i < amount; i++)
    fiso.append(min + step * i);
}

void IsoParam::fillIsoParam(float amin, float amax, float astep)
{
  min = amin;
  max = amax;
  step = astep;
  int amount = (int) ((amax - amin) / step + 1);
  fiso.clear();
  for (int i = 0; i < amount; i++)
    fiso.append(min + step * i);
}



void IsoParam::set(const obanalParam & param, int svaluelevel)
{
  switch (param) {
  case TOPO_PARAM:		//высота над уровнем моря
    unit = "м.";
    name = "высота";
    min = 0.;
    max = 10000.;
    step = 100.0;
    break;
  case P_PARAM:
    name = "Давление";
    unit = "гПа";
    min = 930.;
    max = 1070.;
    //TODO if(prj_par->Par1>BOUNDARYSCALE)
    step = 5.0;
    break;
  case H_PARAM:
    unit = "гПм";
    name = "Высота геопотенциала";
    switch (svaluelevel) {
    case (10000):
      min = -12.;
      max = 52.;
      step = 4.;
      break;
    case (9250):
      min = 24.;
      max = 100.;
      step = 4.;
      break;
    case (8500):
      min = 112.;
      max = 192.;
      step = 4.;
      break;
    case (7000):
      min = 252.;
      max = 352.;
      step = 4.;
      break;
    case (5000):
      min = 500.;
      max = 600.;
      step = 4.;
      break;
    case (4000):
      min = 652.;
      max = 772.;
      step = 4.;
      break;
    case (3000):
      min = 856.;
      max = 976.;
      step = 8.;
      break;
    case (2500):
      min = 1000.;
      max = 1104.;
      step = 8.;
      break;
    case (2000):
      min = 1104.;
      max = 1248.;
      step = 8.;
      break;
    case (1500):
      min = 1304.;
      max = 1448.;
      step = 8.;
      break;
    case (1000):
      min = 1504.;
      max = 1704.;
      step = 8.;
      break;
    case (700):
      min = 1696.;
      max = 2000.;
      step = 16.;
      break;
    case (500):
      min = 1904.;
      max = 2208.;
      step = 16.;
      break;
    case (300):
      min = 2208.;
      max = 2608.;
      step = 16.;
      break;
    case (200):
      min = 2608.;
      max = 3008.;
      step = 16.;
      break;
    case (100):
      min = 3008.;
      max = 3408.;
      step = 16.;
      break;
    case (15000):
      min = 480.;
      max = 600.;
      step = 4.;
      break;
    }
    break;
  case TEND_PARAM:		//тенденция изменения давления
    name = "Тенденция изменения давления";
    unit = "гПа";
    min = -30.;
    max = 30.;
    step = 1.;
    break;
  case TROPO_PARAM:		//высота тропопаузы
    name = "Высота тропопаузы";
    unit = "гПм";
    min = 100.;
    max = 400.;
    step = 25.;
    break;
  case MAXWIND_PARAM:		//максимальный  ветер
    name = "Максимальный ветер";
    unit = "км/ч";
    min = 20.;
    max = 100.;
    step = 5.;
    break;
  case TW_PARAM:		//количество осадков
    name = "Количество осадков";
    unit = "мм";
    min = 0.;
    max = 5.;
    step = 1.;
    break;
  case T_PARAM:		//температура
    name = "Температура";
    unit = "С";
    step = 2.;
    switch (svaluelevel) {
    case (0):
    case (10000):
      min = -80.;
      max = 50.;
      break;
    case (9250):
      min = -80.;
      max = 45.;
      break;
    case (8500):
      min = -50.;
      max = 35.;
      break;
    case (7000):
      min = -20.;
      max = 15.;
      break;
    case (5000):
      min = -45.;
      max = -5.;
      break;
    case (4000):
      min = -50.;
      max = -10.;
      break;
    case (3000):
      min = -70.;
      max = -25.;
      break;
    case (2500):
    case (2000):
    case (1500):
    case (1000):
    case (700):
    case (500):
    case (300):
    case (200):
    case (100):
      min = -90.;
      max = -35.;
      break;
    }
    break;
  case TSEA_PARAM:		//температура пов. моря
    name = "Температура поверхности моря";
    unit = "С";
    min = -5.;
    max = 35.;
    step = 1.;
    break;
  case VOLNSEA_PARAM:		//волнение моря
    name = "Высота волн";
    unit = "м";
    min = 0.;
    max = 40.;
    step = 1.;
    break;
  case FF_PARAM:		//скорость ветра
    name = "Скорость ветра";
    unit = "км/ч";
    min = 20.;
    max = 100.;
    step = 5.;
    break;
  case UWIND_PARAM:		//зональная составляющая ветра
    name = "зональная составляющая скорости ветра";
    unit = "м/с";
    min = -100.;
    max = 100.;
    step = 5.;
    break;
  case VWIND_PARAM:		//меридиональная составляющая ветра
    name = "меридиональная составляющая скорости ветра";
    unit = "м/с";
    min = -100.;
    max = 100.;
    step = 5.;
    break;
  default:
    unit = "---";
    min = 0.;
    max = 1000.;
    step = 1.;
    break;
  };
  int amount = MnMath::ftoi_norm((max - min) / step) + 1;
  fiso.clear();
  for (int i = 0; i < amount; ++i) {
    fiso.append(min + step * i);
  }
}


PropertyChooseData::PropertyChooseData()
{
  scenter = 250;
  heigth = 0;
  slevelType = 1;
  slevels.append(0);
  shour = 0;
  smodel = ModelType(0);
  stypenet = STEP_0500x0500;
  param = P_PARAM;
}

void PropertyChooseData::clear()
{
  scenter = -1;
  heigth = -1;
  slevelType = -1;
  
  smodel = ModelType(0);
  shour = -1;
  stypenet = STEP_0500x0500;
  param = OB_UNKNOWN_PARAM ;
}

PropertyChooseData::PropertyChooseData(const QDateTime & dt,int aparam , int atypelevel, int alevel , 
                       int srok , int acenter , int amodel   )
{
  
  sdate = dt.date().toString("dd.MM.yyyy");
  stime = dt.time().toString("hh:mm");

  scenter = acenter;
  if(0 == alevel ){  slevelType = 1;} else {  slevelType = 0;}
  slevelType = atypelevel;
  slevels.append(alevel);
  shour = srok;
  smodel = static_cast<ModelType> (amodel);
  stypenet = STEP_0500x0500;
  param = static_cast<obanalParam> (aparam);

}

// bool PropertyChooseData::fillShoursList(int shour, int interval)
// {
//   shours.clear();
//   int kol_prognoz = shour / interval + 1;
//   int ostatok_prognoz = shour % interval;

//   if (kol_prognoz < 1)
//     return false;
//   for (int i = 0; i < kol_prognoz; ++i) {
//     shours.append(i * (interval));
//   }
//   if (ostatok_prognoz)
//     shours.append(shours.last() + ostatok_prognoz);

//   return true;
// }

bool PropertyChooseData::getSteps(meteo::GeoPoint* steps)
{
  return getSteps(stypenet, steps);
}

bool PropertyChooseData::getSteps(NetType atypenet, meteo::GeoPoint* steps)
{
  double step_fi = 0., step_la = 0.;
  switch (atypenet) {
  case STEP_500x500:		// через 5 градусов по широте и долготе
    step_fi = MnMath::deg2rad(5.);
    step_la = MnMath::deg2rad(5.);
    break;
  case STEP_250x250:		// через 2.5 градуса по широте и долготе
    step_fi = MnMath::deg2rad(2.5);
    step_la = MnMath::deg2rad(2.5);
    break;
  case STEP_125x125:		// через 1.25 градуса по широте и долготе
    step_fi = MnMath::deg2rad(1.25);
    step_la = MnMath::deg2rad(1.25);
    break;
  case STEP_100x100:		// через 1 градус по широте и долготе
    step_fi = MnMath::deg2rad(1.);
    step_la = MnMath::deg2rad(1.);
    break;
  case STEP_0625x0625:		// через 0.625 градуса по широте и долготе
    step_fi = MnMath::deg2rad(0.625);
    step_la = MnMath::deg2rad(0.625);
    break;
  case STEP_2983x100:		// через 2.983 градуса по широте и долготе
    step_fi = MnMath::deg2rad(2.983);
    step_la = MnMath::deg2rad(1.);
    break;
  case STEP_0500x0500:		// через 0.5 градуса по широте и долготе
    step_fi = MnMath::deg2rad(0.5);
    step_la = MnMath::deg2rad(0.5);
    break;
  case STEP_0100x0100:		// через 0.1 градус по широте и долготе
    step_fi = MnMath::deg2rad(0.1);
    step_la = MnMath::deg2rad(0.1);
    break;
  default:
    return false;
    break;
  }
  steps->setLat(step_fi);
  steps->setLon(step_la);
  return true;
}


void TFieldKey::printDebug()
{

  qDebug() << "_level" << _level;
  qDebug() << "_hour" << _hour;
  qDebug() << "_date" << _date;
  qDebug() << "_param" << _param;
  qDebug() << "isfact" << _isfact;

}

QDataStream &operator>>(QDataStream &in, TFieldKey& key)
{
  in >> key._date;
  in >> key._center;
  in >> key._levelType;
  in >> key._level;
  in >> key._hour;
  int val;
  in >> val;
  key._param = obanalParam(val);
  in >> key._isfact;

  return in;
}

QDataStream &operator<<(QDataStream &out, const TFieldKey& key)
{
  out << key._date;
  out << key._center;
  out << key._levelType;
  out << key._level;
  out << key._hour;
  out << (int)key._param;
  out << key._isfact;

  return out;
}

QDateTime PropertyChooseData::dateTimePrognoz() const
{
  QDate date = date.fromString(sdate, Qt::ISODate);
  QTime time = time.fromString(stime, "hh:mm");
  int hour = shour;
  return QDateTime(date, time).addSecs(hour * 60 * 60);
}

QDateTime PropertyChooseData::dateTime() const
{
  QDate date = date.fromString(sdate, Qt::ISODate);
  QTime time = time.fromString(stime, "hh:mm");
  return QDateTime(date, time);
}


QString PropertyChooseData::getParamName() const
{
  QString ret_val;
  switch (param) {
    case H_PARAM:
      ret_val = "Высота геопотенциала";
    break;
    case T_PARAM:
      ret_val = "Температура";
    break;
    case TW_PARAM:
      ret_val = "Количество осадков";
    break;
    case FF_PARAM:
      ret_val = "Скорость ветра";
    break;
    case P_PARAM:
      ret_val = "Атм. давление";
    break;
    case TEND_PARAM:
      ret_val = "Тенденция изменения давления";
    break;
    case TSEA_PARAM:
      ret_val = "Температура пов. моря";
    break;
    case VOLNSEA_PARAM:
      ret_val = "Волнение моря";
    break;
    case TROPO_PARAM:
      ret_val = "Высота тропопаузы";
    break;
    case MAXWIND_PARAM:
      ret_val = "Максимальный  ветер";
    break;
    case UWIND_PARAM:
      ret_val = "Зональная составляющая ветра";
    break;
    case VWIND_PARAM:
      ret_val = "Меридиональная составляющая ветрая";
    break;
    case TOPO_PARAM:
      ret_val = "Высота над уровнем моря";
    break;
    default:
    break;
  }
  return ret_val;
}

QDataStream &operator>>(QDataStream &in, PropertyChooseData& prop)
{
 // qDebug()<<in;
  in >> prop.sdate;
  in >> prop.stime;
  in >> prop.scenter_name;
  in >> prop.scenter;
  in >> prop.heigth;
  in >> prop.slevelType;
  int val;
  in >> val;
  prop.slevels[0] = val;
  
  in >> val;
  prop.smodel = static_cast<ModelType>(val);

  in >> prop.shour;

  in >> val;
  prop.stypenet = static_cast<NetType>(val);

  in >> val;
  prop.param = static_cast<obanalParam>(val);
    
  // debug_log << "date=" << prop.sdate << "time =" << prop.stime 
  // 	      << "center=" << prop.scenter << "levelType=" << prop.slevelType << "level=" << prop.slevels.at(0)
  // 	      << "hour=" << prop.shour << "param=" << prop.param << "\n";

  return in;
}

QDataStream &operator<<(QDataStream &out, const PropertyChooseData& prop)
{
  out << prop.sdate;
  out << prop.stime;
  out << prop.scenter_name;
  out << prop.scenter;
  out << prop.heigth;
  out << prop.slevelType;
  out << prop.slevels.at(0);
  out << (int)(prop.smodel);
  out << prop.shour;
  out << (int)(prop.stypenet);
  out << (int)(prop.param);

 // debug_log << "date=" << prop.sdate << "time =" << prop.stime 
 // 	      << "center=" << prop.scenter << "levelType=" << prop.slevelType << "level=" << prop.slevels.at(0)
 // 	   << "hour=" << prop.shour << "param=" << prop.param << "\n";
  return out;
}

QTextStream &operator>>(QTextStream &in, PropertyChooseData& prop)
{
  char c;
  prop.beginDt = QDateTime::fromString(in.readLine(), Qt::ISODate);
  prop.endDt = QDateTime::fromString(in.readLine(), Qt::ISODate);
  prop.sdate = in.readLine();
  prop.stime = in.readLine();
  prop.scenter_name = in.readLine();
  in >> prop.scenter >> c;
  in >> prop.heigth >> c;
  in >> prop.slevelType >> c;
  int cnt;
  int val;
  in >> cnt >> c;
  for (int i = 0; i < cnt; i++) {
    in >> val >> c;
    prop.slevels.append(val);
  }
  in >> val >> c;
  prop.smodel = static_cast<ModelType>(val);
  in >> prop.shour >> c;
  in >> val >> c;
  prop.stypenet = static_cast<NetType>(val);
  in >> val >> c;
  prop.param = static_cast<obanalParam>(val);

  return in;
}

QTextStream &operator<<(QTextStream &out, const PropertyChooseData& prop)
{
  out << prop.beginDt.toString(Qt::ISODate) << '\n';
  out << prop.endDt.toString(Qt::ISODate) << '\n';
  out << prop.sdate << '\n';
  out << prop.stime << '\n';
  out << prop.scenter_name << '\n';
  out << prop.scenter << '\n';
  out << prop.heigth << '\n';
  out << prop.slevelType << '\n';
  out << prop.slevels.count() << '\n';
  for (int i = 0; i < prop.slevels.count(); i++) {
    out << prop.slevels.at(i) << '\n';
  }
  out << prop.smodel << '\n';
  out << prop.shour << '\n';
  out << prop.stypenet << '\n';
  out << prop.param << '\n';

  return out;
}

  
NetType obanal::netTypeFromStep(int di, bool* ok /*= nullptr*/)
{
  NetType nt = STEP_0500x0500; //шаг по умолчанию
  if (nullptr != ok) {
    *ok = true;
  }
  
  switch (di) {
  case 5000:
    nt = STEP_500x500;
    break;
  case 2500:
    nt = STEP_250x250;
    break;
  case 1250:
    nt = STEP_125x125;
    break;
  case 1000:
    nt = STEP_100x100;
    break;
  case 625:
    nt = STEP_0625x0625;
    break;
  case 2983:
    nt = STEP_2983x100;
    break;
  case 500:
    nt = STEP_0500x0500;
    break;
  case 100:
    nt = STEP_0100x0100;
    break;
  default:
    if (nullptr != ok) {
      *ok = false;
    }

    break;
  }
 
  return nt;
}

//ближайший тип по широте, для тех, что с равным шагом
NetType obanal::nearNetTypeFromStep(int di)
{
  NetType nt = STEP_0500x0500; //шаг по умолчанию

  if (di < 100 + (500 - 100)/2) {
    nt = STEP_0100x0100;
  } else if (di < 500 + ( 625 -  500)/2) {
    nt = STEP_0500x0500;
  } else if (di < 625 + (1000 -  625)/2) {
    nt = STEP_0625x0625;
  } else if (di < 1000 + (1250 - 1000)/2) {
    nt = STEP_100x100;
  } else if (di < 1250 + (2500 - 1250)/2) {
    nt = STEP_125x125;
  } else if (di < 2500 + (5000 - 2500)/2) {
    nt = STEP_250x250;
  } else {
    nt = STEP_500x500;
  }
  
  return nt;
}


QString obanal::netTypetoStr(NetType type)
{
  QString str;
  switch (type) {
  case STEP_500x500:
    str = "5.00x5.00";
    break;
  case STEP_250x250:
    str = "2.50x2.50";
    break;
  case STEP_125x125:
    str = "1.25x1.25";
    break;
  case STEP_100x100:
    str = "1.00x1.00";
    break;
  case STEP_0625x0625:
        str = "0.625x0.625";
    break;
  case STEP_2983x100:
    str = "2.983x1.00";
    break;
  case STEP_0500x0500:
    str = "0.500x0.500";
    break;
  case STEP_0100x0100:
    str = "0.100x0.100";
    break;
  case GENERAL_NET:
    str = "";
    break;
  default: {
    str = QObject::tr("Не указан");
  }
  }
  
  return str;
}
