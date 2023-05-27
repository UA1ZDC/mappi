function name(){
  return "Metel"
}

function title(){
  return "Метель";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function icon()
{
  return ":/meteo/icons/geopixmap/osadki.png";
}

function methodHelpFile()
{
  return "Metel.htm";
}

function season()
{
  return "0;1;3";
}

function typeYavl()
{
  return "1";
}

function init(haveUi){
    new TForecastMetel(haveUi);
}

function TForecastMetel(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastMetel.prototype.initSrc = function(h) {
    h.addCombo("Состояние снежного покрова","state","Рыхлый снег;Плотный снег");
    h.parent("Прогноз в пункте на 12 ч","pr");
        h.add("Температура,°C","T",0,1,12);
        h.add("Скорость ветра, м/с","ff",0,1,12);
        h.add("Интенсивность осадков, мм","R24",0,1,12);

  h.addResultFunction("Результат прогноза","metelForc","Metel.state","Metel.pr.R24","Metel.pr.ff","Metel.pr.T");
}
