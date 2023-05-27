function name(){
  return "Volna"
}

function icon()
{
  return ":/meteo/icons/geopixmap/volni.png";
}

function title(){
  return "Высоты ветровых волн";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Volna.htm";
}

function typeYavl()
{
  return "6";
}


function init(haveUi){
    new TForecastVolna(haveUi);
}

function TForecastVolna(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastVolna.prototype.initSrc = function(h) {

    h.add("Cкорость ветра в пункте прогноза","ff",0,1,"factFF");
    h.addFieldValue("Прогноз скорости ветра в пункте прогноза на 12 ч","ff",12,0,1,"progFF");

    h.add("Направление ветра в пункте прогноза","dd",0,1,"factDD");
    h.addFieldValue("Прогноз направления ветра в пункте прогноза на 12 ч","dd",12,0,1,"progDD");

    h.add("Высота волны в исходный срок, м","Hwa",0,1);

//    prognVolnaAll(Hwa,ff_t0,dd_t0,progFF,progDD)
    h.addResultFunction("Высота ветровых волн, м","prognVolnaAll","Volna.Hwa","Volna.factFF","Volna.factDD","Volna.progFF","Volna.progDD");

}
