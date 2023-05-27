function name(){
  return "Poryv"
}

function title(){
  return "Порывы ветра";
}

function icon()
{
  return ":/meteo/icons/geopixmap/veter.png";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "8;12";
}

function methodHelpFile()
{
  return "poryv.htm";
}

function typeYavl()
{
  return "6";
}

function init(haveUi){
    new TForecastPoryv(haveUi);
}

function TForecastPoryv(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastPoryv.prototype.initSrc = function(h) {
  h.addFunction("Максимальная скорость ветра от земли до высоты 2 км, м/с","maxspeed");
  h.addFunction("Высота уровня максимальной скорости ветра, м","hmaxspeed");
  h.addFunction("Прогноз уровня максимального ветра (на 12ч), м","urovMaxWind",12);
  h.addFieldValue("Прогноз температуры воздуха на уровне земли (на 12ч),°C","T",12,0,1,"T12");
  h.addFunction("Прогноз температуры воздуха на уровне максимального ветра (на 12ч),°C","tempUrov","Poryv.urovMaxWind");
  h.addFunction("Прогностический вертикальный градиент температуры в слое от земли до уровня максимального ветра","gradientT","Poryv.T12","Poryv.tempUrov","Poryv.urovMaxWind");
  h.addFunction("Толщина слоя, в пределах которого скорость ветра превышает 15 м/с","dh","Poryv.maxspeed");


  h.addResultFunction("Cкорость максимальных порывов ветра, м/с","getA","Poryv.maxspeed","Poryv.dh","Poryv.gradientT");  // Значение индекса К

}
