#include "maker.h"


#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <commons/obanal/tfield.h>
#include <sql/nosql/nosqlquery.h>
#include <cross-commons/app/waitloop.h>
#include <meteo/commons/services/obanal/gribobanalmaker.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qfile.h>

namespace meteo {
namespace obanal {

Maker::Maker( commons::ArgParser* options)
  : service_(new meteo::TObanalService()),
    omaker_(new meteo::obanal::ObanalMaker()),
    options_(options)
{
  fillRequestsFromArg( requests_, gribRequests_);
}

Maker::~Maker()
{
  delete service_ ;
  service_ = nullptr;
  delete omaker_ ;
  omaker_ = nullptr;
}

void Maker::autoObanal()
{
  info_log << "Анализ Obanal";
  if ( nullptr == options_ ) {
      error_log << QObject::tr("Недостаточно аргументов для анализа.");
      return;
    }
  if (options_->installed(kFile)) {
      info_log << QObject::tr("Был указан параметр ") << kFile.longname();
      if (options_->installed(kSurfaceArg)) {
          error_log << QObject::tr("Был указан параметр ") << kSurfaceArg.longname()
                    << QObject::tr(" он не будет обработан");
        }
      if (options_->installed(kAeroArg)) {
          error_log << QObject::tr("Был указан параметр ") << kAeroArg.longname()
                    << QObject::tr(" он не будет обработан");
        }
      if (options_->installed(kOceanArg)) {
          error_log << QObject::tr("Был указан параметр ") << kOceanArg.longname()
                    << QObject::tr(" он не будет обработан");
        }
      if (options_->installed(kGribAnalyze)) {
          error_log << QObject::tr("Был указан параметр ") << kGribAnalyze.longname()
                    << QObject::tr(" он не будет обработан");
        }
      customObanal();
      customGribObanal();
      return;
    }
  if ( options_->installed(kSurfaceArg)
       || options_->installed(kAeroArg)
       || options_->installed(kOceanArg)) {
      obanal();
    }
  if ( options_->installed(kGribAnalyze) ) {
      gribObanal();
    }
  return;
}

void Maker::obanal()
{
  if (options_->installed(kDescr)
      || options_->installed(kStation)
      || options_->installed(kLevelP)
      || options_->installed(kLevelH)
      || options_->installed(kLevelType)) {
    customObanal();
  }
  else {
    if (options_->installed(kSurfaceArg)) {
      makeAutoObanalSurf( requests_.first() );
    }
    if (options_->installed(kAeroArg)) {
      if (requests_.first().has_date_start() || requests_.first().has_date_end()) {
        makeCustomDateAutoObanalAero(requests_.first());
      }
      else {
        makeAutoObanalAero( requests_.first() );
      }
    }
    if (options_->installed(kOceanArg)) {
      makeAutoObanalOcean( requests_.first() );
    }
  }
}

bool Maker::customObanal()
{
  if ( 0 == requests_.size() ) {
      QObject::tr("Ошибка. Не составлено ни одного запроса для анализа");
      return false;
    }
  if ( nullptr == options_) {
      QObject::tr("Ошибка. Нет опций запуска");
      return false;
    }
  for (auto req: requests_) {
      if (true == options_->installed(kAeroArg)) {
          customObanalAero(req);
        }
      if (true == options_->installed(kOceanArg)) {
          customObanalOcean(req);
        }
      if (true == options_->installed(kSurfaceArg)) {
          customObanalSurf(req);
        }
    }
  return true;
}



void Maker::gribObanal()
{
  if (options_->installed(kDescr)
      || options_->installed(kLevelP)
      || options_->installed(kLevelType)
      || options_->installed(kCenter)
      || options_->installed(kHour)
      || options_->installed(kModel)) {
      customGribObanal();
    }
  else {
      makeAutoGribObanal();
    }
}

void Maker::slotMakeAutoObanal(){
  autoObanal();
  emit finished(true);
}

bool Maker::makeAutoObanalSurf(const meteo::surf::DataRequest& rdt){
  surf::DataRequest request;
  request.CopyFrom(rdt);
  request.add_meteo_descr(10051);
  request.add_meteo_descr(12101);
  request.add_meteo_descr(10061);
  request.add_meteo_descr(12111);//Максимальная температура
  request.add_meteo_descr(12112);//Минимальная температура
  request.add_meteo_descr(13023);//Количество осадков в сутки
  request.add_meteo_descr(12103);//Температура точки росы
  request.add_meteo_descr(20010);//20011"    ="Количество всех наблюдающихся облаков CL или CM" />
//  request.add_meteo_descr(20011);//20011"    ="Количество всех наблюдающихся облаков CL или CM" />
//  request.add_meteo_descr(20012);// 20012" "Род облаков" />
//  request.add_meteo_descr(20013);//20013" "Высота основания самых низких видимых облаков над поверхностью земли,м" />
  request.add_meteo_descr(11001);//Компонент ветра u
  //  request.add_meteo_descr(10004);
  //  request.add_meteo_descr(20034 );//Сполченность льда
  //  request.add_meteo_descr(11002 );//Компонент ветра v
  //  request.add_meteo_descr(22043);//Температура поверхности моря,C
  //приземные
  return customObanalSurf(request);
}


bool Maker::customObanalSurf( const surf::DataRequest &rdt )
{
  surf::DataRequest request;
  request.CopyFrom(rdt);
  QDateTime dts ,dte;
  getDt(request,&dts, &dte,  hoursToSurf);
  bool ret_val = false;
  request.clear_date_end();
  request.set_type_level(1);
  request.set_level_p(0);
  request.set_max_srok(0);
  request.clear_type();


  request.add_type(meteo::surf::kSynopFix);
  request.add_type(meteo::surf::kSynopCoast);
  //request.add_type(meteo::surf::kSynopWarep);
 // request.add_type(meteo::surf::kSynopSnow);
  //request.add_type(meteo::surf::kSynopBufr);
  request.add_type(meteo::surf::kSynopMob);
  request.add_type(meteo::surf::kSynopMobSea);
  request.add_type(meteo::surf::kSynopMobShip);

  request.add_type(meteo::surf::kHydroStation);

/*  for (int i = meteo::surf::kSynopType; i < meteo::surf::kEndSynopType;++i){
      request.add_type(i);
    }*/
  for (int i = meteo::surf::kAirportType; i < meteo::surf::kAirportSpecType;++i){
      request.add_type(i);
    }
  for (int i = meteo::surf::kAeroType; i < meteo::surf::kEndAeroType;++i){
      request.add_type(i);
    }
  for (int i = meteo::surf::kOceanType; i < meteo::surf::kOceanDepth;++i){
      request.add_type(i);
    }



  QDateTime obanaldt = dte;
  for ( int i =0; obanaldt >= dts; i+=stepHoursToSurf ) {
      obanaldt = dte.addSecs( -i*3600 );
      request.set_date_start( obanaldt.toString(Qt::ISODate).toStdString() );
      obanalPoDescrs(request);
    }

  return   ret_val;
}

bool Maker::makeAutoObanalAero(const meteo::surf::DataRequest& rdt )
{
  QDateTime dts ,dte;
  getDt(rdt,&dts, &dte,  hoursToAero);
  dts = QDateTime::currentDateTimeUtc();
  int hour = (dts.time().hour() / 12) * 12;
  dts.setTime(QTime(hour, 0,0));
//  dte = QDateTime::currentDateTimeUtc();
//  if(dte.time().hour() < 12){
//      dts.setTime(QTime(12,0,0));
//      dte.setTime(QTime(0,0,0));
//    } else {
//      dts.setTime(QTime(0,0,0));
//      dte.setTime(QTime(12,0,0));
//    }
//  QDateTime obanaldt = dte;
//  for( int i =0; obanaldt >= dts; i+=stepHoursToAero ) {
//      obanaldt = dte.addSecs( -i*3600 );
      meteo::surf::DataRequest request;
      request.clear_type();
      request.set_date_start( dts.toString(Qt::ISODate).toStdString() );
      QList<int> pLevels;
      //на стандартных изобарических поверхностях
      pLevels<<1000<<925<<850<<700<<500<<400 <<300<<250<<200<<150<<100<<70<<50<<30<<20<<10<<5;

      request.clear_meteo_descr();
      request.add_meteo_descr(10009); //Геопотенциальная высота
      request.add_meteo_descr(12101); //Температура
      request.add_meteo_descr(12108); //Дефицит точки росы на уровнях
      request.add_meteo_descr(12103); //Дефицит точки росы на уровнях
      request.add_meteo_descr(11001);//Компонент ветра dd

      request.set_type_level(100);

      for ( int j = 0, levsz = pLevels.size(); j < levsz; ++j ) {
          int level = pLevels[j];
          request.set_level_p(level);
          obanalPoDescrs(request);
        }

      //тропопауза
     // request.add_meteo_descr(10009); //Геопотенциальная высота
      request.add_meteo_descr(5510004); //
      request.add_meteo_descr(12101); //Температура
      request.add_meteo_descr(12108); //Дефицит точки росы на уровнях
      request.add_meteo_descr(12103); //Дефицит точки росы на уровнях
      request.add_meteo_descr(11001);//Компонент ветра dd

      request.set_type_level(7);
      request.clear_level_h();
      request.clear_level_p();

      obanalPoDescrs(request);

      request.clear_meteo_descr();
     // request.add_meteo_descr(10009); //Геопотенциальная высота
      request.add_meteo_descr(5510004); //
      request.add_meteo_descr(11001);//Компонент ветра dd
      request.add_meteo_descr(11061);//Абсолютная величина векторной разности ветра между максимальным ветром и ветром на 1 км ниже него, м/с"
      request.add_meteo_descr(11062);//Абсолютная величина векторной разности ветра между максимальным ветром и ветром на 1 км выше него, м/с" />
      //request.add_meteo_descr(12101); //Температура
      //request.add_meteo_descr(12108); //Дефицит точки росы на уровнях
      //request.add_meteo_descr(12103); //Дефицит точки росы на уровнях
      request.set_type_level(6);
      obanalPoDescrs(request);

//    }
  return true;
}

bool Maker::makeCustomDateAutoObanalAero(const surf::DataRequest &rdt)
{
  QDateTime dts ,dte;
  getDt(rdt,&dts, &dte,  hoursToAero);
  QDateTime obanaldt = dts;
  for( int i =0; obanaldt <= dte; ++i, obanaldt = dts.addSecs( i*3600 ) ) {
    meteo::surf::DataRequest request;
    request.clear_type();
    request.set_date_start( obanaldt.toString(Qt::ISODate).toStdString() );
    QList<int> pLevels;
    //на стандартных изобарических поверхностях
    pLevels<<1000<<925<<850<<700<<500<<400 <<300<<250<<200<<150<<100<<70<<50<<30<<20<<10<<5;

    request.clear_meteo_descr();
    request.add_meteo_descr(10009); //Геопотенциальная высота
    request.add_meteo_descr(12101); //Температура
    request.add_meteo_descr(12108); //Дефицит точки росы на уровнях
    request.add_meteo_descr(12103); //Дефицит точки росы на уровнях
    request.add_meteo_descr(11001);//Компонент ветра dd

    request.set_type_level(100);

    for ( int j = 0, levsz = pLevels.size(); j < levsz; ++j ) {
      int level = pLevels[j];
      request.set_level_p(level);
      obanalPoDescrs(request);
    }

    //тропопауза
    request.add_meteo_descr(10009); //Геопотенциальная высота
    request.add_meteo_descr(5510004); //
    request.add_meteo_descr(12101); //Температура
    request.add_meteo_descr(12108); //Дефицит точки росы на уровнях
    request.add_meteo_descr(12103); //Дефицит точки росы на уровнях
    request.add_meteo_descr(11001);//Компонент ветра dd

    request.set_type_level(7);
    request.clear_level_h();
    request.clear_level_p();
    obanalPoDescrs(request);

    request.clear_meteo_descr();
    request.add_meteo_descr(10009); //Геопотенциальная высота
    request.add_meteo_descr(5510004); //
    request.add_meteo_descr(11001);//Компонент ветра dd
    request.add_meteo_descr(11061);//Абсолютная величина векторной разности ветра между максимальным ветром и ветром на 1 км ниже него, м/с"
    request.add_meteo_descr(11062);//Абсолютная величина векторной разности ветра между максимальным ветром и ветром на 1 км выше него, м/с" />
    //request.add_meteo_descr(12101); //Температура
    //request.add_meteo_descr(12108); //Дефицит точки росы на уровнях
    //request.add_meteo_descr(12103); //Дефицит точки росы на уровнях
    request.set_type_level(6);
    obanalPoDescrs(request);
  }
  return true;
}

void Maker::customObanalAero( const surf::DataRequest& rdt )
{
  if(false == checkParams(  rdt)){return;}
  surf::DataRequest request;
  request.CopyFrom(rdt);

  QDateTime dts, dte;
  getDt(rdt,&dts,&dte,hoursToAero);
  QDateTime obanaldt = dte;
  request.clear_date_end();
  request.clear_type();

  for( int i =0; obanaldt >= dts; ++i ) {
      obanaldt = dte.addSecs( -i*3600 );
      request.set_date_start( obanaldt.toString(Qt::ISODate).toStdString() );
      obanalPoDescrs(request);
    }
}


bool Maker::makeAutoObanalOcean( const meteo::surf::DataRequest& rdt ){
  QDateTime dts ,dte;
  getDt(rdt,&dts, &dte,  hoursToOcean);
  meteo::surf::DataRequest request;
  request.CopyFrom(rdt);
  QDateTime obanaldt = dte;
  request.clear_type();
  for( int i = 0; obanaldt >= dts; i+=stepHoursToOcean ) {
      obanaldt = dte.addSecs( -i*3600 );
      request.set_date_start( obanaldt.toString(Qt::ISODate).toStdString() );

      request.clear_meteo_descr();
      request.add_meteo_descr(22043); //Температура поверхности моря,C
      request.add_meteo_descr(22021); //Высота волн,м
      request.add_meteo_descr(22011); //Период волн,с

      request.set_type_level(1);
      request.set_level_h(0);
      obanalPoDescrs(request);

      request.set_type_level(160);
      QList<int> hLevels;
      hLevels<< 2000<<1500<<1200<<1000<<750<<500<<400<<300<<200<<100<<50<<25<<10<<5<<1;

      request.clear_meteo_descr();
      request.add_meteo_descr(22031); //Скорость течения на характерных горизонтах в м/с
      request.add_meteo_descr(22042); //Температура воды на характерных горизонтах с точностью до сотых долей градуса Цельсия
      request.add_meteo_descr(22062); //Соленость воды на характерных горизонтах с точностью до сотых долей промилле
      for ( int h = 0, descsh = hLevels.size(); h < descsh; ++h ) {
          request.set_level_p(hLevels.at(h));
          obanalPoDescrs(request);
        }
    }
  return true;
}

void Maker::customObanalOcean(const surf::DataRequest &rdt)
{
  if(false == checkParams(  rdt)) { return ;}

  meteo::surf::DataRequest request;
  request.CopyFrom(rdt);

  QDateTime dts, dte;
  getDt(rdt,&dts,&dte,hoursToOcean);

  QDateTime obanaldt = dte;
  request.clear_date_end();
  request.clear_type();
  //request.add_type(kOcean);

  for( int i = 0; obanaldt >= dts; i+=stepHoursToOcean ) {
      obanaldt = dte.addSecs( -i*3600 );
      request.set_date_start( obanaldt.toString(Qt::ISODate).toStdString() );
      obanalPoDescrs(request);
    }
}


bool Maker::customGribObanal()
{

  if ( 0 == gribRequests_.size() ) {
      error_log << QObject::tr("Анализ данных не выполнен. Список запросов пуст. Необходимо больше параметров");
      return false;
    }
  GribObanalMaker gribMaker;
  bool finished = false;
  int errorCnt = 0;
  for (int i = 0, sz = gribRequests_.size(); i < sz; ++i) {
      while (!finished) {
          QTime timer;
          timer.start();
          int res = gribMaker.makeCustomObanal( service_, gribRequests_[i]);
          switch (res) {
            case AUTOOBANAL_CONNECTION_REFUSED: {
                error_log << QObject::tr("Невозможно установить связь с сервисом.");
                finished = true;
                break;
              }
            case AUTOOBANAL_FINISHED:{
                finished = true;
                break;
              }
            case AUTOOBANAL_NOREPLY:{
                finished = true;
                break;
              }
            case AUTOOBANAL_ERROR: {
                static const int maxErrors = 10000;
                ++errorCnt;
                error_log << QObject::tr("Произошла ошибка при выполнении autoObanal. Количество ошибок: %1")
                             .arg(errorCnt);
                if (maxErrors <= errorCnt) {
                    finished = true;
                  }
                break;
              }
            case AUTOOBANAL_NOINFO:
            case AUTOOBANAL_NODATA: {
                break;
              }
            case 0: {
                break;
              }
            default: {
                break;
              }
            }
        }
    }
  return true;
}

bool Maker::makeGribObanal( const meteo::surf::GribDataRequest& request )
{

  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if(nullptr == ctrl) {
      error_log << meteo::msglog::kServiceConnectFailedSimple
                   .arg(meteo::global::serviceAddress( meteo::settings::proto::kSrcData ));
      return false;
    }
  meteo::surf::GribDataReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetGribData, request, 300000 );
  if ( nullptr == reply ) {
      error_log << QObject::tr("При попытке выполнить анализ  данных в коде GRIB ответ от сервиса данных не получен");
      return false;
    }
  bool ok = false;
  if( 0 < reply->data_size() ) {
      ok = service_->haveValueRespGrib(reply);
    }
  else {
      error_log << QObject::tr("Доступных данных в коде GRIB за указанный срок нет");
      ok = false;
    }
  meteo::surf::SimpleDataRequest req;
  *req.mutable_id() = reply->ids();
  delete reply; reply = nullptr;
  if (req.id_size() > 0) {
      QTime ttt_remove; ttt_remove.start();
      meteo::surf::SimpleDataReply* sreply = ctrl->remoteCall( &meteo::surf::SurfaceService::RemoveGribDataAnalysed, req , 10000);
      if ( nullptr == sreply ) {
          error_log << QObject::tr("При попытке установить флаг анализа данных в коде GRIB ответ от сервиса данных не получен");
          ok = false;
        } else {
          ok = sreply->result();
        }
      delete sreply;
      sreply = nullptr;
    }
  delete ctrl;
  ctrl = nullptr;
  delete reply;
  reply = nullptr;
  return ok;
}

bool Maker::makeAutoGribObanal()
{
  info_log << QObject::tr("Анализ GRIB...");
  GribObanalMaker gribMaker;
  bool finished = false;
  int errorCnt = 0;
  static const int maxErrors = 100;
  auto onError = [&errorCnt](){
    ++errorCnt;
    WaitLoop wl;
    wl.wait(5000);
  };
  while ( (false == finished) && ( errorCnt <= maxErrors) ) {
      int res = gribMaker.makeAutoGribObanal(service_);
      switch (res) {
      case AUTOOBANAL_CONNECTION_REFUSED: {
        onError();
        error_log << QObject::tr("Невозможно установить связь с сервисом.");
        break;
      }
      case AUTOOBANAL_FINISHED:{
        finished = true;
        break;
      }
      case AUTOOBANAL_NOREPLY:{
        finished = true;
        break;
      }
      case AUTOOBANAL_ERROR: {
        onError();
        error_log << QObject::tr("Произошла ошибка при выполнении autoObanal. Количество ошибок: %1")
                     .arg(errorCnt);
        break;
      }
      case AUTOOBANAL_NOINFO:
      case AUTOOBANAL_NODATA: {
        break;
      }
      case 0: {
        info_log << "Status: ok";
        break;
      }
      default: {
        break;
      }
      }
  }
  info_log  << "Анализ GRIB завершен";
  return EXIT_SUCCESS;
}

void Maker::fillRequestsFromArg( QList<meteo::surf::DataRequest>& requests, QList<meteo::surf::GribDataRequest>& gribRequests)
{
  if ( nullptr == options_ ) {
      error_log << QObject::tr("Ошибка. Не заданы параметры запуска.");
      return;
    }
  meteo::surf::GribDataRequest grib_request;
  meteo::surf::DataRequest request;
  std::string beginDate;
  std::string endDate;
  if ( true == options_->hasValues(kBeginDate) ) {
      beginDate = options_->at(kBeginDate).value().toStdString();
      grib_request.set_date_start(beginDate);
      grib_request.mutable_info()->set_date(beginDate);
      request.set_date_start(beginDate);
    }
  if ( true == options_->hasValues(kFinishDate) ) {
      endDate = options_->at(kFinishDate).value().toStdString();
      grib_request.set_date_end(endDate);
      request.set_date_end(endDate);
    }
  if ( true == options_->hasValues(kFile) ) {
      run_fromFile(options_, &grib_request, &requests, &gribRequests,beginDate,endDate);
    }
  else {
      if ( true == options_->hasValues(kDescr) ) {
          QStringList descrlist = options_->at(kDescr).values();
          for ( int i = 0, sz = descrlist.size(); i < sz; ++i ) {
              request.add_meteo_descr(descrlist[i].toInt());
            }
            
          for(int i =0 ;i < descrlist.size();++i){
              grib_request.mutable_info()->add_param(descrlist[i].toInt());
            }
        }
      if ( true == options_->hasValues(kStation) ) {
          QStringList statlist = options_->at(kStation).values();
          for ( int i = 0, sz = statlist.size(); i < sz; ++i ) {
              request.add_station(statlist[i].toStdString());
            }
        }
      if ( true == options_->hasValues(kLevelP) ) {
          request.set_level_p( options_->at(kLevelP).value().toInt() );
          grib_request.mutable_info()->set_level(request.level_p());
        }
      if ( true == options_->hasValues(kLevelH) ) {
          request.set_level_h( options_->at(kLevelH).value().toInt() );
        }
      if ( true == options_->hasValues(kLevelType) ) {
          request.set_type_level( options_->at(kLevelType).value().toInt() );
          grib_request.mutable_info()->set_level_type(options_->at(kLevelType).value().toInt());
        }
      requests.append(request);
      if ( true == options_->installed(kGribAnalyze) ) {
          if ( true == options_->hasValues(kCenter) ) {
              grib_request.mutable_info()->set_center(options_->at(kCenter).value().toInt());
            }
          if ( true == options_->hasValues(kHour) ) {
              grib_request.mutable_info()->mutable_hour()->set_hour(options_->at(kHour).value().toInt());
            }
          if ( true == options_->hasValues(kModel) ) {
              grib_request.mutable_info()->set_model(options_->at(kModel).value().toInt());
            }
          gribRequests.append(grib_request);
        }
    }
}

bool Maker::taskGribHandler(const meteo::obanalsettings::Task& task, meteo::surf::GribDataRequest* grib_request,
                                  QList<meteo::surf::GribDataRequest> *gribRequests)
{
  QDateTime startDateTime = task.params().has_dt_begin() ? QDateTime::fromString(QString::fromStdString(task.params().dt_begin()), Qt::ISODate)
                                                         : QDateTime();
  QDateTime endDateTime = task.params().has_dt_end() ? QDateTime::fromString(QString::fromStdString(task.params().dt_end()), Qt::ISODate)
                                                     : QDateTime();
  if (startDateTime.isValid()) {
      grib_request->set_date_start(startDateTime.toString(Qt::ISODate).toStdString());
    }
  if (endDateTime.isValid()) {
      grib_request->set_date_end(endDateTime.toString(Qt::ISODate).toStdString());
    }
  if (!grib_request->has_date_start()) {
      QDateTime dt = QDateTime::currentDateTimeUtc();
      dt.setTime( QTime( dt.time().hour(), 0 ) );
      grib_request->set_date_start( dt.toString(Qt::ISODate).toStdString() );
    }

  bool hasCenters = task.params().center_size() > 0;
  QList<int> allCenters;
  if (!hasCenters) {
      meteo::rpc::Channel* ctrl =  meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
      if ( nullptr == ctrl ) {
          return false;
        }
      meteo::sprinf::MeteoCenterRequest req;
      meteo::sprinf::MeteoCenters* res = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetMeteoCenters, req, 5000);
      delete ctrl;
      if (res != nullptr) {
          for (int j = 0, jsz = res->center_size(); j < jsz; ++j) {
              allCenters.append(res->center(j).id());
            }
        }
      delete res;
    }

  bool hasDescrs = task.params().descr_size() > 0;
  QList<int> allDescrs;
  if (!hasDescrs) {
      meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
      if ( nullptr == ctrl ) {
          return false;
        }
      meteo::sprinf::MeteoParameterRequest req;
      meteo::sprinf::MeteoParameters* res = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetMeteoParametersByBufr, req, 5000);
      delete ctrl;
      if (res != nullptr) {
          for (int j = 0, jsz = res->parameter_size(); j < jsz; ++j) {
              allDescrs.append(res->parameter(j).bufr());
            }
        }
      delete res;
    }

  if (task.params().has_hour()) {
      grib_request->mutable_info()->mutable_hour()->set_hour(task.params().hour());
    }

  if (task.params().level_size() > 0) {
      grib_request->mutable_info()->set_level_type(100);
    }

  for (int ci = 0, csz = (hasCenters ? task.params().center_size()
                          : allCenters.size());
       ci < csz; ++ci) {
      grib_request->mutable_info()->set_center((hasCenters ? task.params().center(ci)
                                                           : allCenters.at(ci)));
      for (int di = 0, dsz = (hasDescrs ? task.params().descr_size()
                              : allDescrs.size());
           di < dsz; ++di) {
          grib_request->mutable_info()->add_param((hasDescrs ? task.params().descr(di)
                                                             : allDescrs.at(di)));
          for (int li = 0, lsz = task.params().level_size();
               li < lsz; ++li) {
              grib_request->mutable_info()->set_level(task.params().level(li));
              gribRequests->append(*grib_request);
            }
        }
    }
  grib_request->Clear();
  return true;
}

void Maker::fillDtsRequests(QList<meteo::surf::DataRequest>* requests, meteo::surf::DataRequest& request,
                                  QDateTime& startDateTime, QDateTime& endDateTime )
{
  int stepNum = 0;
  while (startDateTime.addSecs(stepHoursToSurf*stepNum*3600) <= endDateTime) {
      request.set_date_start(startDateTime.addSecs(stepHoursToSurf*stepNum*3600).toString(Qt::ISODate).toStdString());
      requests->append(request);
      ++stepNum;
    }
}

void Maker::fillLevelsRequests(QList<meteo::surf::DataRequest> *requests,QList<int>& levels,
                                     QDateTime& startDateTime, QDateTime& endDateTime,
                                     int datatype, meteo::surf::DataRequest request)
{
  for (int levelNum = 0; levelNum < levels.count(); ++levelNum) {
      if ( datatype == OBANAL_TYPE::kAero) {
          request.set_level_p(levels.at(levelNum));
        }
      else if (request.type_level() == 100 || request.type_level() == 1) {
          request.set_level_h(levels.at(levelNum));
        }
      else if (request.type_level() == 105) {
          request.set_level_p(levels.at(levelNum));
        }
      if (!endDateTime.isValid()) {
          requests->append(request);
        }
      else {
          fillDtsRequests(requests,request,startDateTime,endDateTime);
        }
    }
}

void Maker::fillDescrRequests(QList<meteo::surf::DataRequest> *requests,QList<int>& levels,
                                    QDateTime& startDateTime, QDateTime& endDateTime,
                                    int datatype, QList<int> descrs, meteo::surf::DataRequest& request)
{
  for (int descrNum = 0; descrNum < descrs.count(); ++descrNum) {
      request.clear_meteo_descr();
      request.add_meteo_descr(descrs.at(descrNum));
      if (levels.isEmpty()) {
          if (!endDateTime.isValid()) {
              requests->append(request);
            }
          else {
              fillDtsRequests(requests,request,startDateTime,endDateTime);
            }
        }
      else {
          fillLevelsRequests(requests,levels,startDateTime,endDateTime,datatype, request);
        }
    }
}

bool Maker::taskHandler(const meteo::obanalsettings::Task& task, QList<meteo::surf::DataRequest> *requests,
                              const std::string &beginDate, const std::string &endDate)
{
  QDateTime startDateTime = task.params().has_dt_begin() ? QDateTime::fromString(QString::fromStdString(task.params().dt_begin()), Qt::ISODate)
                                                         : QDateTime();
  QDateTime endDateTime = task.params().has_dt_end() ? QDateTime::fromString(QString::fromStdString(task.params().dt_end()), Qt::ISODate)
                                                     : QDateTime();


  meteo::surf::DataRequest request;
  if (beginDate.empty() == false) {
      request.set_date_start(beginDate);
    }
  else if (task.params().has_dt_begin()) {
      request.set_date_start(task.params().dt_begin());
    }

  if (endDate.empty() == false) {
      request.set_date_end(endDate);
    }

  request.clear_type();
  switch (task.params().type()) {
    case meteo::obanalsettings::kAero:
      request.add_type(OBANAL_TYPE::kAero);
      break;
    case meteo::obanalsettings::kRadar:
      request.add_type(kRadar);
      break;
    case meteo::obanalsettings::kSurface:
      request.add_type(OBANAL_TYPE::kSurface);
      break;
    case meteo::obanalsettings::kOcean:
      request.add_type(OBANAL_TYPE::kOcean);
      break;
    default:
      break;
    }
  if ( 0 == request.type_size() ) {
      error_log << QObject::tr("Ошибка. Не установлен ни один тип данных");
      return false;
    }
  int datatype = request.type(0);
  request.set_max_srok(0);
  if (!request.has_date_start()) {
      QDateTime dt = QDateTime::currentDateTimeUtc();
      dt.setTime( QTime( dt.time().hour(), 0) );
      request.set_date_start(dt.toString(Qt::ISODate).toStdString() );
    }
  QList<int> descrs;
  if (task.params().descr_size() > 0) {
      if (task.params().descr_size() == 1 && task.params().descr(0) == -1) {
          meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
          if ( nullptr == ctrl ) {
              return false;
            }
          meteo::sprinf::MeteoParameterRequest req;
          meteo::sprinf::MeteoParameters* res = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetMeteoParametersByBufr, req, 5000);
          delete ctrl;
          if (res != nullptr) {
              for (int i = 0, sz = res->parameter_size(); i < sz; ++i) {
                  descrs.append(res->parameter(i).bufr());
                }
            }
          delete res;
        }
      else {
          for (int j = 0, jsz = task.params().descr_size(); j < jsz; ++j) {
              descrs.append(task.params().descr(j));
            }
        }
    }

  QList<int> centers;
  for (int j = 0, jsz = task.params().center_size(); j < jsz; ++j) {
      if (task.params().center(j) != -1) {
          centers.append(task.params().center(j));
        }
    }

  QList<int> levels;
  int levelsSize = task.params().level_size();
  if (levelsSize > 0 && task.params().level(0) != -1) {
      for (int j = 0; j < levelsSize; ++j) {
          levels.append(task.params().level(j));
          switch (datatype) {
            case OBANAL_TYPE::kAero:
              if (!request.has_type_level()) {
                  request.set_type_level(100);
                }
              break;
            case OBANAL_TYPE::kRadar:
              if (!request.has_type_level()) {
                  request.set_type_level(105);
                }
              break;
            case OBANAL_TYPE::kOcean:
              if (!request.has_type_level()) {
                  //                      request.set_type_level(105);
                  request.set_type_level(1);
                }
              break;
            case OBANAL_TYPE::kSurface:
              if (!request.has_type_level()) {
                  request.set_type_level(1);
                }
              break;
            default:
              break;
            }
        }
    }
  if (centers.isEmpty()) {
      fillDescrRequests(requests,levels,startDateTime,endDateTime,datatype, descrs, request);
    }
  else {
      for (int centerNum = 0; centerNum < centers.count(); ++centerNum) {
          request.clear_station();
          request.add_station( QString::number(centers.at(centerNum)).toStdString() );
          fillDescrRequests(requests,levels,startDateTime,endDateTime,datatype, descrs, request);
        }
    }
  return true;
}

void Maker::run_fromFile( commons::ArgParser* options, meteo::surf::GribDataRequest* grib_request,
                                QList<meteo::surf::DataRequest> *requests, QList<meteo::surf::GribDataRequest> *gribRequests,
                                const std::string &beginDate, const std::string &endDate )
{
  if ( false == options->hasValues(kFile) ) {
      error_log << QObject::tr("Отсутвует имя файла");
      return;
    }
  QList<int> tasksUids;
  if ( true == options->hasValues(kTasksUid) ) {
      foreach (const QString& each, options->at(kTasksUid).values()) {
          bool ok = false;
          int uid = each.toInt(&ok);
          if (ok == true) {
              tasksUids.append(uid);
            }
        }
    }
  QString tasksFileName(options->at(kFile).value());
  if ( true == tasksFileName.isEmpty() ) {
      error_log << QObject::tr("Не задано имя файла");
      return;
    }
  meteo::obanalsettings::Settings config;
  if ( false == TProtoText::fromFile(tasksFileName, &config)) {
      error_log << QObject::tr("Не удалось прочитать настройки из файла ") << tasksFileName;
      return;
    }
  bool isAllTasks = tasksUids.isEmpty();
  for (int i = 0, sz = config.task_size(); i < sz; ++i) {
      if (!(isAllTasks == true || tasksUids.contains(config.task(i).uid()) == true)) {
          continue;
        }
      QDateTime startDateTime = config.task(i).params().has_dt_begin() ? QDateTime::fromString(QString::fromStdString(config.task(i).params().dt_begin()), Qt::ISODate)
                                                                       : QDateTime();
      QDateTime endDateTime = config.task(i).params().has_dt_end() ? QDateTime::fromString(QString::fromStdString(config.task(i).params().dt_end()), Qt::ISODate)
                                                                   : QDateTime();
      if (config.task(i).params().type() == meteo::obanalsettings::kGrib) {
          taskGribHandler(config.task(i), grib_request, gribRequests);
        }
      else {
          taskHandler(config.task(i), requests, beginDate, endDate );
        }
    }
}

void Maker::getDt( const surf::DataRequest& rdt,QDateTime *dts, QDateTime *dte, int hours_to)
{
  *dts = QDateTime::currentDateTimeUtc();
  *dte = QDateTime::currentDateTimeUtc();
  *dts = dts->addSecs(hours_to*3600);

  if(rdt.has_date_start() ){
    *dts = QDateTime::fromString( QString::fromStdString(rdt.date_start()),Qt::ISODate);
  }
  if( rdt.has_date_end()){
    *dte = QDateTime::fromString( QString::fromStdString(rdt.date_end()), Qt::ISODate);
  }

  QTime tm = dte->time();
  tm.setHMS( tm.hour(), 0, 0 );
  dte->setTime(tm);
  tm = dts->time();
  tm.setHMS( tm.hour(), 0, 0 );
  dts->setTime(tm);
}

bool Maker::obanalPoDescrs(const surf::DataRequest &rdt){
  if (options_->installed(kToJSON)){
    if ( json_saved_ ){
      return false;
    }
    return saveJSONFile();
  }
  return omaker_->obanalPoDescrs(rdt);
}

/**
 * @brief Сохраняем просто поле в файл
 * 
 * @param rdt 
 * @return true 
 * @return false 
 */
bool Maker::saveJSONFile(){
  meteo::field::DataRequest request;

  QString param = "";
  QString level = "";
  
  // request.set_date_start( req_date_->addSecs( req_forecasthour_ * 3600 ).toString(Qt::ISODate).toStdString() );
  // request.set_date_start( req_date_->addSecs( req_hour_ -  req_forecasthour_ * 3600 ).toString(Qt::ISODate).toStdString() );
  if ( true == options_->hasValues(kBeginDate) ) {
    request.set_date_start( options_->at(kBeginDate).value().toStdString() );
  }
  if ( true == options_->hasValues(kFinishDate) ) {
    // request.set_date_end( options_->at(kFinishDate).value().toStdString() );
  }
  if ( true == options_->hasValues(kCenter) ) {
    request.add_center(options_->at(kCenter).value().toInt());
  }
  if ( true == options_->hasValues(kHour) ) {
    request.add_hour(options_->at(kHour).value().toInt());
  }
  if ( true == options_->hasValues(kModel) ) {
    request.set_model(options_->at(kModel).value().toInt());
  }
  if ( true == options_->hasValues(kLevelP) ) {
    request.add_level(options_->at(kLevelP).value().toInt());
    if ( options_->at(kLevelP).value().toInt()==0 ){
      level="surface-level";
    }else{
      level=QString("isobaric-%1hPa").arg(options_->at(kLevelP).value().toInt());
    }
  }
  if ( true == options_->hasValues(kLevelType) ) {
    request.add_type_level(options_->at(kLevelType).value().toInt());
  }

  
  // request.add_hour(       req_forecasthour_*3600 );
  // request.add_level(      req_level_     );
  // request.add_type_level( req_level_type_);
  // request.add_meteo_descr(req_param_     );
  // request.add_center(     req_center_    );
  // request.set_model(      req_model_     );
  request.set_need_field_descr( true );

  // STEP_500x500 = 1,  //!< через 5 градусов по широте и долготе
  // STEP_250x250 = 2,  //!< через 2.5 градуса по широте и долготе
  // STEP_125x125 = 3,  //!< через 1.25 градуса по широте и долготе
  // STEP_100x100 = 4,  //!< через 1 градус по широте и долготе
  // STEP_0625x0625 = 5,//!< через 0.625 градуса по широте и долготе
  // STEP_2983x100 = 6, //!< через 2.983 градуса по широте и долготе
  // STEP_0500x0500 = 7, //!< через .5 градуса по широте и долготе
  // STEP_0100x0100 = 8, //!< через .1 градуса по широте и долготе
  request.set_net_type( 2 );
  // request.set_is_df(      false     );
  // request.set_net_type()

  QJsonArray mainArray;

  // создаем синглтон для работы с дескрипторами
  // надо для того, чтобы из числа получить букву
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  bool all_ok=true;
  
  if ( true == options_->hasValues(kDescr) ) {
    QStringList descrlist = options_->at(kDescr).values();
    for ( int i = 0, sz = descrlist.size(); i < sz; ++i ) {
      request.clear_meteo_descr();
      request.add_meteo_descr(descrlist[i].toInt());
      // запрагиваем дескриптор по имени
      param = md->name(descrlist[i].toInt());
      QJsonObject obj_ = getObjectFromFiled(request, param, level);
      if (obj_.isEmpty()){
        all_ok = false;
      }
      mainArray.push_back( obj_ );
    }
  }

  QJsonDocument doc(mainArray);

  if (param=="T"){
    param="temp";
  }else if (param=="u" || param=='v'){
    param="wind";
  }

  if (all_ok){
    saveJson( doc, QString("current-%1-%2-gfs-1.0.json").arg(param).arg(level) );
  }

  json_saved_= true;
  return true;
}

QJsonObject Maker::getObjectFromFiled( meteo::field::DataRequest& request, QString& param, QString& level ){
  meteo::field::DataReply* reply = nullptr;
  ::obanal::TField*        field = nullptr;
  QJsonObject mainObject;
  field = new ::obanal::TField();

  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kField );
  if(nullptr == ctrl) {
    error_log << meteo::msglog::kServiceConnectFailedSimple
                  .arg(meteo::global::serviceAddress( meteo::settings::proto::kField ));
    return mainObject;
  }
  reply = ctrl->remoteCall( &meteo::field::FieldService::GetFieldData, request, 50000 );

  debug_log <<  QString("Данные приняты, всего %1").arg(reply->fielddata().size()) ;

  QByteArray arr( reply->fielddata().data(), reply->fielddata().size() );
  
  debug_log << request.DebugString();
  // debug_log << reply->DebugString() << reply->comment();
  debug_log << "Data count" << reply->fielddata().size();
  // debug_log << reply->fielddata().data();
  // загружаем данные
  field->fromBuffer(&arr);
  if (field->kolData()==0){
    error_log << "Данные не получены. Расчет невозможен";
    return mainObject;
  }
  debug_log <<  QString("Данные приняты, всего %1").arg(field->kolData()) ;

  QJsonArray dataArray;
  int sizeofdata = 0;
  // int sizeofdata = field->kolData();
  // for ( int i = sizeofdata; i > 0; --i ) {    
  //   dataArray.push_back( field->data()[i] );    
  // }
  meteo::GeoPoint point;
  // int points_i = 0;
  // int points_j = 0;
  for ( int j = 90; j >= -90; --j ) {   
    // points_j = 0; 
    for ( int i = 0; i < 360; ++i ) {   
      point.setLatDeg(float(j));
      point.setLonDeg(float(i));
      // debug_log << "lat "<< float(i) << " " << point.lat() << " lon "<< float(j)<< " " << point.lon()  << " = "<<field->pointValue(point);
      dataArray.append( field->pointValue(point) );
      // sizeofdata++;
      // points_j++;
    }
    // points_i++;
  }
  // debug_log << "points_j " << points_j ;
  // debug_log << "points_i " << points_i ;
  // debug_log << "size " << sizeofdata ;
  mainObject.insert("data", dataArray);

  QJsonObject headerObject;
  
  headerObject.insert( "discipline"                   , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "disciplineName"               , QJsonValue::fromVariant( "Meteorological products" ) );
  headerObject.insert( "gribEdition"                  , QJsonValue::fromVariant( 2 ) );
  headerObject.insert( "gribLength"                   , QJsonValue::fromVariant( 131858 ) );
  headerObject.insert( "center"                       , QJsonValue::fromVariant( 7 ) );
  headerObject.insert( "centerName"                   , QJsonValue::fromVariant( "52 kafedra" ) );
  headerObject.insert( "subcenter"                    , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "refTime"                      , QJsonValue::fromVariant( field->getDate().toString(Qt::ISODate)+".000Z" ) ); // "2014-01-31T00:00:00.000Z"
  headerObject.insert( "significanceOfRT"             , QJsonValue::fromVariant( 1 ) );
  headerObject.insert( "significanceOfRTName"         , QJsonValue::fromVariant( "Start of forecast" ) );
  headerObject.insert( "productStatus"                , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "productStatusName"            , QJsonValue::fromVariant( "Operational products" ) );
  headerObject.insert( "productType"                  , QJsonValue::fromVariant( 1 ) );
  headerObject.insert( "productTypeName"              , QJsonValue::fromVariant( "Forecast products" ) );
  headerObject.insert( "productDefinitionTemplate"    , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "productDefinitionTemplateName", QJsonValue::fromVariant( "Analysis/forecast at horizontal level/layer at a point in time" ) );
  headerObject.insert( "parameterCategory"            , QJsonValue::fromVariant( 2 ) );
  headerObject.insert( "parameterCategoryName"        , QJsonValue::fromVariant( "Momentum" ) );
  headerObject.insert( "parameterNumber"              , QJsonValue::fromVariant( 2 ) );
  headerObject.insert( "parameterNumberName"          , QJsonValue::fromVariant( param+"  "+level ) );
  headerObject.insert( "parameterUnit"                , QJsonValue::fromVariant( "C" ) ); //m.s-1
  headerObject.insert( "genProcessType"               , QJsonValue::fromVariant( 2 ) );
  headerObject.insert( "genProcessTypeName"           , QJsonValue::fromVariant( "Forecast" ) );
  headerObject.insert( "forecastTime"                 , QJsonValue::fromVariant( options_->at(kHour).value().toInt()/3600 ) );
  headerObject.insert( "surface1Type"                 , QJsonValue::fromVariant( 103 ) );
  headerObject.insert( "surface1TypeName"             , QJsonValue::fromVariant( "Specified height level above ground" ) );
  headerObject.insert( "surface1Value"                , QJsonValue::fromVariant( 10 ) );
  headerObject.insert( "surface2Type"                 , QJsonValue::fromVariant( 255 ) );
  headerObject.insert( "surface2TypeName"             , QJsonValue::fromVariant( "Missing" ) );
  headerObject.insert( "surface2Value"                , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "gridDefinitionTemplate"       , QJsonValue::fromVariant( 0 ) );
  // headerObject.insert( "gridDefinitionTemplateName"   , QJsonValue::fromVariant( "Latitude_Longitude" ) );
  headerObject.insert( "numberPoints"                 , QJsonValue::fromVariant( sizeofdata ) );
  headerObject.insert( "shape"                        , QJsonValue::fromVariant( 6 ) );
  headerObject.insert( "shapeName"                    , QJsonValue::fromVariant( "Earth spherical with radius of 6,371,229.0 m" ) );
  headerObject.insert( "gridUnits"                    , QJsonValue::fromVariant( "degrees" ) );
  headerObject.insert( "resolution"                   , QJsonValue::fromVariant( 48 ) );
  headerObject.insert( "winds"                        , QJsonValue::fromVariant( "true" ) );
  headerObject.insert( "scanMode"                     , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "nx"                           , QJsonValue::fromVariant( 360 ) );
  headerObject.insert( "ny"                           , QJsonValue::fromVariant( 181 ) );
  headerObject.insert( "basicAngle"                   , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "subDivisions"                 , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "lo1"                          , QJsonValue::fromVariant( 0 ) );
  headerObject.insert( "la1"                          , QJsonValue::fromVariant( 90 ) );
  headerObject.insert( "lo2"                          , QJsonValue::fromVariant( 359 ) );
  headerObject.insert( "la2"                          , QJsonValue::fromVariant( -90 ) );
  headerObject.insert( "dx"                           , QJsonValue::fromVariant( 1 ) );
  headerObject.insert( "dy"                           , QJsonValue::fromVariant( 1 ) );

  mainObject.insert("header", headerObject);

  QJsonObject metaObject;
  metaObject.insert( "date"                           , QJsonValue::fromVariant( field->getDate().toString(Qt::ISODate)+".000Z" ) );
  mainObject.insert("meta", metaObject);
  
  delete field;
  field = nullptr;
  delete reply;
  reply = nullptr;
  delete ctrl;
  ctrl = nullptr;

  return mainObject;
}

QJsonDocument Maker::loadJson(QString fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    return QJsonDocument().fromJson(jsonFile.readAll());
}

void Maker::saveJson(QJsonDocument document, QString fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
}

bool Maker::checkParams( const surf::DataRequest& rdt){
  if ( false == rdt.has_type_level()) {
      warning_log << QObject::tr("Параметр ") << kLevelType.longname()
                  << QObject::tr("Необходимо установить параметр ") << kLevelType.longname();
      return false;
    }
  if ( false == rdt.has_level_p() ) {
      error_log << QObject::tr("Анализ аэрологичеких данных не произведён.")
                << QObject::tr("Необходимо установить параметр ") << kLevelP.longname();
      return false;
    }
  if ( 0 == rdt.meteo_descr_size() ) {
      error_log << QObject::tr("Анализ аэрологичеких данных не произведён.")
                << QObject::tr("Необходимо установить параметр ") << kDescr.longname();
      return false;
    }
  return true;
}

}
}
