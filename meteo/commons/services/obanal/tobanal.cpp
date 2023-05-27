#include "tobanal.h"
#include "tobanaldb.h"

#include <qsettings.h>

#include <cross-commons/debug/tlog.h>

#include <commons/landmask/landmask.h>
#include <commons/obanal/func_obanal.h>
#include <commons/textproto/pbtools.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/grib/iface/tgribiface.h>
#include <meteo/commons/grib/parser/tgridgrib.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/proto/tgrid.pb.h>
#include <meteo/commons/global/global.h>

constexpr int kMinKolStation = 10;

TObanal::TObanal()
  : db_(new TObanalDb())
  , fa_(nullptr)
{
  QString path = QDir::homePath() + "/obanal";
  db_->setPath(path);
  fa_ = FieldCalcPtr(new obanal::TFieldCalc(db_));
  // grib_factors_map_.insert(10004,0.01);// "Давление";"Па";"";10004
  // grib_factors_map_.insert(10051,0.01);// "Давление, приведенное к среднему уровню моря (СУМ)";"Па";"";10051
  // grib_factors_map_.insert(10052,0.01);// QNH
  // grib_factors_map_.insert(10061,0.01);// "Барическая тенденция";"Па.с-1";"";10061
  grib_factors_map_.insert(10009,0.1);// "Геопотенциальная высота"

  src_factors_map_.insert(10004,0.1);// "Давление";"Па";"";10004
  src_factors_map_.insert(10051,1);// "Давление, приведенное к среднему уровню моря (СУМ)";"Па";"";10051
  src_factors_map_.insert(5510004,0.1);// QNH
  src_factors_map_.insert(7004,0.1);// "Барическая тенденция";"Па.с-1";"";10061
  src_factors_map_.insert(10009,0.1);// "Геопотенциальная высота"
  src_factors_map_.insert(20011,10);// "Количество всех наблюдающихся облаков CL или CM"

  land_sea_mask_need_.append(22043);// Температура воды
  land_sea_mask_need_.append(22021);// Высота волн,м
  land_sea_mask_need_.append(22031);// Скорость течения на характерных горизонтах в м/с
  land_sea_mask_need_.append(22042);// Температура воды на характерных горизонтах с точностью до сотых долей градуса Цельсия
  land_sea_mask_need_.append(22011);// Период волн,с
  land_sea_mask_need_.append(22062);// Соленость воды на характерных горизонтах с точностью до сотых долей промилле
  land_sea_mask_need_.append(20034);// Соленость воды на характерных горизонтах с точностью до сотых долей промилле

  predel_grad_map_.insert(12101,5);//"Температура";"К";"";12101
  predel_grad_map_.insert(12103,5);//"Температура точки росы";"К";"";
  predel_grad_map_.insert(12111,5);
  predel_grad_map_.insert(12112,5);

  predel_grad_map_.insert(22043,2);

  predel_grad_map_.insert(12108,5);//"Понижение (или дефицит) точки росы";"К";"";12108

  predel_grad_map_.insert(5510004,5);
  predel_grad_map_.insert(10004,5);
  predel_grad_map_.insert(7004,5);
  predel_grad_map_.insert(10051,5);
  predel_grad_map_.insert(10052,5);
  predel_grad_map_.insert(10061,5);
  predel_grad_map_.insert(10009,4);
  loadSettings();
}


void TObanal::loadSettings()
{
  QSettings s(MnCommon::etcPath() + "obanal.ini", QSettings::IniFormat);

  s.beginGroup("grib_factors_map");
  QStringList keys = s.childKeys();
  for(int i = 0; i < keys.size();++i){
    const QString& k = keys.at(i);
    grib_factors_map_.insert(k,s.value(k).toFloat());
  }
  s.endGroup();
  s.beginGroup("src_factors_map");
  keys = s.childKeys();
  for(int i = 0; i < keys.size();++i){
    const QString& k = keys.at(i);
    src_factors_map_.insert(k,s.value(k).toFloat());
  }
  s.endGroup();
  s.beginGroup("predel_grad_map");
  keys = s.childKeys();
  for(int i = 0; i < keys.size();++i) {
      const QString& k = keys.at(i);
      predel_grad_map_.insert(k,s.value(k).toFloat());
    }
  s.endGroup();
  s.beginGroup("deltas_map");
  keys = s.childKeys();
  for(int i = 0; i < keys.size();++i){
      const QString& k = keys.at(i);
      deltas_map_.insert(k,s.value(k).toFloat());
    }
  s.endGroup();
  s.beginGroup("settings");
  land_sea_mask_need_=s.value("land_sea_mask_need").toList();
  rast_ = s.value("predel_radius",5).toFloat();
  kolstan_ = s.value("kol_stan",20).toInt();
  kolstmin_ = s.value("kol_stan_min",3).toInt();
  rastfrprorej_=s.value("rast_from_prorej",0.5).toFloat();
  shir_skl_= MnMath::deg2rad(s.value("shir_skley",12.).toFloat());
  s.endGroup();
}


TObanal::~TObanal() { }

bool TObanal::calculateAndSaveGrib(meteo::GeoData* gdata, const QVector<RegionParam>& corners, meteo::field::DataDesc* fdescr)
{
 /* QString log = QObject::tr("Анализ данных GRIB за %1 величина %2 уровень %3 центр %4 срок  %5 ч.")
  .arg(QString::fromStdString(fdescr->date()))
  .arg(fdescr->meteodescr())
  .arg(fdescr->level())
  .arg(fdescr->center())
  .arg(fdescr->hour() / 3600);*/

  QString log = createInfo(fdescr, gdata->size());

  obanal::TField rezult_;

  // if(10004 == fdescr->meteodescr()) { fdescr->set_meteodescr(10051); }

  rezult_.setNet      (corners, static_cast<NetType>(fdescr->net_type()));
  rezult_.setValueType(fdescr->meteodescr(), fdescr->level(), fdescr->level_type());
  rezult_.setHour     (fdescr->hour());
  rezult_.setModel    (fdescr->model());
  rezult_.setCenter   (fdescr->center());
  rezult_.setDate     (QDateTime::fromString(QString::fromStdString(fdescr->date()), Qt::ISODate));


  if(fdescr->has_center_name())     { rezult_.setCenterName(QString::fromStdString(fdescr->center_name())); }
  if(fdescr->has_level_type_name()) { rezult_.setLevelTypeName(QString::fromStdString(fdescr->level_type_name())); }

  //double mnoz = 1.;
  // switch(fdescr->meteodescr()) {
  //   case 12101: // "Температура";"К";"";12101
  //   case 12103: // "Температура точки росы";"К";"";12103
  //   case 12108: // "Понижение (или дефицит) точки росы";"К";"";12108
  //     if(gdata->data()->data > 100.0f) {
  //       gdata->addDeltaData(-273.15f);
  //     }
  //     break;
  //   default:
  //     gdata->addDeltaData(deltas_map_.value(fdescr->meteodescr(),0));
  //     break;
  // }
  gdata->mnozData(grib_factors_map_.value(fdescr->meteodescr(),1));


  int ret_val =  MnObanal::prepInterpolOrder(gdata, &rezult_);
  if(ERR_OBANAL == ret_val) {
      error_log <<log<<" - ошибка интерполяции";
      return false;
    }

  if(ERR_NODATA_OBANAL == ret_val) {
    info_log <<log<<" - нет данных для анализа";
    return false;
  }

  if(land_sea_mask_need_.contains(fdescr->meteodescr())){
      for(int i = 0; i < rezult_.kolFi(); ++i) {
        for(int j = 0; j < rezult_.kolLa(); ++j) {
          int is_land = meteo::map::LandMask::instance()->get(MnMath::rad2deg(rezult_.netFi(i)), MnMath::rad2deg(rezult_.netLa(j)));
          rezult_.setMasks(rezult_.num(i, j), !(is_land == 1)); // TODO если океан - то другая маска!
        }
      }
    }

  fdescr->set_count_point(gdata->count());
  if(false == db_->saveField(rezult_, *fdescr))
  {
    error_log  << QObject::tr(" - ошибка записи в базу!");
    return false;
  }
  info_log << log << QObject::tr("- успешно");

  //ветер
  if(nullptr == fa_) { return true; }
  if(true == fa_->calcWind(rezult_, fdescr)) {
    info_log << log << QObject::tr("- успешно");
  }
  if(true == fa_->calcOT500_1000(rezult_, fdescr)){
    info_log << log << QObject::tr("- успешно");
   }
  if ( 10008 == fdescr->meteodescr() ) {
    if ( true == fa_->calcGeopotentialHeight( rezult_, fdescr ) ) {
      info_log << log << QObject::tr("- успешно");
    }
    else {
      debug_log << QObject::tr("Не удалось посчитать поле геопотенциальной высоты");
    }
  }
  return true;
}



QString TObanal::createInfo( meteo::field::DataDesc* fdescr, int sz){
  int ntypel = fdescr->level_type();
  QString ltype = QObject::tr("Неизвестный уровень: ")+ QString::number(ntypel);
  QMap< int, QString > ltypes = meteo::global::kLevelTypes();
  if(ltypes.contains(ntypel)) {
      ltype = ltypes[ntypel];
  }
  QString log = QObject::tr("Анализ %1 за %2 %3")
      .arg(TMeteoDescriptor::instance()->name(fdescr->meteodescr()))
      .arg(QString::fromStdString(fdescr->date()))
      .arg(ltype);

  switch (ntypel) {
    case 100:
      log += QObject::tr(" %1 мбар").arg(fdescr->level());
      break;
    case 20:
    case 160:
    case 102:
    case 103:
    case 106:
    case 15000:
      log += QObject::tr(" %1 м").arg(fdescr->level());
      break;
    }
  QString center =  meteo::global::kMeteoCenters()[fdescr->center()].first;
  log += QObject::tr(" (%1) данных %2")
      .arg(center)
      .arg(sz);
  if(0 != fdescr->hour()){
      log += QObject::tr(" %1 ч.").arg(fdescr->hour()/3600);
    }

  return log;
}

bool TObanal::calculateAndSaveField(meteo::GeoData* all_data,  meteo::field::DataDesc* fdescr)
{
  QString log = createInfo(fdescr, all_data->size());

  if(all_data->count() < kMinKolStation) {
    info_log << log << QObject::tr("- недостаточно данных для анализа");
    return false;
  }
  obanal::TField rezult;
  QDateTime dt = meteo::DbiQuery::datetimeFromString(fdescr->date());
  rezult.setNet      (rezult.getRegPar(), static_cast<NetType>(fdescr->net_type()));
  rezult.setNetMask  (rezult.getRegPar());
  rezult.setValueType(fdescr->meteodescr(),  fdescr->level(), fdescr->level_type());
  rezult.setHour     (fdescr->hour());
  rezult.setModel    (fdescr->model());
  rezult.setCenter   (fdescr->center());
  rezult.setDate     (dt);
  if(fdescr->has_center_name())     { rezult.setCenterName(QString::fromStdString(fdescr->center_name())); }
  if(fdescr->has_level_type_name()) { rezult.setLevelTypeName(QString::fromStdString(fdescr->level_type_name())); }

  float predel_grad = -1.;
  predel_grad = predel_grad_map_.value(fdescr->meteodescr(),-1);


  switch(fdescr->meteodescr()) {
    case 5510004: // "Давление";
    case 10004: // "Давление";
    case 7004:  // "Давление";
    case 10051: // "Давление, приведенное к среднему уровню моря (СУМ)";"Па";"";10051
      fdescr->set_meteodescr(10051);
    case 10009: // "Геопотенциальная высота"
      predel_grad = 4.0; // PREDEL_GRAD_H
      break;
//    case 20011: //20011"    ="Количество всех наблюдающихся облаков CL или CM" />
//    break;
    case 11001: // "Компонент dd: ad_= 11001; break;
    case 11002: // "Компонент ff: ad_= 11002; break;
      return false;
    default:
        break;
  }
  //--begin '2015-11-27 12:00:00' -d 610009 -t 60 -p 1000
  all_data->mnozData(src_factors_map_.value(fdescr->meteodescr(),1));

  QStringList badData;
  int res = MnObanal::prepInterpolHaos(all_data, &rezult, predel_grad, &badData,
                                       rast_,kolstan_,kolstmin_,rastfrprorej_,shir_skl_);

  if(ERR_OBANAL == res) {
    error_log << log << " - ошибка интерполяции";
    return false;
  }
  if(ERR_NODATA_OBANAL == res) {
    info_log << log << " - нет данных для анализа";
    return false;
  }
  db_->updateQuality(dt,badData, TMeteoDescriptor::instance()->name(fdescr->meteodescr()), control::DOUBTFUL);
  if(land_sea_mask_need_.contains(fdescr->meteodescr())){
      for(int i = 0; i < rezult.kolFi(); ++i) {
        for(int j = 0; j < rezult.kolLa(); ++j) {
          int is_land = meteo::map::LandMask::instance()->get(MnMath::rad2deg(rezult.netFi(i)), MnMath::rad2deg(rezult.netLa(j)));
          rezult.setMasks(rezult.num(i, j), !(is_land == 1));//TODO если океан - то другая маска!
        }
      }
  }

  fdescr->set_count_point(rezult.getSrcPointCount());
  if(false == db_->saveField(rezult, *fdescr))
  {
    error_log  << log << QObject::tr("- ошибка записи в базу!") ;
    return false;
  }
  info_log << log << QObject::tr("- успешно");
  //ветер
  if(nullptr == fa_) { return true; }

  if(false == fa_->calcWind(rezult, fdescr)) {
  }

  fa_->calcOT500_1000(rezult, fdescr);

  return true;
}
