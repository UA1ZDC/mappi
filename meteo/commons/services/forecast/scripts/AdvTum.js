function name(){
  return "AdvTum";
}

function icon()
{
  return ":/meteo/icons/geopixmap/tuman.png";
}

function methodHelpFile()
{
  return "AdvTumW.htm";
}

function title(){
  return "Адвективный туман и минимальная видимость (метод Н.В.Петренко)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;3;6;9;12;15;18;21;24";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "6;9";
}

function typeYavl()
{
  return "2";
}

function init(haveUi){
    new TForecastAdvTum(haveUi);
}

function TForecastAdvTum(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());
  this.initSrc(h);
  obj.reset();

}

TForecastAdvTum.prototype.initSrc = function(h) {
  var hour = dataObj.getHourData();

  h.parent("В пункте прогноза","insit");
    h.add("Температура,°C","T",0,1);
    h.add("Дефицит точки росы,°C","D",0,1);
    h.add("Температура точки росы,°C","Td",0,1);
    h.addFieldValue("Прогноз скорости ветра на 9 ч","ff",9,0,1,"progFF");

  h.addField("Точка адвекции 850 гПа (9 ч.)","hh",0,850,100,-9,"850adv9");
    h.add("Температура,°C","T",850,100);
    h.add("Температура точки росы,°C","Td",850,100);
   // h.addFieldValue("Температура,°C","T",0,850,100,"T");
   // h.addFieldValue("Температура точки росы,°C","Td",0,850,100,"Td");
    h.addFunction("Длинна траектории, км","traek");

  h.addResultFunction("Прогноз тумана на 9 ч","getTumNal","AdvTum.850adv9.T",
                      "AdvTum.850adv9.Td",
                      "AdvTum.insit.progFF","AdvTum.850adv9.traek","AdvTum.insit.T");
  h.addResultFunction("Прогноз видимости в тумане на 9 ч","getTumW","AdvTum.insit.T",
                      "AdvTum.insit.progFF","AdvTum.850adv9.T",
                      "AdvTum.insit.Td","AdvTum.insit.D","AdvTum.850adv9.Td","AdvTum.850adv9.traek");

}
