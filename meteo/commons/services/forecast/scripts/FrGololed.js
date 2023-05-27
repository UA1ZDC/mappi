function name(){
  return "FrGololed"
}

function icon()
{
  return ":/meteo/icons/geopixmap/gololed.png";
}


function title(){
  return "Фронтальный гололед (метод Р. А. Ягудина)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Jagydin.htm";
}

function season()
{
  return "0;1;2;3";
}

function typeYavl()
{
  return "5";
}


function init(haveUi){
    new TForecastFrGololed(haveUi);
}

function TForecastFrGololed(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastFrGololed.prototype.initSrc = function(h) {
   h.parent("Данные для прогноза на 6 часов","6hr");
   h.addFieldValue("Прогноз температуры на уровне земли,°C","T",6,0,1,"prT");
   h.addFieldValue("Прогноз температуры на уровне 850 гПа,°C","T",6,850,100,"prT850");
   h.addFunction("Прогноз высоты изотермы -10°С на 6 ч, м","H10height",6);

   h.parent("Данные для прогноза на 12 часов","12hr");
   h.addFieldValue("Прогноз температуры на уровне земли,°C","T",12,0,1,"prT");
   h.addFieldValue("Прогноз температуры на уровне 850 гПа,°C","T",12,850,100,"prT850");
   h.addFunction("Прогноз высоты изотермы -10°С на 12 ч, м)","H10height",12);

   h.parent("Данные для прогноза на 18 часов","18hr");
   h.addFieldValue("Прогноз температуры на уровне земли,°C","T",18,0,1,"prT");
   h.addFieldValue("Прогноз температуры на уровне 850 гПа,°C","T",18,850,100,"prT850");
   h.addFunction("Прогноз высоты изотермы -10°С на 18 ч, м","H10height",18);

   h.addResultFunction("Прогноз интенсивности фронтального гололёда на 6 часов",
                       "GololedIntens","FrGololed.6hr.prT850","FrGololed.6hr.prT","FrGololed.6hr.H10height",6);
   h.addResultFunction("Прогноз интенсивности фронтального гололёда на 12 часов",
                       "GololedIntens","FrGololed.12hr.prT850","FrGololed.12hr.prT","FrGololed.12hr.H10height",12);
   h.addResultFunction("Прогноз интенсивности фронтального гололёда на 18 часов",
                       "GololedIntens","FrGololed.18hr.prT850","FrGololed.18hr.prT","FrGololed.18hr.H10height",18);
}
