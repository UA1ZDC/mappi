function name(){
  return "MDVprecip"
}

function icon()
{
  return ":/meteo/icons/geopixmap/eye.png";
}
function title(){
  return "Метеорологическая дальность видимости в осадках";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "6;18";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;1";
}

function methodHelpFile()
{
  return "MdvOsad.htm";
}

function typeYavl()
{
  return "8";
}

function init(haveUi){
    new TForecastMDVprecip(haveUi);
}

function TForecastMDVprecip(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();
  
}

TForecastMDVprecip.prototype.initSrc = function(h) {
   h.parent("В пункте прогноза","input");
   
   h.addComboFunction("Фаза осадков","getPrecipFaza");
   h.add("Интенсивность осадков, мм/ч","R",0,1);
   h.add("Скорость ветра на время начала осадков","ff",0,1);

   h.addResultFunction("МДВ, км","MDV","MDVprecip.input.R","MDVprecip.input.ff","MDVprecip.input.getPrecipFaza");

}
