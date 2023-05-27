#include "tforecast.h"
#include "tforecastlist.h"

#include <QDebug>
#include <qfile.h>
// #include <QtScriptTools/qscriptenginedebugger.h>
#include <QMainWindow>
#include <QAction>
#include <QToolBox>
#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QGridLayout>
#include <QtUiTools/QUiLoader>
#include <QScriptClass>
#include <QHeaderView>
#include <QGroupBox>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/custom/stationwidget.h>

#include "tfdelegate.h"
#include "tdataprovider.h"
#include "tfmodel.h"
#include "tforecastlist.h"
#include "tfitem.h"


struct QtMetaObject : private QObject
{
public:
  static const QMetaObject *get()
  { return &static_cast<QtMetaObject*>(nullptr)->staticQtMetaObject; }
};

TForecast::TForecast( TDataProvider* adataProvider_, TDataProcessor* adataProcessor_, TForecastList* amethodsList_,const StationData & ast)
  :ui_(nullptr),
    engine_(nullptr),
    srcModel_(nullptr),
    resModel_(nullptr),
    delegat_(nullptr),
    cur_synSit_(-1),
    cur_season_(-1),
    station_(ast)
{
  methodsList_ = amethodsList_;
  dataProvider_ = adataProvider_;
  dataProcessor_ = adataProcessor_;
  methodHours_ = "0;24";
  methodTimes_ = "0";
  front_type_ = 0;
  engine_ = new QScriptEngine(this);
}

TForecast::~TForecast() {
  if ( nullptr != srcModel_ ){
    delete srcModel_;
    srcModel_ = nullptr;
  }
  if ( nullptr != resModel_ ){
    delete resModel_;
    resModel_ = nullptr;
  }
  if ( nullptr != delegat_ ){
    delete delegat_;
    delegat_ = nullptr;
  }
  if ( nullptr != engine_ ){
    delete engine_;
    engine_ = nullptr;
  }
}


QString TForecast::stringSeason(int season)
{
  QString s = "Не определен";
  switch (season)
  {
    case ZIMA:
      s = "Зима";
    break;
    case VESNA:
      s = "Весна";
    break;
    case LETO:
      s = "Лето";
    break;
    case OSEN:
      s = "Осень";
    break;
  }
  return s;
}

QString TForecast::stringSynSit(int synsit)
{
  switch (synsit)
  {
    case 0:
    return "Центральная часть циклона или ось ложбины";
    case 1:
    return "Передняя часть циклона или передняя часть ложбины";
    case 2:
    return "Теплый сектор циклона";
    case 3:
    return "Тыловая часть циклона или тыловая часть ложбины";
    case 4:
    return "Центральная часть антициклона или ось барического гребня";
    case 5:
    return "Северная периферия антициклона или северная часть барического гребня";
    case 6:
    return "Западная периферия антициклона или тыловая часть барического гребня";
    case 7:
    return "Восточная периферия антициклона или передняя часть барического гребня";
    case 8:
    return "Южная периферия антициклона или отрог(ядро)";
    case 9:
    return "Малоградиентная область пониженного (повышенного) давления";
    case 10:
    return "Седловина или перемычка пониженного (повышенного) давления";
  }
  return "Не определена";
}

int TForecast::oprSeason(const QDate& ad){
  int y = ad.year();
  QDate z = QDate(y,12,21);//наступает зима
  QDate v = QDate(y,3,20);//наступает весна
  QDate l = QDate(y,6,20);//наступает лето
  QDate o = QDate(y,9,23);//наступает осень

  if(ad >=z || ad < v ){
    return ZIMA;
  }
  if(ad >=v && ad< l ){
    return VESNA;
  }
  if(ad >=l && ad< o ){
    return LETO;
  }
  if(ad >=o && ad< z ){
    return OSEN;
  }
  return LETO;
}

/**
 * загружаем методы в ГУИ
 */

void TForecast::loadModels()
{
  if (nullptr == ui_) return;
  QTreeView *ait = ui_->findChild< QTreeView *>("treeViewSrc");
  if(nullptr == ait) return;
  ait->setModel(srcModel_);
  ait->setItemDelegate(delegat_);
  ait->setWordWrap ( true );
  ait->setTextElideMode ( Qt::ElideNone );

  int width = ait->fontMetrics().size( Qt::TextExpandTabs , tr("Простая сторка для отображения отображения\n")).width();

  ait->setColumnWidth(0, width);
  ait->setColumnWidth(1,ait->fontMetrics().size( Qt::TextExpandTabs , tr("Простая сторка для \n")).width());
  ait->expandAll ();

  ait->header()->setSectionResizeMode(QHeaderView::Interactive);
  ait = ui_->findChild<QTreeView *>("treeViewResult");
  if(nullptr == ait) return;
  ait->setModel(resModel_);
  ait->setItemDelegate(delegat_);
  ait->setWordWrap ( true );
  ait->setTextElideMode ( Qt::ElideNone );
  ait->setColumnWidth(0,width);
  ait->setColumnWidth(1,width);

  ait->header()->setSectionResizeMode(QHeaderView::Interactive);

  /**
     * задаем стили для гроупбоксов
     * чтобы шрифт был жирный у результатов
     * чтобы рамки были зелеными, когда все резульаты рассчитаны (нет бадвалуев)
     * gb->setProperty("success",  true);
     */
  QGroupBox *gb = ui_->findChild<QGroupBox *>("forecastBoxPage2");
  gb->setStyleSheet(this->GBStyle+ " QGroupBox { font-weight:bold; }" ); //

  gb = ui_->findChild<QGroupBox *>("forecastBoxPage1");
  gb->setStyleSheet( this->GBStyle );

}

//****************************************************************
//**                     Методы для работы со скриптами                       **
//****************************************************************
QScriptValue toScriptStationData(QScriptEngine *engine, const StationData &s)
{
  QScriptValue obj = engine->newObject();
  obj.setProperty("fi", QScriptValue(engine, s.coord.fiDeg()));
  obj.setProperty("la", QScriptValue(engine, s.coord.laDeg()));
  obj.setProperty("h", QScriptValue(engine, s.coord.alt()));
  obj.setProperty("name", QScriptValue(engine, s.name));
  obj.setProperty("index", QScriptValue(engine, s.index));
  obj.setProperty("station_type", QScriptValue(engine, s.station_type));
  return obj;
}

void fromScriptStationData(const QScriptValue &obj, StationData &s)
{
  s.coord.setFiDeg(obj.property("fi").toNumber());
  s.coord.setLaDeg( obj.property("la").toNumber());
  s.coord.setAlt(obj.property("h").toNumber());
  s.name = obj.property("name").toString();
  s.index = obj.property("index").toString();
  s.station_type = meteo::sprinf::MeteostationType(obj.property("station_type").toInteger());
}

QScriptValue toScriptGeoPoint(QScriptEngine *engine, const meteo::GeoPoint &s)
{
  QScriptValue obj = engine->newObject();
  obj.setProperty("fi", QScriptValue(engine, s.fiDeg()));
  obj.setProperty("la", QScriptValue(engine, s.laDeg()));
  obj.setProperty("alt", QScriptValue(engine, s.alt()));
  return obj;
}

void fromScriptGeoPoint(const QScriptValue &obj, meteo::GeoPoint &s)
{
  s.setFiDeg( obj.property("fi").toNumber());
  s.setLaDeg(obj.property("la").toNumber());
  s.setAlt(obj.property("alt").toNumber());
}


void TForecast::loadScript(const QString& scriptName)
{

  QScriptValue scriptObj = engine_->newQObject(this);
  QScriptValue zondObj   = engine_->newQObject(dataProcessor_);
  QScriptValue dataObj   = engine_->newQObject(dataProvider_);
  qMetaTypeId<QString>();
  qScriptRegisterMetaType(engine_, toScriptStationData, fromScriptStationData);
  qScriptRegisterMetaType(engine_, toScriptGeoPoint, fromScriptGeoPoint);


  engine_->globalObject().setProperty("obj", scriptObj);
  engine_->globalObject().setProperty("zondObj", zondObj);
  engine_->globalObject().setProperty("dataObj", dataObj);

  evaluateFile(engine_, scripts_path+"forecastmain.js");
  evaluateFile(engine_, scripts_path+"forecast"+scriptName); //функции реализации метода
  evaluateFile(engine_, scripts_path+scriptName);  //окно с методом

  methodName_     = engine_->globalObject().property("name").call().toString();
  methodHours_    = engine_->globalObject().property("hours").call().toString();
  methodTimes_    = engine_->globalObject().property("times").call().toString();
  methodFullName_ = engine_->globalObject().property("title").call().toString();
  methodIconName_ = engine_->globalObject().property("icon").call().toString();
  methodSeason_   = engine_->globalObject().property("season").call().toString();
  methodSynSit_   = engine_->globalObject().property("synsit").call().toString();
  methodYavlType_ = engine_->globalObject().property("typeYavl").call().toString();
  methodHelpFile_ = engine_->globalObject().property("methodHelpFile").call().toString();

}



void TForecast::tryRunForecast(bool need_ui)
{
  if(!setMethodTime()) return;
  if(ui_){
    ui_->show();
  }
  runForecast(need_ui);

}

bool TForecast::setMethodTime(){

  if(!checkSeason()){
    info_log << tr("Метод ") <<methodFullName_ << tr(" не предназначен для работы в это время года.");
    return false;
  }

  if(!checkSynSit()){
    info_log << tr("Метод ") << methodFullName_  << tr(" не предназначен для работы в этой синоптической ситуации.");
    return false;
  }
  QDateTime cdt = getDate();
  QTime tm = cdt.time();
  QStringList tlist =  methodTimes_.split(';');
  int cur_hour = -1;
  bool ok = false;
  bool eq = false;
  for(int i=0;i < tlist.size();++i){
    cur_hour = tlist.at(i).toInt(&ok);

    if(!ok){
      debug_log<< tr("Ошибка в скрипте (функция times())");
      return false;
    }
    if(tm.hour() == cur_hour){
      eq = true;
      break;
    }
  }

  if(!eq){
    for(int i=0;i < tlist.size();++i){
      if(tm.hour() < tlist.at(i).toInt(&ok)+1){
        break;
      }
      cur_hour = tlist.at(i).toInt(&ok);

      if(!ok){
        debug_log<< tr("Ошибка в скрипте (функция times())");
        return false;
      }
    }
  }
  if(cur_hour <0){
    info_log << tr("Запуск метода ") << methodFullName_ << tr(" отложен. Время запуска ") << methodTimes_<<cur_hour;
    return false;
  }

  tm.setHMS(cur_hour,0,0);
  cdt.setTime(tm);
  dataProvider_->setDate(cdt);
  setUiTitle();
  return true;

}

bool TForecast::checkSeason(){
  if(!methodSeason_.contains("-1")){
    if(!methodSeason_.contains(QString::number( cur_season_))){
      return false;
    }
  }
  return true;
}


bool TForecast::checkSynSit(){
  if(!methodSynSit_.contains("-1")){
    if(!methodSynSit_.contains(QString::number( cur_synSit_))){
      return false;
    }
  }
  return true;
}


void TForecast::runForecastWithFilledModelOnly(bool need_ui)
{
  info_log << tr("Прогнозирование методом ") << methodFullName_ << "("<<methodHours_<< tr("ч.)") << getDate().toString(Qt::ISODate);
  

  // подключаем модель к изменению стиля рамки
  if(true == need_ui){
    QObject::connect( srcModel_, SIGNAL(allGoodChanges()), this, SLOT( checkGBStyles() ) );
  }

  if(nullptr != resModel_){
    delete resModel_;
    resModel_ = nullptr;
  }
  if(nullptr == resModel_){
    resModel_ = new TFModel(dataProvider_, this, "result");
    // подключаем модель к изменению стиля рамки
    if(true == need_ui){

      QObject::connect( resModel_, SIGNAL(allGoodChanges()), this, SLOT( checkGBStyles() ) );
    }

  }
  if(nullptr == delegat_ && need_ui){
    delegat_ = new TFItemDelegate(this);
  }
  if(nullptr == ui_){
    initScript( need_ui );
    if(need_ui){
      loadUi("forecast_simple_new.ui");
    }
    // loadModels();
    // checkGBStyles();
  }

}

/**
 * @brief запускаем инициализацию скрипта
 * 
 */
void TForecast::initScript(bool need_ui)
{
  QScriptValue init = engine_->globalObject().property("init");
  init.call(QScriptValue(), QScriptValueList() << need_ui);
}

/**
 * @brief Создаем исходную модель
 * 
 */
void TForecast::createSrcModel()
{
  if(nullptr != srcModel_){
    delete srcModel_;
    srcModel_ = nullptr;
  }
  srcModel_ = new TFModel(dataProvider_, this, methodName_);

  currentParent = srcModel_->rootItem()->getDescr();
  if(nullptr == delegat_ && false){
    delegat_ = new TFItemDelegate(this);
  }
}

void TForecast::runForecast(bool need_ui)
{
  info_log << tr("Прогнозирование методом ") << methodFullName_ << "("<<methodHours_<< tr("ч.)") << getDate().toString(Qt::ISODate);

  // создаем модель с исходными данными
  createSrcModel();

  currentParent = srcModel_->rootItem()->getDescr();
  // подключаем модель к изменению стиля рамки
  if(true == need_ui){
    QObject::connect( srcModel_, SIGNAL(allGoodChanges()), this, SLOT( checkGBStyles() ) );
  }

  if(nullptr != resModel_){
    delete resModel_;
    resModel_ = nullptr;
  }
  if(nullptr == resModel_){
    resModel_ = new TFModel(dataProvider_, this, "result");
    // подключаем модель к изменению стиля рамки
    if(true == need_ui){

      QObject::connect( resModel_, SIGNAL(allGoodChanges()), this, SLOT( checkGBStyles() ) );
    }

  }
  if(nullptr == delegat_ && need_ui){
    delegat_ = new TFItemDelegate(this);
  }
  if(nullptr == ui_){
    
    initScript(need_ui);

    if(need_ui){
      loadUi("forecast_simple_new.ui");
    }
    loadModels();
    checkGBStyles();
  }
}

/**
 * проверяем все ли данные введены - и меняем стили
 */
void TForecast::checkGBStyles(){
  if (nullptr == ui_) return;

  /*
   * проверяем стили исходных данных
   */
  QGroupBox *gb = ui_->findChild<QGroupBox *>("forecastBoxPage1");
  if (  gb==nullptr ){
    return;
  }

  if ( srcModel_->isAllGood() ){
    gb->setProperty("success",true);
  }else{
    gb->setProperty("success",false);
  }
  gb->setStyleSheet( this->GBStyle );

  gb = ui_->findChild<QGroupBox *>("forecastBoxPage2");

  if ( resModel_->isAllGood() ){
    gb->setProperty("success",true);
  }else{
    gb->setProperty("success",false);
  }
  gb->setStyleSheet( this->GBStyle+ " QGroupBox { font-weight:bold; }" );

  return;
}

void TForecast::runForecastOnly(bool need_ui)
{
  if(!setMethodTime()) return;
  runForecast(need_ui);
}

TFItem *TForecast::getMainItem(const QString &aname){

  if(srcModel_){
    TFItem * vs = srcModel_->getItem(aname);
    if(nullptr != vs) return vs;
    TForecast * fvs = methodsList_->getMethod(aname);
    if(nullptr != fvs) {
      return fvs->getMainItem(aname);
    }
  }

  return nullptr;
}

QScriptValue TForecast::evaluateFile(QScriptEngine *engine, const QString &fileName)
{
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) qDebug() << "Cannot open script file:" << fileName;
  QScriptValue result = engine->evaluate( QObject::tr(file.readAll()), fileName);
  if (engine->hasUncaughtException()) {
    int lineNo = engine->uncaughtExceptionLineNumber();
    qWarning() << "QSCript: Filename" << fileName << "line" << lineNo << ":" << result.toString();

  }
  return result;
}

void TForecast::setStation(const StationData &ast)  {
  station_ = ast;
}

void TForecast::setStationName(const QString& aname ){
  station_.name=aname;

}

void TForecast::resetModels(){
  if(nullptr == srcModel_) return;
  srcModel_->resetModel();
  srcModel_->rootItem()->needResetAll(true);

  if(nullptr == resModel_) return;
  resModel_->resetModel();
  resModel_->rootItem()->needResetAll(true);

}

void TForecast::setDate(const QDateTime &adt) {
  dataProvider_->setDate( adt /*adt.toUTC()*/);
  setUiTitle();
}


StationData TForecast::getStationData()const
{
  return station_;
}



//****************************************************************
//**              Методы для работы с ГУИ                                            **
//****************************************************************

void TForecast::destroyedW(QObject *){
}

QScriptValue TForecast::loadUi(const QString& uiFileName)
{
  QUiLoader loader(this);
  QFile uiFile(ui_path+uiFileName);
  uiFile.open(QIODevice::ReadOnly);

  if (!uiFile.isOpen()){
    debug_log <<" File "+ui_path+uiFileName+" not found.";
    return 0;
  }
  ui_ = loader.load(&uiFile);
  connect(ui_, SIGNAL(destroyed(QObject*)),
          this, SLOT(destroyedW(QObject*)));
  uiFile.close();
  setUiTitle();
  return 0;
}


void TForecast::setUiTitle(){
  if(!ui_) return ;

  QLabel *lbl = ui_->findChild<QLabel *>("namelbl");
  if(lbl&&dataProvider_) {
    QString title = methodFullName();
    QDateTime dt1,dt2;
    int h = dataProvider_->getPrognozInterval(methodHours_, &dt1, &dt2);
    if(h > -1 ){
      title += QString::fromUtf8(" (с %1 до %2)")
               .arg(dt1.toString("hh:mm dd.MM"))
               .arg(dt2.toString("hh:mm dd.MM"));
    }

    lbl->setText(title);
  }
}

void TForecast::setParrent(const QString& aText,const QString& aName){
  Q_UNUSED(aText);
  Q_UNUSED(aName);

  if(nullptr == srcModel_) return;
  currentParent = aName;
  TFItem *it = srcModel_->addDataItem(aText,aName,methodName_,"-100", "-100");
  if(nullptr != it){
    it->setRole(meteo::forecast::ForecastMethodItemRole::kNoData);
  }

}

void TForecast::addDataItem(const QString& itemText,const QString& itemDescr, const QString & alvl,
                            const QString &atypelevel,const QString &asrok,const QString &aname){
  Q_UNUSED(itemText);
  Q_UNUSED(itemDescr);
  if(nullptr == srcModel_) return;
  srcModel_->addDataItem(itemText,itemDescr,currentParent,alvl, atypelevel,asrok,aname );
}


void TForecast::addDTItem(const  QScriptValue& list)
{
  if(nullptr == srcModel_) return;
  QStringList vs = list.toVariant().toString().split("//");
  if(vs.length() < 3){
    debug_log << "bad arguments"<<vs;
    return;
  }
  QScriptValue global = engine_->globalObject();
  QString func_name = vs.at(1).section('_',0,0);
  QScriptValue func = global.property(func_name);
  srcModel_->addDTItem(func, list, currentParent);
}

void TForecast::addForecastDataItem(const QString& itemText,const QString& itemDescr, const QString &alvl, const QString &atypelevel, const QString &srok ){
  Q_UNUSED(itemText);
  Q_UNUSED(itemDescr);
  if(nullptr == srcModel_) return;
  srcModel_->addDataItem(itemText,itemDescr,currentParent,alvl, atypelevel,srok );
}

void TForecast::addSimpleItem(const QString& itemText,const QString& descrName, const QString& val){
  Q_UNUSED(itemText);
  if(nullptr == srcModel_) return;
  srcModel_->addSimpleItem(itemText,descrName,currentParent, val);
}


void TForecast::addCalcItem(const QString& itemText,const QString& descrName, const QString& funcName/*,const QString& itemDescr, int alvl*/){
  Q_UNUSED(itemText);
  if(nullptr == srcModel_) return;
  srcModel_->addCalcItem(itemText,descrName,funcName,/*,itemDescr,*/currentParent/*,alvl*/);
}

void TForecast::addResultItem(const QString& itemText,const QString& itemDescr){
  Q_UNUSED(itemText);
  Q_UNUSED(itemDescr);
}

void TForecast::addResultFunction(const  QScriptValue& list )
{
  if(nullptr == resModel_) return;
  QStringList vs = list.toVariant().toString().split("//");
  if(vs.length() < 2){
    debug_log << "bad arguments"<<vs;
    return;
  }

  QScriptValue global = engine_->globalObject();

  QScriptValue func = global.property(vs.at(1));

  TFItem* it = resModel_->addFuncItem(func, list,"result");
  if(nullptr!= it){
    it->setRole(meteo::forecast::ForecastMethodItemRole::kResultData);
  }
}

void TForecast::addComboItem(const QString& text, const QString& descr, const QString& items)
{
  if(nullptr == srcModel_) return;
  srcModel_->addComboItem(text, descr, items.split(";"), currentParent);
}

void TForecast::addComboFunction(const  QScriptValue &list )
{
  if(nullptr == srcModel_) return;
  QStringList vs = list.toVariant().toString().split("//");
  if(vs.length() < 3){
    debug_log << "bad arguments"<<vs;
    return;
  }
  QScriptValue global = engine_->globalObject();
  QString func_name = vs.at(1).section('_',0,0);
  QScriptValue func = global.property(func_name);
  srcModel_->addComboFuncItem(func, list,currentParent);

}


void TForecast::addFunction(const  QScriptValue &list )
{
  Q_UNUSED(list);
  if(nullptr == srcModel_) return;
  QStringList vs = list.toVariant().toString().split("//");
  if(vs.length() < 3){
    debug_log << "bad arguments"<<vs;
    return;
  }
  QScriptValue global = engine_->globalObject();
  QString func_name = vs.at(1).section('_',0,0);
  QScriptValue func = global.property(func_name);
  srcModel_->addFuncItem(func, list,currentParent);

}


void TForecast::addSeparator(const QString& separatorName)
{
  Q_UNUSED(separatorName);
  if(nullptr == srcModel_) return;
  // srcModel_->addSeparator(currentParent,separatorName);
}

void TForecast::setFieldList(const QString &itemText, const QString & aparam,const QString &asrok,const QString &alevel,const QString &atypelevel,
                             const QString &atime,const QString& aName)
{
  if(nullptr == srcModel_) return;
  currentParent = srcModel_->rootItem()->getDescr();
  srcModel_->addFieldItem(currentParent, itemText,aName, aparam, asrok, alevel,atypelevel,atime);
  currentParent = aName;

}

meteo::GeoPoint TForecast::getCurrentCoord() const {
  if(nullptr == srcModel_) return meteo::GeoPoint() ;
  TFItem* cit = srcModel_->getItem(currentParent);
  if(nullptr == cit) return meteo::GeoPoint() ;
  return cit->curCoord();
}

void TForecast::addFieldValuetem( const QString &itemText, const QString & aparam,
                                  const QString &asrok,const QString &alevel,const QString &atypelevel,const QString& aName)
{
  if(nullptr == srcModel_) return;
  srcModel_->addFieldValueItem(currentParent, itemText, aparam, asrok, alevel,atypelevel,aName);
}

void TForecast::addFieldAdditem( const QString &itemText, const QString & aparam,const QString & addparam,
                                 const QString &asrok,const QString &alevel,const QString &atypelevel,const QString& aName)
{
  if(nullptr == srcModel_) return;
  srcModel_->addFieldAdd(currentParent,addparam, itemText, aName, aparam, asrok, alevel,atypelevel,"0");
}

meteo::GeoPoint TForecast::toGeoPoint(const QVariant & v)const
{
  return v.value<meteo::GeoPoint>();
}


