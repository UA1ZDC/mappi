function name(){
  return "Ilina"
}

function icon()
{
  return ":/meteo/icons/geopixmap/veter.png";
}

function title(){
  return "Изменение скорости ветра в струйном течении (метод Е.С.Ильиной)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Ilina.htm";
}

function typeYavl()
{
  return "6";
}

function init(haveUi){
    new TForecastIlina(haveUi);
}

function TForecastIlina(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastIlina.prototype.initSrc = function(h) {
  h.addFunction("Максимальная скорость ветра в пункте прогноза, м/с","veterData");
  h.addFunction("Давление на уровне максимального ветра, гПа","PveterData");
  h.addTime("Время адвекции","TveterDate","Ilina.veterData");

//  h.addFunction("Время адвекции, ч","TveterData","Ilina.veterData");
  h.addFunction("Ближайший уровень, гПа","NearLevel","Ilina.PveterData");
  h.addField("Поле адвекции на уровне максимального ветра","hh",0,"Ilina.NearLevel",100,"Ilina.TveterDate");
  h.addFieldValue("Скорость ветра, м/с", "ff", 0,300, 100, "veterDataAdv");

  h.addResultFunction("Cкорость струйного течения, км/ч","V_Progn_Ilina","Ilina.veterData","Ilina.veterDataAdv");
}
