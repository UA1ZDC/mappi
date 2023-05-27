
function name(){
  return "SurfaceWind"
}

function icon()
{
  return ":/meteo/icons/geopixmap/veterz.png";
}

function title(){
  return "Скорость и направление ветра у земли";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "veterZ_P.htm";
}

function typeYavl()
{
  return "6";
}


function init(haveUi){
    new TForecastSurfaceWind(haveUi);
}

function TForecastSurfaceWind(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastSurfaceWind.prototype.initSrc = function(h) {
    h.addCombo("Тип подстилающей поверхности","pp","Cуша;Море;Холмистая");

    h.addFieldAddValue("Прогноз на 12 ч лапласиана давления в пункте на уровне земли","P","lapl", 12,0,1,"lap0");
    h.addFieldAddValue("Прогноз на 12 ч градиента давления ОХ в пункте на уровне земли","P","gradx", 12,0,1,"grx");
    h.addFieldAddValue("Прогноз на 12 ч градиента давления OY в пункте на уровне земли","P","grady", 12,0,1,"gry");


     h.addResultFunction("Скорость ветра у земли, м/с","veter_skor_Z_P","SurfaceWind.lap0","SurfaceWind.grx","SurfaceWind.gry","SurfaceWind.pp");
     h.addResultFunction("Направление ветра у земли, в градусах","veter_nap_Z_P","SurfaceWind.lap0","SurfaceWind.grx","SurfaceWind.gry","SurfaceWind.pp");



}
