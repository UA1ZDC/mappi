#include "cloudmask.h"

#include "channel.h"
#include "themformat.h"
#include "datastore.h"

#include <mappi/settings/mappisettings.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>

#include <qimage.h>
#include <qfile.h>



namespace {

mappi::to::ThemAlg* createCloudMask(mappi::conf::ThemType type, const std::string &them_name, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new mappi::to::CloudMask(type, them_name, ds);
}

static const bool resmask = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kCloudMask, createCloudMask); //только маска облако /не облако
static const bool restype = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kCloudType, createCloudMask); //тип облаков верх нижние средние
static const bool resalt = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kCloudFull, createCloudMask); //высота облаков
static const bool ressnow = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kSnowBorder, createCloudMask); //граница снега
static const bool resIce = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kIceBorder, createCloudMask); //граница льда

}

using namespace mappi;
using namespace to;


CloudMask::CloudMask(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<DataStore> &ds)
  :ThemAlg(type,them_name,ds)
{
}


CloudMask::~CloudMask()
{
}

//void CloudMask::fillPalette()
//{
//  int alpha = 255;
//  palette_.resize(8);

//  palette_[CLOUD] = QColor(255, 255, 255, alpha).rgba();
//  palette_[SNOW] = QColor(0, 255, 0, alpha).rgba();
//  palette_[ICE] = QColor(0, 0, 255, alpha).rgba();
//  palette_[OPEN_WATER] = QColor(255, 0, 0, alpha).rgba();
//  palette_[OPEN_HYDRO] = QColor(0, 0, 0, alpha).rgba();
//  palette_[OPEN_LAND] = QColor(0, 0, 0, alpha).rgba();
//  palette_[UNKNOW] = QColor(0, 0, 0, alpha).rgba();
//}




//! Обработка данных спутника
bool CloudMask::process()
{
  data_.clear();
  bitmap_.clear();
  const auto chs = channels();
  if(chs.size() == 0) return false;

  for(const auto &config_val : qAsConst(config_.vars())){
    if(!chs.contains(config_val.name())){
      error_log << QObject::tr("В потоке не найден канал %1.").arg(QString::fromStdString(config_val.name())) << chs.keys() ;
      return false;
    }
  }

  QSharedPointer<Channel> A1 =  _ch.value("A1");
  QSharedPointer<Channel> A2 =  _ch.value("A2");
  QSharedPointer<Channel> T3 =  _ch.value("T3");
  QSharedPointer<Channel> T4 =  _ch.value("T4");
  if(A1.isNull() || A2.isNull() || T3.isNull() || T4.isNull() )
  {
    error_log << QObject::tr("Не произведена привязка данных");
    return false;
  }
  if(!initProjection(A1) || nullptr == projection_)
  {
    error_log << QObject::tr("Проекция не создана");
    return false;
  }
  if(!initLandMask() || nullptr == landmask_)
  {
    error_log << QObject::tr("Не удалось загрузить маску суша/вода");
    return false;
  }


  int sz = A1->size();
  data_.resize( sz );

  for (int idx = 0; idx < sz; idx++) {
    pixelType pixtype = CLOUD;
    meteo::GeoPoint geoCoord;
    if(!projection_->X2F_one(idx, &geoCoord) || !geoCoord.isValid()){
      error_log << QObject::tr("Ошибка расчета координат пикселя %1 из %2:").arg(idx).arg(sz) << geoCoord.toString();
      data_[idx] = UNKNOW;
      continue;
    }

    float a1 = A1->at(idx);
    float a2 = A2->at(idx);
    float t3 = T3->at(idx);
    float t4 = T4->at(idx);
    pixtype = cloudTest(a1,a2,t3,t4,-1);

//    CLOUD =0 ,
//    CLOUD_LOWER = 1,
//    CLOUD_MIDL  = 2,
//    CLOUD_HI    = 3,
//    SNOW        = 4,
//    ICE         = 5,
//    OPEN_WATER  = 6,
//    OPEN_HYDRO  = 7,
//    OPEN_LAND   = 8,
//    UNKNOW      = 10


    switch (_themType) {
      case mappi::conf::kCloudMask:
        if(CLOUD == pixtype || CLOUD_LOWER == pixtype || CLOUD_MIDL == pixtype || CLOUD_HI == pixtype){
          pixtype = CLOUD;
        } else {
          pixtype = NOT_CLOUD;
        }
      break;
      case mappi::conf::kCloudType:
      case mappi::conf::kCloudAlt:
        if(CLOUD_LOWER != pixtype && CLOUD_MIDL != pixtype && CLOUD_HI != pixtype){
          pixtype = UNKNOW;
        }
      break;
      case mappi::conf::kSnowBorder:
        if(SNOW != pixtype) {
          pixtype = UNKNOW;
        }
      break;
      case mappi::conf::kIceBorder:
        if(ICE != pixtype) {
          pixtype = UNKNOW;
        }
      break;
      default: break;
    }

    data_[idx] = pixtype;

  }
  return !data_.isEmpty();
}

ThemAlg::pixelType CloudMask::cloudTest(float a1, float a2, float t3,float t4, int landmask)
{
  pixelType pixtype = cloudTestWithLandMask(a1,a2,t3,t4,landmask);
  if(CLOUD != pixtype ) return pixtype;

  if (t4 > them_thresholds_.cloudlower()) return CLOUD_LOWER;
  if (t4 < them_thresholds_.cloudhi()) {
    return CLOUD_LOWER;
  } else {
    return CLOUD_MIDL;
  }
}




