

function TForecastHeader(haveUi, nameForm){
  this.haveUi = haveUi;
  this.date = obj.getDate();
  this.IndexNumber = obj.getStationIndex();
  this.st =  obj.getStationData();
  if(this.haveUi) {
    this.initUi(nameForm);
  }
}

function getFromZondPoP(dv, level,typelevel,  srok){
  var ff;
  ff = zondObj.getDataFromP(obj.getStationData(), level, srok,dv);
  if(!isValidNum( ff ) )
  {
    return badvalue();
  }
  return ff;
}

function getFromZondPoH(dv, level,typelevel, srok){
  var ff;
  ff = zondObj.getDataFromHeight(obj.getStationData(), level, srok,dv);
  if(!isValidNum( ff ) )
  {
    return badvalue();
  }
  return ff;
}


//function icon()
//{
//  return ":/meteo/icons/geopixmap/veter.png";
//}

// ZIMA =0,
// VESNA =1,
// LETO =2,
// OSEN=3
// -1 Любая ситуация
function season()
{
  return "-1";
}
//  0:  "Центральная часть циклона или ось ложбины";
//  1:  "Передняя часть циклона или передняя часть ложбины";
//  2:  "Теплый сектор циклона";
//  3:  "Тыловая часть циклона или тыловая часть ложбины";
//  4:  "Центральная часть антициклона или ось барического гребня";
//  5:  "Северная периферия антициклона или северная часть барического гребня";
//  6:  "Западная периферия антициклона или тыловая часть барического гребня";
//  7:  "Восточная периферия антициклона или передняя часть барического гребня";
//  8:  "Южная периферия антициклона или отрог(ядро)";
//  9:  "Малоградиентная область пониженного (повышенного) давления";
//  10 "Седловина или перемычка пониженного (повышенного) давления";
// -1 Любая ситуация
function synsit()
{
  return "-1";
}

//  0:  "Гроза";
//  1:  "Осадки";
//  2:  "Туман";
//  3:  "Облачность";
//  4:  "Температура";
//  5:  "Гололёд";
//  6:  "Ветер";
//  7:  "Влажность";
//  8:  "Дальность видимости";
//  9:  "Обледенение";
// 10:  "Турбулентность";

function typeYavl()
{
  return "-1";
}

function methodHelpFile()
{
  return "bez_spravki.htm";
}

function badvalue()
{
  return -9999;
}

function isValidNum( T )
{
  return ( undefined != T && T != badvalue()&& isFinite(T) && !isNaN(T) )

}

function ftoi_norm(d)
{
  if( !isValidNum( d ) )
  {
    return badvalue();
  }
  d = parseFloat(d);
  if(d<0) return parseInt(d-0.5);
  else return parseInt(d+0.5);
}

TForecastHeader.prototype.addResult = function(itemText, parentItem, itemIndex)
{
  obj.addResultItem(itemText,parentItem, itemIndex);
}


TForecastHeader.prototype.addResultFunction = function() {
    var result = "";
   // var result_descr = arguments[0];
    for (var i = 0; i < arguments.length; ++i)
        result += String(arguments[i])+"//";
    obj.addResultFunction(result);
}

TForecastHeader.prototype.addFunction = function() {

  var result = "";
  for (var i = 0; i < arguments.length; ++i)
    result += String(arguments[i])+"//";
  obj.addFunction(result);
}

TForecastHeader.prototype.addComboFunction = function() {

  var result = "";
  for (var i = 0; i < arguments.length; ++i)
    result += String(arguments[i])+"//";
  obj.addComboFunction(result);
}

TForecastHeader.prototype.addFieldValue = function()
{
  obj.addFieldValuetem();
}

TForecastHeader.prototype.setCoords = function()
{
  obj.addCoordsItem();
}


TForecastHeader.prototype.setLevel = function(level)
{
  obj.setLevel(level);
}


TForecastHeader.prototype.addTime = function()
{
  var result = "";
  for (var i = 0; i < arguments.length; ++i)
    result += String(arguments[i])+"//";
  obj.addDTItem(result);
}

TForecastHeader.prototype.addCombo = function(text, desc, items)
{


   // var result = "";
   // for (var i = 0; i < arguments.length; ++i)
   // result += String(arguments[i])+"//";
    //obj.addComboItem(result);
    obj.addComboItem(text, desc, items);
}


// type_level для add
// Gr_Station       = 1, //!< на уровне станции"
// Gr_Tropo          = 2, //!< на уровне тропопаузы"
// Gr_MaxWind    = 3, //!< на уровне максимального ветра"
// Gr_Tempr         = 4, //!< особые точки  по температуре
// Gr_Wind           = 5, //!< особые точки  по ветру
// Gr_Standart     = 6, //!< на стандартных изобарических поверхностях"
// Gr_Lidar     = 7, //!<
// Gr_MaxWind_D    = 13, //!< на уровне максимального ветра выше 100
// Gr_Tempr_D         = 14, //!< особые точки  по температуре выше 100
// Gr_Wind_D           = 15, //!< особые точки  по ветру выше 100
// Gr_Standart_C     = 16, //!< на стандартных изобарических поверхностях выше 100
// Gr_PrizInvHi    = 17, //!< на стандартных изобарических поверхностях выше 100
// Gr_PrizInvLow    = 18, //!< на стандартных изобарических поверхностях выше 100
// LastGroup        = 19,//!< для цикла по группам в упорядочении по абсолютным значениям
//
// Gr_5001000      = 20 //!< относительные значения P,T,V для карт относительной топографии ОТ 500/1000

TForecastHeader.prototype.add = function(itemText, itemIndex, itemlevel,type_level,srok,aname)
{
 // if(!isValidNum(srok)) srok = 0;

  obj.addDataItem(itemText, itemIndex,itemlevel,type_level,srok,aname);
}

TForecastHeader.prototype.addFForecast = function(itemText, itemIndex, itemlevel,type_level, srok)
{

  obj.addForecastDataItem(itemText, itemIndex,itemlevel,type_level, srok);
}

TForecastHeader.prototype.parent = function(itemText, itemIndex)
{
  obj.setParrent(itemText,itemIndex);
}

// для addField и addFieldValue
// switch(type) {
//  Gr_Station:
// ad_= 1;
// break;
//  Gr_Tropo:
// ad_= 7;
// break;
//  Gr_MaxWind_D:  //TODO
//  Gr_MaxWind:
// ad_= 6;
// break;
//  Gr_Tempr_D:   //TODO одинаковые дескрипторы для ветра
//  Gr_Wind_D:   //TODO одинаковые дескрипторы для ветра
//  Gr_Tempr:
//  Gr_Wind:
// ad_= 101;
// break;
//  Gr_Lidar:
// ad_= 105;
// break;
//  Gr_Standart:
//  Gr_Standart_C:
// default:
// ad_= 100; break;//TODO одинаковые дескрипторы для ветра*/
//

TForecastHeader.prototype.addField = function(itemText, aparam, asrok, alevel, type_level, atime, aname)
{
  obj.setFieldList(itemText,aparam,asrok,alevel,type_level,atime,aname);
  //asrok - срок, за который берутся данные полей. не путать с atime
  //atime - параметр адвекции, на сколько часов назад возвращаемся "назад" по потоку.
}

TForecastHeader.prototype.addFieldValue = function(itemText, aparam, asrok, alevel, type_level, aname)
{
  obj.addFieldValuetem(itemText,aparam,asrok,alevel,type_level, aname);
}

TForecastHeader.prototype.addFieldAddValue = function(itemText, aparam,typeAdd, asrok, alevel, type_level, aname)
{
  obj.addFieldAdditem(itemText,aparam,typeAdd,asrok,alevel,type_level, aname);
}

TForecastHeader.prototype.addDepMethodResult = function()
{
  var result = "";
  for (var i = 0; i < arguments.length; ++i)
    result += String(arguments[i])+"//";
  obj.addDepMethodResult(result);
}

/*
TForecastHeader.prototype.addCalc = function(itemText, itemDescr, funcName, itemIndex)
{
  obj.addCalcItem(itemText, itemDescr, funcName, itemIndex);
}*/

TForecastHeader.prototype.addSeparator = function(separatorName, parentName)
{
  obj.addSeparator(parentName,separatorName);
}

TForecastHeader.prototype.addSimple = function(itemName, descr, value)
{
  obj.addSimpleItem(itemName, descr, value);
}



TForecastHeader.prototype.initUi = function(nameForm) {
 /* this.ui  = obj.loadUi("forecast_header.ui");
  this.ui.setWindowTitle(nameForm);
  this.reloadStation();
  this.ui.findChild("srok_date_time").setDateTime(this.date);
  this.ui.findChild("cb_forecast_srok")['currentIndexChanged(QString)'].connect(this,this.prognozSrokChanged);
  this.ui.findChild("srok_date_time").dateTimeChanged .connect( this,this.dateChanged);
  this.ui.findChild("le_index").textChanged.connect(this, this.IndexChanged);
  this.ui.findChild("le_punkt").textChanged.connect(this, this.punktChanged);*/
 // this.ui.show();

  obj.loadUi("forecast_simple.ui");
 //this.ui.setWindowTitle(nameForm);
 // this.ui.findChild("srok_date_time").setDateTime(this.date);
 // this.ui.findChild("srok_date_time").dateTimeChanged .connect( this,this.dateChanged);

}

//*********************************СЛОТЫ*********************8**********


TForecastHeader.prototype.prognozSrokChanged = function(str) {
  this.srok = this.ui.cb_forecast_srok.currentText;
}

TForecastHeader.prototype.dateChanged = function(adate) {
   this.date = adate; //.setFullYear(adate.getUTCFullYear, adate.;
   obj.setDate(adate);
}

 TForecastHeader.prototype.punktChanged = function(str) {
   this.punktName = str;
   this.reloadStation();
}

TForecastHeader.prototype.IndexChanged = function(str) {
 //  this.IndexNumber = str;
   //this.st = obj.setStationIndex(str);
  // this.reloadStation();
//   this.ui.findChild("le_punkt").setText( this.st.name);
//   this.ui.findChild("le_shir").setText( this.st.fi);
//   this.ui.findChild("le_dolg").setText( this.st.la);
}

TForecastHeader.prototype.reloadStation = function() {
    this.ui.findChild("le_index").setText( this.IndexNumber);
    this.ui.findChild("le_punkt").setText( this.st.name);
    this.ui.findChild("le_shir").setText( this.st.fi);
    this.ui.findChild("le_dolg").setText( this.st.la);
}



function getSynopFrSit(){
  var d=["Теплый фронт","Холодный фронт","Внутримассовый процесс"];
  var synsit = obj.getFrontType();
  //print("synsit",synsit);
  switch (synsit)
  {
    case 1:
      d=["Теплый фронт","Холодный фронт","Внутримассовый процесс"];
      break;
    case 2:
      d=["Холодный фронт","Теплый фронт","Внутримассовый процесс"];
      break;
    case 0:
    default:
      d=["Внутримассовый процесс","Теплый фронт","Холодный фронт"];
      break;
  }
  return d;
}
// ZIMA =0,
// VESNA =1,
// LETO =2,
// OSEN=3
function periodGoda(){
  var d=["Теплый","Холодный"];
  var season = obj.getSeason();
  //print("synsit",synsit);
  switch (season)
  {
    case 0:
    case 3:
      d=["Холодный","Теплый"];
      break;
    case 1:
    case 2:
    default:
      d=["Теплый","Холодный"];
      break;
  }
  return d;
}

function getPrecipFaza(){
  var d=["Без осадков","Дождь","Снег"];
  var javl =  zondObj.getValue(obj.getStationData(),0, 1, "w",0);

  // print(javl);
  if((50 <= javl  && javl <=69)
    ||(80 <= javl  && javl <=86)){
    d=["Дождь","Снег"];
    }
    if((70 <= javl  && javl <=79 )
      ||(87 <= javl  && javl <=97)){
      d=["Снег","Дождь"];
      }

      return d;
}


function getPrecipNal(){
  var d=["Осадки","Без осадков"];
  var javl =  zondObj.getValue(obj.getStationData(),0, 1, "w",0);

  // print(javl);
  if(50 <= javl  && javl <=99){
    d=["Осадки","Без осадков"];
  } else {
    d=["Без осадков","Осадки"];
  }

  return d;
}

function getYesNo(){
  var d=["Да","Нет"];

  return d;
}

function events(){
  var d=["Без явлений погоды","Дымка","Дождь","Снег","Морось"];

  var javl =  zondObj.getValue(obj.getStationData(),0, 1, "w",0);

  if(50 <= javl  && javl <=59){
    d=["Морось","Без явлений погоды","Дымка","Дождь","Снег"];
  }else
  if((60 <= javl  && javl <=69)
    ||(80 <= javl  && javl <=82)){
    d=["Дождь","Морось","Без явлений погоды","Дымка","Снег"];
  }else
  if((70 <= javl  && javl <=79 )
      ||(83 <= javl  && javl <=97)){
    d=["Снег","Дождь","Морось","Без явлений погоды","Дымка"];
  }else
  if(10 == javl ){
    d=["Дымка","Снег","Дождь","Морось","Без явлений погоды"];
  }

  return d;
}

function pic80 ( x0,  y0)//Ta-T,Da-D
{
   var k=[1,0,0.375];
   var b=[0,5,-1.875];
   var x1=3.5;
   var x2=5;
   var y, y1, y2, y4;
//Проверка на попадание в область допустимых значений
    if ((x0<=-10.) ) { return 1;}
    if ((x0>=10.) ) { return 3;}
    if ((y0<=-10) )  { return 2;}
    if ((y0>=10) )  { return 1;}
//Проверка на попадание в области 1,2,3
   if (x0<x1)
   {
      y=parseFloat(k[0]*x0)+parseFloat(b[0]);
      if (y0<=y)
      {return 2;}//"Ситуация не определена"
      else {return 1;}//"Облачность не образуется (размывается)"
   }

   if ((x0>=x1) && (x0<x2))
   {
      y4=parseFloat(k[2]*x0)+parseFloat(b[2]);
      y1=parseFloat(k[0]*x0)+parseFloat(b[0]);
      if (y0<=y4)
      {return 3;}//"Облачность  образуется (не размывается)";

   if ((y0<=y1) && (y0>y4))
      {return 2;}//"Ситуация не определена"

   if (y0>y1)
      {return 1;}//"Облачность не образуется (размывается)"
   }

   if (x0>=x2)
   {
     y4=parseFloat(k[2]*x0)+parseFloat(b[2]);
     y2=b[1];
     if (y0<=y4)
     {return 3;}//"Облачность  образуется (не размывается)";
     if ((y0<=y2) && (y0>y4))
     {return 2;}//"Ситуация не определена"

     if (y0>y2)
     {return 1;}//"Облачность не образуется (размывается)"
   }
  return 0;
}

/**
 * @brief Давление насыщениия (эмпирическая формула)
 * @return давление насыщениия, гПа
 */
function EP(t)
{
  return   Math.exp ((10.79574*(1.0-273.16/(parseFloat(t)+273.15))-
         5.02800*Math.log((parseFloat(t)+273.15)/273.16)*0.4343+
         0.000150475*(1.0-((parseFloat(t)+273.15)/273.16-1.0))+
         0.0000001745*(58823.38332*(1.0-273.16/(parseFloat(t)+273.15))-1.0)+
         0.78614)*Math.log(10.0));
}

function oprSPoTP(p, t)
{
  var res;
  if(t<-60.||t>50.) return "err1";
  if(p<100.||p>1100.) return "err2";
  var a;
  a = EP(t);
  if (a == badvalue())
  {
     return "err3";
  }
  res=0.622*a/p;
  return res;
}
