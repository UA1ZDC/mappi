#include "tforecastlist.h"
#include "tforecast.h"

#include <qdir.h>
#include <qprocess.h>

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/log.h>

TForecastList::TForecastList(QObject *parent, bool is_service) :
  QObject(parent),cur_synSit_(-1),cur_season_(-1)
{
  date_time_ = QDateTime::currentDateTimeUtc();
  cur_season_ = TForecast::oprSeason(date_time_.date());
  dataProvider_  = new TDataProvider(date_time_,is_service);
  dataProcessor_ = new TDataProcessor(dataProvider_);
}

TForecastList::TForecastList(const QDateTime& adate_time_,  const StationData &astation, QObject *parent) :
  QObject(parent),date_time_(adate_time_),station_(astation),cur_synSit_(-1),cur_season_(-1)
{
  cur_season_ = TForecast::oprSeason(adate_time_.date());
  dataProvider_  = new TDataProvider(date_time_);
  dataProcessor_ = new TDataProcessor(dataProvider_);
}

TForecastList::~TForecastList(){
  qDeleteAll(methodsList_);
  delete dataProvider_;
  delete dataProcessor_;
}

void TForecastList::checkProvider()
{
  if(nullptr != dataProvider_ && nullptr != dataProcessor_) {
    return;
  }
  if(nullptr == dataProvider_ ) {
    dataProvider_  = new TDataProvider(date_time_);
    if(nullptr != dataProcessor_ ){
      delete dataProcessor_;
    }
    if(nullptr == dataProcessor_ ){
      dataProcessor_ = new TDataProcessor(dataProvider_);
    }
  }
  if(nullptr == dataProcessor_ && nullptr != dataProvider_ ) {
    dataProcessor_ = new TDataProcessor(dataProvider_);
  }
}


const QMap<QString,TForecast*> TForecastList::methodsList(){
  return methodsList_;
}

void TForecastList::setSynSit(int ss){
  cur_synSit_ = ss;
  foreach(TForecast* fm , methodsList_){
    if(fm) fm->setSynSit(ss);
  }
}

void TForecastList::setStation(const StationData &astation){
  station_ = astation;
  foreach(TForecast* fm , methodsList_){
    if(fm) fm->setStation(astation);
  }
}

void TForecastList::setFrontType(int ss){
  //cur_synSit_ = ss;
  foreach(TForecast* fm , methodsList_){
    if(fm) fm->setFrontType(ss);
  }
}


void TForecastList::setSeason(int as){
  cur_season_ = as;
  foreach(TForecast* fm , methodsList_){
    if(fm) fm->setSeason(as);
  }
}


int TForecastList::getSynSit(){
  return cur_synSit_;
}

int TForecastList::getSeason(){
  return cur_season_;
}

const QStringList TForecastList::methodYavlTypes(){
  return methodYavlTypes_;
}


void TForecastList::setDateTime(const QDateTime& dt){
  date_time_ = dt;
  checkProvider();
  dataProvider_->setDate(dt);
}


void TForecastList::addMethod(const QString& scriptName,bool need_ui)
{
  if(nullptr == dataProvider_) return;
  if(nullptr == dataProcessor_) return;

  TForecast *forecast = new TForecast(dataProvider_, dataProcessor_, this,station_);
  forecast->setSynSit(cur_synSit_);
  forecast->setSeason(cur_season_);
  forecast->loadScript(scriptName);
  forecast->tryRunForecast(need_ui);
  methodsList_.insert(forecast->methodName(),forecast);
  methodYavlTypes_.append(forecast->methodYavlType());
}

TForecast * TForecastList::addMethodNoRun(const QString& scriptName)
{
  if(nullptr == dataProvider_) return nullptr;
  if(nullptr == dataProcessor_) return nullptr;

  TForecast *forecast = new TForecast(dataProvider_, dataProcessor_, this,station_);
  forecast->setSynSit(cur_synSit_);
  forecast->setSeason(cur_season_);
  forecast->loadScript(scriptName);
  QString mname = forecast->methodName();
  if(methodsList_.contains(mname)){
    delete forecast;
    return methodsList_.value(mname);
  } else {
    methodsList_.insert(mname,forecast);
    methodYavlTypes_.append(forecast->methodYavlType());
  }
  return forecast;
}

int TForecastList::loadMethodsNoRun(const QString & scripts_path)
{
  QDir dir(scripts_path);
  dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
  dir.setSorting(QDir::Name);
  QStringList filters;
  filters << "*.js";
  dir.setNameFilters(filters);
  QFileInfoList list = dir.entryInfoList();
  for (int i = 0; i < list.size(); ++i) {
    QFileInfo fileInfo = list.at(i);
    if(fileInfo.fileName().contains("forecast")) continue; //TODO
    setDateTime(QDateTime::currentDateTimeUtc());
    addMethodNoRun(fileInfo.fileName());
  }

  return methodsList_.count();
}

int TForecastList::loadMethods(const QString & scripts_path,bool need_ui)
{
  QDir dir(scripts_path);
  dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
  dir.setSorting(QDir::Name);
  QStringList filters;
  filters << "*.js";
  dir.setNameFilters(filters);
  QFileInfoList list = dir.entryInfoList();
  for (int i = 0; i < list.size(); ++i) {
    QFileInfo fileInfo = list.at(i);
    if(fileInfo.fileName().contains("forecast")) continue; //TODO
    setDateTime(QDateTime::currentDateTimeUtc());
    addMethod(fileInfo.fileName(),need_ui );
  }

  return methodsList_.count();
}


QString TForecastList::fileListName()const{
  return station_.index
      +QString::number(station_.coord.fiDeg(),'f',2)
      +QString::number(station_.coord.laDeg(),'f',2)
      +".meth.list";

}

bool TForecastList::validateMethod(const QString & mn){

  QString methods_valid_file = meteo::global::kForecastMethodsValidDir+"/"+ fileListName();
  meteo::forecast::ForecastMethodList ml;
  if(false == TProtoText::fromFile(methods_valid_file,&ml) ){
    //если файла с настройками прогнозирования нет, то прогнозируем по всем методам
    return true;
  }

  for(int i=0; i< ml.method_size();++i){
    QString mm = pbtools::toQString(ml.method(i).name());
    if(0 == mm.compare(mn, Qt::CaseInsensitive) ){
      return ml.method(i).isauto();
    }
  }
  return true; //false; NOTE логично вроде, чтоб true, т.к. при отсутствии файла - true
}

bool TForecastList::setValidateMethod(const QString & mn, bool rr ){
  QString methods_valid_file = meteo::global::kForecastMethodsValidDir+"/"+fileListName();
  bool res = false;
  QDir dir;
  if(!dir.mkpath(meteo::global::kForecastMethodsValidDir)){
    return false;
  }
  QProcess chmod;
  chmod.start("chmod", QStringList() << "777" << meteo::global::kForecastMethodsValidDir);
  chmod.waitForFinished();

  meteo::forecast::ForecastMethodList ml;
  if(!TProtoText::fromFile(methods_valid_file,&ml)){
    if(!TProtoText::toFile(ml,methods_valid_file)){
      return false;
    }
  }
  for(int i=0; i< ml.method_size();++i){
    QString mm = pbtools::toQString(ml.method(i).name());
    if(0 == mm.compare(mn, Qt::CaseInsensitive) ){
      ml.mutable_method(i)->set_isauto(rr);
      res = true;
    }
  }
  if(!res){
    meteo::forecast::ForecastMethod* nwm = ml.add_method();
    nwm->set_name(mn.toStdString());
    nwm->set_isauto(rr);
  }
  res = TProtoText::toFile(ml,methods_valid_file);

  chmod.start("chmod", QStringList() << "666" << methods_valid_file);
  chmod.waitForFinished();

  return res;
}

TForecast* TForecastList::getMethod(const QString& methodName)
{
  return methodsList_.value(methodName);
}
