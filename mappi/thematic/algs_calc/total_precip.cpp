#include "total_precip.h"
#include "channel.h"
#include "themformat.h"

#include <qimage.h>
#include <qfile.h>


using namespace mappi;
using namespace to;

namespace {

mappi::to::ThemAlg* createTotalPrecip(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new TotalPrecip(type,them_name, ds);
}

static const bool res8 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kTotalPrecip, createTotalPrecip);
//static const bool resNdvi = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kNdvi, createTotalPrecip);
}

TotalPrecip::TotalPrecip(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<DataStore> &ds)
  :ThemAlg(type,them_name,ds)
{
}

TotalPrecip::~TotalPrecip()
{
}

float TotalPrecip::waterEmissivity(float mu)//The e missivity of water
{
  return  0.1824 + 0.9048*mu - 0.6221*mu*mu;

}

float TotalPrecip::emissivity(float t1, float t2, float t3, float mu)//retrieved emissivity at 23.8 GHz
{
  float a = 1.84 - 0.723*mu;
  float b = -0.00088;
  float c = 0.0066 + 0.0029*mu;
  float d = -0.00926;
  return a+b*t1+c*t2+d*t3;
}

//! Обработка данных спутника
bool TotalPrecip::process()
{
  data_.clear();
  bitmap_.clear();
  const auto chs = channels();
  if(chs.size() == 0) return false;

  if(config_.vars_size() < chs.size() || false == isValid(config_))
    return false;

  QSharedPointer<Channel> T23 =  _ch.value("T23");
  QSharedPointer<Channel> T31 =  _ch.value("T31");
  QSharedPointer<Channel> T50 =  _ch.value("T50");
  if(T23.isNull()||T31.isNull()||T50.isNull())
  {
    error_log << QObject::tr("Не произведена привязка данных");
    return false;
  }
  if(false == initProjection(T23) || nullptr == projection_)
  {
    error_log << QObject::tr("Проекция не создана");
    return false;
  }
  int sz = T23->size();
  debug_log << QObject::tr("Размеры каналов: T23(%1) T31(%2) T50(%3) -> T23")
                    .arg(T23->size())
                    .arg(T31->size())
                    .arg(T50->size());
  data_.resize( sz );
  //float value = INVALID_VALUE;

  for (int idx = 0; idx < sz; idx++)
  {
    double cosza = 0.5; ;
    cosza =  cos(projection_->bt(idx));//косинус угла сканирования радиометра
    //bool ok =  projection_->zenithCos(idx, &cosza);//косинус зенитного угла радиометра
//    if(false == ok) {
//      data_[idx] = INVALID_VALUE;
//      continue;
//    }
    float t23 = T23->at(idx);
    float t31 = T31->at(idx);
    float t50 = T50->at(idx);

    if (qFuzzyCompare( t23, INVALID_VALUE) || qFuzzyCompare( t31, INVALID_VALUE) || qFuzzyCompare( t50, INVALID_VALUE) )
    {
      data_[idx] = INVALID_VALUE;
      debug_log << QObject::tr("Неверное значение по индексу %1: T23(%2) T31(%3) T50(%4)")
                        .arg(idx)
                        .arg(t23)
                        .arg(t31)
                        .arg(t50);
      continue;
    }




    float landT = 2.9079e2 - (8.5059e-1 - 1.9821e-3* t23)*t23
                   + (6.1433e-1 - 2.3579e-3 * t31)*t31
                   - (1.1493 - 5.4709e-3 * t50)*t50
                   - 1.50e1*(cosza - 5.40e-1);
//    float eps = waterEmissivity(cosza);
//    float eps31 = eps;
//    float eps23 = eps;
    float lnt1 = log(landT- t23);
    float lnt2 = log(landT- t31);

//    float kv23 = 4.80423e-3; // water vapor mass absorption coefficient 23.8 GHz
//    float kv31 = 1.93241e-3;//water vapor mass absorption coefficient 31.4 GHz

//    float tauo23 = 3.21410E-2 -6.31860E-5*landT;
//    float tauo31 = 5.34214E-2 -1.04835E-4*landT;
//    float kl23 = 1.18201E-1 - 3.48761E-3*landT +5.01301E-5*landT*landT ; //
//    float kl31 = 1.98774E-1 - 5.45692E-3*landT +7.18339E-5*landT*landT ; //

//    float a0 = -0.5*kv23/(kv23*kl31 - kv31*kl23);
//    float a1 = kv31/kv23;
//    float b0 = 0.5*kl23/(kv23*kl31 - kv31*kl23);
//    float b1 = kl31/kl23;


//    float a2 = -2.0*(tauo31 - a1* tauo23)/cosza + (1.0 - a1)*log(landT) + log(1.0 - eps31) - a1*log(1.0 - eps23);
//    float b2 = -2.0*(tauo31 - b1* tauo23)/cosza + (1.0 - b1)*log(landT) + log(1.0 - eps31) - b1*log(1.0 - eps23);

//    float L = a0*(lnt2 - a1*lnt1 - a2);
//    float V = b0*(lnt2 - b1*lnt1 - b2);

    //debug_log<<idx << t23 << t31 << t50 << landT;

    float c0= 247.92 - (69.235 - 44.177* cosza)*cosza;
    float c1 = -116.27;
    float c2=73.409;
    float d0 = 8.240 - (2.622 - 1.846 * cosza)*cosza;
    float d1 = 0.754;
    float d2 = -2.265;

    float TPW = cosza*(c0+c1*lnt1+c2*lnt2);
    float CLW = cosza*(d0+d1*lnt1+d2*lnt2);
    if(CLW < 0.2) TPW = 0.942 *TPW -2.17;
   // debug_log << TPW<<CLW;
    if(qIsNaN(TPW) ||qIsInf(TPW) || (TPW <0) ){
      debug_log << QObject::tr("Неверное значение TPW по индексу %1: %2").arg(idx).arg(TPW);
      TPW = -9999.;
    }
      data_[idx] = TPW;
  }
  return !data_.isEmpty();
}


//bool TotalPrecip::saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format)
//{

//}

