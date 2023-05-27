#include "esimoparam.h"

#include <commons/meteo_data/tmeteodescr.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
  bool set_P0076(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0399(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0237(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0254(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0255(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0068(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0595(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0248(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0596(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0504(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0477(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0460(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0469(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0248(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0247(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0414(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0433(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0992(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P0964(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P1132(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P1135(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P1137(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
  bool set_P1138(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
    

}


meteo::EsimoParam::EsimoParam()
{
  init();
}

meteo::EsimoParam::~EsimoParam()
{
}

void meteo::EsimoParam::init()
{
  _funcs.insert("P0076_01", meteo::set_P0076);
  _funcs.insert("P0076_00", meteo::set_P0076);
  _funcs.insert("P0399_00", meteo::set_P0399);
  _funcs.insert("P0237_00", meteo::set_P0237);
  _funcs.insert("P1134_00", meteo::set_P0237);
  _funcs.insert("P0254_00", meteo::set_P0254);
  _funcs.insert("P0255_00", meteo::set_P0255);
  _funcs.insert("P0068_00", meteo::set_P0068);
  _funcs.insert("P0068_03", meteo::set_P0068);
  _funcs.insert("P0595_00", meteo::set_P0595);
  _funcs.insert("P0248_00", meteo::set_P0248);
  _funcs.insert("P0249_00", meteo::set_P0248);
  _funcs.insert("P0993_00", meteo::set_P0248);
  _funcs.insert("P0994_00", meteo::set_P0248);
  _funcs.insert("P0596_00", meteo::set_P0596);
  _funcs.insert("P0504_00_M", meteo::set_P0504);
  _funcs.insert("P0477_00", meteo::set_P0477);
  _funcs.insert("P0460_00", meteo::set_P0460);
  _funcs.insert("P0469_00", meteo::set_P0469);
  _funcs.insert("P0061_00", meteo::set_P0247);
  _funcs.insert("P0247_00", meteo::set_P0247);
  _funcs.insert("P0250_00", meteo::set_P0247);
  _funcs.insert("P0062_00", meteo::set_P0247);
  _funcs.insert("P0251_00", meteo::set_P0247);  
  _funcs.insert("P0057_00", meteo::set_P0247);
  _funcs.insert("P0414_00", meteo::set_P0414);
  _funcs.insert("P0415_00", meteo::set_P0414);
  _funcs.insert("P0416_00", meteo::set_P0414);
  _funcs.insert("P0433_00", meteo::set_P0433);
  _funcs.insert("P0969_00", meteo::set_P0433);
  _funcs.insert("P0970_00", meteo::set_P0433);
  _funcs.insert("P0972_00", meteo::set_P0433);
  _funcs.insert("P0974_00", meteo::set_P0433);
  _funcs.insert("P0976_00", meteo::set_P0433);
  _funcs.insert("P0973_00", meteo::set_P0433);
  _funcs.insert("P0971_00", meteo::set_P0433);
  _funcs.insert("P0992_00", meteo::set_P0992);
  _funcs.insert("P0964_00", meteo::set_P0964);
  _funcs.insert("P0965_00", meteo::set_P0964);
  _funcs.insert("P1132_00", meteo::set_P1132);
  _funcs.insert("P1139_00", meteo::set_P1132);
  _funcs.insert("P1135_00", meteo::set_P1135);
  _funcs.insert("P1137_00", meteo::set_P1137);
  _funcs.insert("P1138_00", meteo::set_P1138);
  // _funcs.insert("", meteo::set_);
  // _funcs.insert("", meteo::set_);
  // _funcs.insert("", meteo::set_);
  // _funcs.insert("", meteo::set_);
  // _funcs.insert("", meteo::set_);
  // _funcs.insert("", meteo::set_);

}


bool meteo::EsimoParam::fillMeteo(const QString& esimoname, descr_t descr, float val, TMeteoData* data)
{
  if (!_funcs.contains(esimoname)) {
    data->add(descr, "", val, control::NO_CONTROL);
    return true;
  }

  return _funcs.value(esimoname)(esimoname, descr, val, data);
}
    

//гориз видимость, км в м
//"P0076_01"
//"P0076_00"
bool meteo::set_P0076(const QString&, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = val * 1000;
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

// облачность общая, баллы в % "N"
// "P0399_00"
bool meteo::set_P0399(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }

  float cval = BAD_METEO_ELEMENT_VAL;
  switch (int(val)) {
  case 0: cval =  0;  break;
  case 1: cval = 10;  break;
  case 2: cval = 25;  break;
  case 3: cval = 40;  break;
  case 4: cval = 50;  break;
  case 5: cval = 60;  break;
  case 6: cval = 75;  break;
  case 7: cval = 90;  break;
  case 8: cval = 100; break;
  case 9: cval = 113; break;
  }

  if (cval != BAD_METEO_ELEMENT_VAL) {
    data->add(descr, "", cval, control::NO_CONTROL);
  }
  
  return true;
}

//ВНГО, код в м  "h"
//"P0237_00"
//"P1134_00"
bool meteo::set_P0237(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = BAD_METEO_ELEMENT_VAL;
  if (val < 51)                   { cval = 30.*val; }
  else if (val >= 56 && val < 81) { cval = (val-50.)*300.; }
  else if (val >= 81 && val < 90) { cval = (val-80.)*1500.+9000.; }
  else {
    switch (int(val)) {
    case 90: cval =   10.;  break;
    case 91: cval =   50.;  break;
    case 92: cval =  100.;  break;
    case 93: cval =  200.;  break;      
    case 94: cval =  300.;  break;
    case 95: cval =  600.;  break;
    case 96: cval = 1000.;  break;
    case 97: cval = 1500.;  break;
    case 98: cval = 2000.;  break;
    case 99: cval = 2500.;  break;
    }
  }

  if (cval != BAD_METEO_ELEMENT_VAL) {
    data->add(descr, "", cval, control::NO_CONTROL);
  }
  
  return true;
}

//TODO не понятно как
//скорость обледен, см/ч в код ?
//"P0254_00"
bool meteo::set_P0254(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = val;
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}


//сплоченность льда, балл в код
//"P0255_00"
bool meteo::set_P0255(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = BAD_METEO_ELEMENT_VAL;
  if (val < 3) {
    cval = 2;
  } else if (val <= 6) {
    cval = 3;
  } else if (val <= 8) {
    cval = 4;
  } else {
    cval = 5;
  }  

  if (cval != BAD_METEO_ELEMENT_VAL) {
    data->add(descr, "", cval, control::NO_CONTROL);
  }
  
  return true;
}

//скорость течения см/с в м/с c0c0
//"P0068_00"
//"P0068_03"
bool meteo::set_P0068(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = val / 100.;
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

// видимость код в м
//"P0595_00"
bool meteo::set_P0595(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = BAD_METEO_ELEMENT_VAL;
  if (val < 51)                   { cval = 100.*val; }
  else if (val >= 56 && val < 81) { cval = (val-50.)*1000.; }
  else if (val >= 81 && val < 90) { cval = ((val-80.)*5. + 30)*1000.; }
  else {
    switch (int(val)) {
    case 90: cval =   10.;  break;
    case 91: cval =   50.;  break;
    case 92: cval =  200.;  break;
    case 93: cval =  500.;  break;      
    case 94: cval = 1000.;  break;
    case 95: cval = 2000.;  break;
    case 96: cval = 4000.;  break;
    case 97: cval =10000.;  break;
    case 98: cval =15000.;  break;
    case 99: cval =50000.;  break;
    }
  }

  if (cval != BAD_METEO_ELEMENT_VAL) {
    data->add(descr, "", cval, control::NO_CONTROL);
  }

  return true;
}


//направление код в градус как dw2
//"P0596_00"
bool meteo::set_P0596(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  const QList<float> vallist = QList<float>() << 0 << 45 << 90 << 135 << 180 << 225 << 270 << 315 << 360 << -9999;
  if (val >= vallist.size()) {
    return false;
  }
  
  float cval = vallist.at(val);
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}



//ширина зоны прибоя, код в м B1
//"P0504_00_M"
bool meteo::set_P0504(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  const QList<float> vallist = QList<float>() << 0 << 1 << 6 << 11 << 16 << 26 << 51 << 101 << 201 << 300;
  if (val >= vallist.size()) {
    return false;
  }
  
  float cval = vallist.at(val);
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

//скоростьдрейф. льда
//"P0477_00" Fd см/с в код
bool meteo::set_P0477(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }

  float cval = 0;
  
  if (val > 0 && val < 0.14) {
    cval = 1;
  } else if (val < 0.28) {
    cval = 2;
  } else if (val < 0.83) {
    cval = 3;
  } else {
    cval = 4;
  }
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

//измен темп код в С dT
//"P0460_00"
bool meteo::set_P0460(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }

  QString strelem = QString::number(val, 'f', 0);
  
  int sign = 1;
  if (strelem.size() == 2 && ("1" == strelem.left(1) || "M" == strelem.left(1)))  {
    sign = -1;
  }

  float cval = strelem.right(1).toFloat();  
  if (cval <= 4) cval += 10;

  cval *= sign;
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

//ВВГО сотни м в м h_
//"P0469_00"
bool meteo::set_P0469(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = val*100;
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

//диаметр отложений RR32, RR34
//"P1132_00"
//"P1139_00"
bool meteo::set_P1132(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }

  float cval = BAD_METEO_ELEMENT_VAL;
  
  if      (val < 56)  { cval = val; }
  else if (val >= 56 && val < 91) { cval = 10.0*(val-50.); }
  else if (val >= 91 && val < 97) { cval = 0.1*(val-90.);  }
  else if (val == 97)             { cval = 0.; }
  else if (val == 98)             { cval = 450.; }

  if (cval != BAD_METEO_ELEMENT_VAL) {
    data->add(descr, "", cval, control::NO_CONTROL);
  }
  
  return true;
}


//"P1135_00", код в м  "hshs"
bool meteo::set_P1135(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  const QList<float> vallist = QList<float>() << 0 << 50 << 100 << 200 << 300 << 600 << 1000 << 1500 << 2000 << 2500;
  if (val >= vallist.size()) {
    return false;
  }
  
  float cval = vallist.at(val);
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;  
}

//продолж явления, код в ч "tt906"
//"P1137_00"
bool meteo::set_P1137(const QString&, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = BAD_METEO_ELEMENT_VAL;

  if (val <= 60) {
    cval = val * 0.1;
  } else {
    cval = (val - 60) + 6;
  }  

  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

//продолж явления, мин в ч
//"P1138_00"
bool meteo::set_P1138(const QString&, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = val / 60;
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

//-----

//волнение ветровое, 1 система
//"P0248_00" период
//"P0249_00" высота 0,5м в м
//                   2 система
//"P0994_00" период
//"P0993_00" высота 0.5м в м
bool meteo::set_P0248(const QString& esimoname, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }

  int idx = 0;
  if (esimoname == "P0993_00" || esimoname == "P0994_00") {
    idx = 100;
  }
  
  float cval = val;
  if (esimoname == "P0249_00" || esimoname == "P0993_00") {
    cval *= 2;
  }
  
  data->add(descr, TMeteoParam("", cval, control::NO_CONTROL), idx);
  
  return true;
}

//зыбь
//"P0061_00" dw1s
//"P0247_00" dw2s
//"P0250_00" Pw1s
//"P0062_00" Pw2s
//"P0251_00" Hw1s
//"P0057_00" Hw2s
bool meteo::set_P0247(const QString& esimoname, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }

  int idx = 0;

  if (esimoname == "P0247_00" ||
      esimoname == "P0062_00" ||
      esimoname == "P0057_00") {
    idx = 100;
  }
  
  float cval = val;
  
  data->add(descr, TMeteoParam("", cval, control::NO_CONTROL), idx);
  
  return true;
}



//форма облачности
//"P0414_00" CL
//"P0415_00" CM
//"P0416_00" CH
bool meteo::set_P0414(const QString& esimoname, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  float cval = val;
  if (esimoname == "P0414_00") {
    cval += 30;
  } else if (esimoname == "P0415_00") {
    cval += 20;
  } else if (esimoname == "P0416_00") {
    cval += 10;
  }
  
  data->add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}


//солнечное сияние 0.1ч в ч, срок 1ч SSS
//"P0433_00"
//радиация 
// "P0969_00" 1ч  *(-1)
// "P0970_00" 1ч
// "P0972_00" 1ч
// "P0974_00" 1ч
// "P0976_00" 1ч
// "P0973_00" 24ч
// "P0971_00" 24ч
bool meteo::set_P0433(const QString& esimoname, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }

  TMeteoData& child = data->addChild();

  int GGp = -1;
  float cval = val;
  
  if (esimoname == "P0433_00") {
    cval *= 10;
  } else if (esimoname == "P0969_00") {
    cval *= -1;
  }

  if (esimoname == "P0973_00" || esimoname == "P0971_00") {
    GGp = -24;
  }
  
  child.add(TMeteoDescriptor::instance()->descriptor("GGp"), "", GGp, control::NO_CONTROL);
  child.add(descr, "", cval, control::NO_CONTROL);
  
  return true;
}

//макс ветер при порыве за 12 ч
//"P0992_00"
bool meteo::set_P0992(const QString& , descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  TMeteoData& child = data->addChild();

  child.add(TMeteoDescriptor::instance()->descriptor("GGp"), "", -12, control::NO_CONTROL);
  child.add(descr, "", val, control::NO_CONTROL);
  
  return true;
}

//"P0964_00" осадки
//"P0965_00" tR для осадков
bool meteo::set_P0964(const QString& esimoname, descr_t descr, float val, TMeteoData* data)
{
  if (nullptr == data) {
    return false;
  }
  
  if (esimoname == "P0964_00") {
    TMeteoData& child = data->addChild();
    child.add(descr, "", val, control::NO_CONTROL);
  } else if (esimoname == "P0965_00") {
    QList<TMeteoData*> childs = data->findChilds(TMeteoDescriptor::instance()->descriptor("R"));
    if (childs.count() == 1) {
      childs.at(0)->add(descr, "", -val, control::NO_CONTROL);
    } else {
      warning_log << QObject::tr("Значений осадков") << childs.count();
    }    
  }
  
  return true;
}



