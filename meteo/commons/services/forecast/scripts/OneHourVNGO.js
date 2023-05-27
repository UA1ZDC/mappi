function name(){
  return "OneHour"
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){   
  return "ВНГО (менее 300 м) через 1 ч (метод в/ч 75354)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;1";
}

function methodHelpFile()
{
  return "OneHour.htm";
}

function typeYavl()
{
  return "3";
}

function init(haveUi){
    new TForecastOneHour(haveUi);
}

function TForecastOneHour(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();
  
}

TForecastOneHour.prototype.initSrc = function(h) {
  h.addComboFunction("Период года","periodGoda");
  h.addComboFunction("Синоптическая ситуация","getSynopFrSit");
  h.addCombo("Тип наблюдаемой облачности","cloud_form","Ns;Sc;St");
  h.add("ВНГО в исходный срок ,м","h",0,1);
  h.addSimple("ВНГО за 15 мин до исходного срока","sim1");
  h.addSimple("ВНГО за 30 мин до исходного срока","sim2");
  h.addSimple("ВНГО за 45 мин до исходного срока","sim3");
  // h.addFieldValue("ВНГО за 60 мин до исходного срока","hh",-1,0,1,"sim4");
  h.addSimple("ВНГО за 60 мин до исходного срока","sim4");
  h.addSimple("ВНГО за 75 мин до исходного срока","sim5");
  h.addSimple("ВНГО за 90 мин до исходного срока","sim6");
    h.addResultFunction("Высота нижней границы облачности через 15 мин","oneHourVNG015","OneHour.cloud_form","OneHour.getSynopFrSit",
                        "OneHour.periodGoda","OneHour.h","OneHour.sim1","OneHour.sim2","OneHour.sim3",
                        "OneHour.sim4","OneHour.sim5","OneHour.sim6");
    h.addResultFunction("Высота нижней границы облачности через 30 мин","oneHourVNG030","OneHour.cloud_form","OneHour.getSynopFrSit",
                        "OneHour.periodGoda","OneHour.h","OneHour.sim1","OneHour.sim2","OneHour.sim3",
                        "OneHour.sim4","OneHour.sim5","OneHour.sim6");
    h.addResultFunction("Высота нижней границы облачности через 45 мин","oneHourVNG045","OneHour.cloud_form","OneHour.getSynopFrSit",
                        "OneHour.periodGoda","OneHour.h","OneHour.sim1","OneHour.sim2","OneHour.sim3",
                        "OneHour.sim4","OneHour.sim5","OneHour.sim6");
    h.addResultFunction("Высота нижней границы облачности через 60 мин","oneHourVNG060","OneHour.cloud_form","OneHour.getSynopFrSit",
                        "OneHour.periodGoda","OneHour.h","OneHour.sim1","OneHour.sim2","OneHour.sim3",
                        "OneHour.sim4","OneHour.sim5","OneHour.sim6");



}

