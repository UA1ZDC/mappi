function name(){
  return "Gololedica"
}

function icon()
{
  return ":/meteo/icons/geopixmap/gololedica.png";
}

function title(){
  return "Гололедица";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Gololedica.htm";
}

function season()
{
  return "0;1;3";
}

function typeYavl()
{
  return "5";
}

function init(haveUi){
    new TForecastGololedica(haveUi);
}

function TForecastGololedica(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastGololedica.prototype.initSrc = function(h) {


  h.parent("Данные для прогноза на 12 часов","12hr");
    h.addFieldValue("Прогноз температуры на уровне земли,°C","T",12,0,1,"pr12T");
    h.addFieldValue("Прогноз дефицита точки росы,°C","D",12,0,1,"pr12Td");
    h.addFieldValue("Прогноз температуры почвы,°C","Ts",12,0,1,"pr12Ts");
  h.parent("Данные для прогноза на 18 часов","18hr");
    h.addFieldValue("Прогноз температуры на уровне земли,°C","T",18,0,1,"pr18T");
    h.addFieldValue("Прогноз дефицита на уровне земли,°C","D",18,0,1,"pr18Td");
    h.addFieldValue("Прогноз температуры почвы,°C","Ts",18,0,1,"pr18Ts");


    h.addResultFunction("Прогноз на 12 часов","GololedicaNal","Gololedica.12hr.pr12T","Gololedica.12hr.pr12Td","Gololedica.12hr.pr12Ts",12);
//    h.addResultFunction("Прогноз на 15 часов","GololedicaNal","Gololedica.pr15T","Gololedica.pr15Td","Gololedica.pr15Ts",);
    h.addResultFunction("Прогноз на 18 часов","GololedicaNal","Gololedica.18hr.pr18T","Gololedica.18hr.pr18Td","Gololedica.18hr.pr18Ts",18);

}
