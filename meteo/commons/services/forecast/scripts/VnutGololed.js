function name(){
  return "VnutGololed"
}

function icon()
{
  return ":/meteo/icons/geopixmap/gololed.png";
}

function title(){
  return "Внутримассовый гололед";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "6;18";
}

function methodHelpFile()
{
  return "Vnytrimassgololed.htm";
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
    new TForecastVnutGololed(haveUi);
}

function TForecastVnutGololed(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastVnutGololed.prototype.initSrc = function(h) {
    h.addField("Точка адвекции 850 гПа (6 ч)","hh",0,850,100,-6,"adv6");
    h.addFieldValue("Температура на уровне земли,°C","T",0,0,1,"T_6");
    h.addFieldValue("Температура на уровне 850 гПа,°C","T",0,850,100,"T_6_850");

    h.addField("Точка адвекции 850 гПа (9 ч)","hh",0,850,100,-9,"adv9");
    h.addFieldValue("Температура на уровне земли,°C","T",0,0,1,"T_9");
    h.addFieldValue("Температура на уровне 850 гПа,°C","T",0,850,100,"T_9_850");

    h.addField("Точка адвекции 850 гПа (12 ч)","hh",0,850,100,-12,"adv12");
    h.addFieldValue("Температура на уровне земли,°C","T",0,0,1,"T_12");
    h.addFieldValue("Температура на уровне 850 гПа,°C","T",0,850,100,"T_12_850");

    h.addResultFunction("Прогноз на 6 ч","MassGololedIntens","VnutGololed.adv6.T_6","VnutGololed.adv6.T_6_850",6);
    h.addResultFunction("Прогноз на 9 ч","MassGololedIntens","VnutGololed.adv9.T_9","VnutGololed.adv9.T_9_850",9);
    h.addResultFunction("Прогноз на 12 ч","MassGololedIntens","VnutGololed.adv12.T_12","VnutGololed.adv12.T_12_850",12);

//    MassGololedIntens(T,T850adv) //адвективная т у землии на уровне 850 гПа

}
