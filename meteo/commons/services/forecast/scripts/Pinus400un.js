function name(){
  return "Pinus400un"
}

function icon()
{
  return ":/meteo/icons/geopixmap/boltanka2.png";
}

function title(){
  return "Болтанка в тропосфере (метод Пинуса Н.З.)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;21";
}

function methodHelpFile()
{
  return "Pinus400un.htm";
}

function typeYavl()
{
  return "10";
}

function init(haveUi){
    new TForecastPinus400un(haveUi);
}

function TForecastPinus400un(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}
TForecastPinus400un.prototype.initSrc = function(h) {

//    h.parent("Исходные данные","calc1");
        h.addFunction("Максимальная разность температур кривой состояния и стратификации,°C","Pinus400undTmax");
        h.addFunction("Давление на уровне максимальной разности, гПа","Pinus400unP");
//    h.parent("Прогноз болтанки в тропосфере","calc2");
//        h.addFunction("Результат","Pinus400unDescr","Pinus400un.calc1.Pinus400undTmax");
    h.addResultFunction("Результат","Pinus400unDescr","Pinus400un.Pinus400undTmax","Pinus400un.Pinus400unP");

}
