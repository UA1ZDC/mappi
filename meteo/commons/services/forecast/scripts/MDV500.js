function name(){
  return "MDV500"
}

function icon()
{
  return ":/meteo/icons/geopixmap/eye.png";
}


function title(){
  return "Оценивание вероятности МДВ менее 500 м при различных ЯП";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;1";
}

function methodHelpFile()
{
  return "Mdv500.htm";
}

function typeYavl()
{
  return "8";
}

function init(haveUi){
    new TForecastMDV500(haveUi);
}

function TForecastMDV500(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastMDV500.prototype.initSrc = function(h) {

  h.addComboFunction("Период года","periodGoda");
    h.addFieldAddValue("Градиент давления в пункте прогноза на уровне земли (на 100км)","P","gradx", 0,0,1,"grad");
    h.addResultFunction("Вероятность МДВ менее 500 м, %","verMDV500","MDV500.grad","MDV500.periodGoda");  // Значение вероятности
}
