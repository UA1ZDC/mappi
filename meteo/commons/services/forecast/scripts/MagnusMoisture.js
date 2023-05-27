function name(){
  return "MagnusMoisture"
}

function icon()
{
  return ":/meteo/icons/geopixmap/vlagnost.png";
}

function title(){
  return "Относительная влажность (метод Магнуса)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Vlazhnost.htm";
}

function typeYavl()
{
  return "7";
}

function init(haveUi){
    new TForecastMagnusMoisture(haveUi);
}

function TForecastMagnusMoisture(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastMagnusMoisture.prototype.initSrc = function(h) {
  h.parent("В пункте прогноза","srf");
//  h.add("Давление","P",0);//добавляет значение по станции (ближайшей к текущим координатам станции )
  h.addFieldValue("Прогноз температуры на уровне земли (на 12ч),°C","T",12,0,1,"T1");
 // h.addFieldValue("Прогноз дефицита точки росы на уровне земли (на 12ч)","Td",12,0,1,"Td1");
  h.addFieldValue("Прогноз температуры точки росы на уровне земли (на 12ч),°C","Td",12,0,1,"Td1");

//  h.add("Температура, C","T",0);
//  h.add("Точка росы, C","Td",0);//добавляет значение по станции (ближайшей к текущим координатам станции )

  h.addResultFunction("Относительная влажность, %","getVlaznostMagnus","MagnusMoisture.srf.T1","MagnusMoisture.srf.Td1");

}
