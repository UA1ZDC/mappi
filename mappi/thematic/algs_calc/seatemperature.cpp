#include "seatemperature.h"

#include "channel.h"
#include "themformat.h"
#include "datastore.h"

#include <mappi/settings/mappisettings.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/zond/diagn_func.h>

#include <qimage.h>
#include <qfile.h>



namespace {

mappi::to::ThemAlg* createSeaTempr(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new mappi::to::SeaTempr(type,them_name, ds);
}

static const bool res8 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kSeaTempr, createSeaTempr);
static const bool resCloudAlt = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kCloudAlt, createSeaTempr);
}

using namespace mappi;
using namespace to;
SeaTempr::SeaTempr(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<DataStore> &ds)
  :ThemAlg(type,them_name,ds)
{
  _themType = type;

  fillPalette();
}


SeaTempr::~SeaTempr()
{
}


//! Обработка данных спутника
bool SeaTempr::process()
{
  data_.clear();
  bitmap_.clear();
  const auto chs = channels();
  if(chs.size() == 0) return false;

  if(config_.vars_size() < chs.size() || false == isValid(config_))
    return false;
  QSharedPointer<Channel> A1 =  _ch.value("A1");
  QSharedPointer<Channel> A2 =  _ch.value("A2");
  QSharedPointer<Channel> T3 =  _ch.value("T3");
  QSharedPointer<Channel> T4 =  _ch.value("T4");
  QSharedPointer<Channel> T5 =  _ch.value("T5");
  if(T5.isNull()||T4.isNull()||T3.isNull()||A1.isNull()||A2.isNull() )
  {
    error_log << QObject::tr("Не произведена привязка данных");
    return false;
  }
  if(false == initProjection(T5) || nullptr == projection_)
  {
    error_log << QObject::tr("Проекция не создана");
    return false;
  }
  if(false == initLandMask() || nullptr == landmask_)
  {
    error_log << QObject::tr("Не удалось загрузить маску суша/вода");
    return false;
  }

  int sz = T5->size();
  data_.resize( sz );

  float value = INVALID_VALUE;
  meteo::GeoPoint geoCoord;

  int day = 1;
  if(true == projection_->X2F_one(0, &geoCoord) ){
    day = landmask_->day(geoCoord,projection_->date(0));
  }

  for (int idx = 0; idx < sz; idx++) {

    float t5 = T5->at(idx);
    float t4 = T4->at(idx);
    float t3 = T3->at(idx);
    float a1 = A1->at(idx);
    float a2 = A2->at(idx);

    if (qFuzzyCompare( t5, INVALID_VALUE)
        ||qFuzzyCompare( t4, INVALID_VALUE)
        ||qFuzzyCompare( t3, INVALID_VALUE)
        ||qFuzzyCompare( a1, INVALID_VALUE)
        ||qFuzzyCompare( a2, INVALID_VALUE) )
    {
      data_[idx] = INVALID_VALUE;
      continue;
    }



    if(false == projection_->X2F_one(idx, &geoCoord) ){
      error_log << QObject::tr("Ошибка расчета координат изображения")<<sz<< idx<< geoCoord.toString();
      return false;
    }
    if(!geoCoord.isValid()){
      debug_log << geoCoord.toString();
      return false;
    }
    int landmask = landmask_->land(geoCoord);//1 for land mask , 0 for sea mask
    double bt = projection_->bt(idx);
    double sectheta = 1./::cos(bt);
    ThemAlg::pixelType pixtype = cloudTestWithLandMask(a1,a2,t3,t4,landmask);
    if(1 == landmask)
    {
      //   value  = LST_process(t4, t5, sectheta, day)-273.15;
      value  = LST_process_noaa(t4, t5, sectheta, day);
    } else {
      //  value  = SST_process(t4, t5, sectheta, day)-273.15;
      value  = SST_process_noaa(t4, t5, sectheta, day);
    }

    if (qFuzzyCompare( value, INVALID_VALUE))
    {
      data_[idx] = INVALID_VALUE;
      continue;
    }


    //    SST_process_noaa(  t4,  t5, 1./::cos(bt), day);
    //    LST_process_noaa(  t4,  t5, 1./::cos(bt), day);



    if( (value < 0 ) && mappi::conf::kCloudAlt == type() ){//определяем высоту по температуре)))
      //конечно, нужен ближайший зонд, но пока возьмем стандартную атмосферу
      if(ThemAlg::CLOUD == pixtype)
      {
        if(value < -60.) {
          value = 10000.;
        } else{
          float t0 = value +273.15;
          for(int h = 0; h < 15000; h+=100){
            float t = t0; float p = 1000;
            zond::SA81_P( float(h), &t, &p );
            if(t <= t0){
              value = h;
              break;
            }
          }
        }
      } else {
        value = -9999.;
      }
    }

    data_[idx] = value;
  }
  return !data_.isEmpty();
}



float SeaTempr::SST_process_noaa( double t4, double t5, double sectheta, int daytime)
{
  float value = 273.15;
  if(1 == daytime){  // Возвращаемые значения: 1 - день, 0 - ночь.
    //день
    //море
    value = them_coef_.seadaytempra0() + them_coef_.seadaytempra1()*t4 +
            them_coef_.seadaytempra2()*(t4-t5) + them_coef_.seadaytempra3()*(t4-t5)*(sectheta-1.);
  } else {
    //ночь
    //море
    value = them_coef_.seanighttempra0() + them_coef_.seanighttempra1()*t4 +
            them_coef_.seanighttempra2()*(t4-t5) + them_coef_.seanighttempra3()*(t4-t5)*(sectheta-1.);
  }
  return value;
}


float SeaTempr::LST_process_noaa( double t4, double t5, double , int daytime)
{
  float value = 273.15;
  float e4 = 0.96;
  float e5 = 0.97;
  float d_eps = e4-e5;
  float eps = (e4 + e5)*0.5;
  float eps1 = (1-eps)/eps;
  float eps2 = d_eps/(eps*eps);
  if(1 == daytime){  // Возвращаемые значения: 1 - день, 0 - ночь.
    //день
    //суша
    float a = them_coef_.landdaytempra0();

    float b = 1+ them_coef_.landdaytemprb0()*eps1-them_coef_.landdaytemprb1()*eps2;
    float c =  them_coef_.landdaytemprc0() + them_coef_.landdaytemprc1()*eps1
               +them_coef_.landdaytemprc2()*eps2;
    value = a + b * (t4+t5)*0.5 + c* (t4-t5)*0.5;


  } else {
    //ночь
    //суша
    float a = them_coef_.landnighttempra0();
    float b = 1+ them_coef_.landnighttemprb0()*eps1-them_coef_.landnighttemprb1()*eps2;
    float c =  them_coef_.landnighttemprc0() + them_coef_.landnighttemprc1()*eps1
               +them_coef_.landnighttemprc2()*eps2;
    value = a + b * (t4+t5)*0.5 + c* (t4-t5)*0.5;
  }
  return value - 273.15;
}


// NOAA NESDIS STAR - GOES LST Algorithm Theoretical Basis
double SeaTempr::LST_process(double T5, double T6, double sectheta, int daytime)
{

  //Zhao Liang Li and Francois Becker - Feasibility of land surface temperature and emissivity determination from AVHRR data
  double eps5_day = 0.978;
  double eps6_day = 0.994;
  double eps5_night = 0.957;
  double eps6_night = 0.976;
  // Или рассчитать на основе NDVI как предложено Jose A.Sobrinoa, Juan C.Jimenez-Munoza, Leonardo Paolinib
  // Land surface temperature retrieval from LANDSAT TM 5
  // Zhengming Wan and Jeff Dozier - A Generalized Split Window Algorithm for Retrieving Land-Surface Temperature from Space
  double A0 = 1.274;
  // Или использовать коэфициент A0=(NDVI-NDVI_bs)/(NDVIv-NDVI_bs), где NDVI_bs это минимальное
  // значение для голой земли, а NDVIv - максимальное значение для полностью зеленого пикселя
  double A1 = 1;
  double A2 = 0.15616;
  double A3 = -0.482;
  double A4 = 6.26;
  double A5 = 3.98;
  double A6 = 38.33;
  double A7 = -3.36; //TODO: требуется моделирование

  double eps_mean, eps_delta, eps_norm;
  if (daytime) {
    eps_mean = (eps5_day + eps6_day) / 2;
    eps_delta = (eps5_day - eps6_day);
  } else {
    eps_mean = (eps5_night + eps6_night) / 2;
    eps_delta = (eps5_night - eps6_night);
  }
  eps_norm = (1 - eps_mean) / eps_mean;
  double P = (A1 + A2 * eps_norm + A3 * eps_delta / (eps_mean * eps_mean)) / 2;
  double M = (A4 + A5 * eps_norm + A6 * eps_delta / (eps_mean * eps_mean)) / 2;
  double value =
      A0 +
      P * (T5 + T6) +
      M * (T5 - T6) +
      A7 * (T5 - T6) * (sectheta - 1);
  return value;

}


double SeaTempr::SST_process(double T5, double T6, double sectheta, int /*daytime*/)
{
  //А.И.Алексанин, С.Е.Дьяков - Калибровка ИК-каналов радиометра МСУ-МР спутника Метор-М
  double b0 = 0.67;
  double b1 = 1.86;
  double b2 = 2.08;
  double b3 = 0.97;
  double b4 = -3.36;
  double b5 = 0.39;
  double b6 = 4.58;

  // bool SST_process(QVector <uchar> &_data, QSharedPointer <Channel> &Band5, QSharedPointer <Channel> &Band6, double theta = 1) {
  return
      b0 +
      b1 * T5  +
      b2 * (T5 - T6) +
      b3 * (T5 + T6) * (sectheta - 1.) +
      b4 * (T5 - T6) * (sectheta - 1.) +
      b5 * (sectheta - 1.) +
      b6 * (sectheta - 1.) * (sectheta - 1.);
}

