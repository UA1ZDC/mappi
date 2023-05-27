function name(){
  return "Pinus400up"
}

function icon()
{
  return ":/meteo/icons/geopixmap/boltanka2.png";
}


function title(){
  return "Болтанка выше уровня 400 гПа (метод Пинуса Н.З.)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;6";
}

function methodHelpFile()
{
  return "Pinus400up.htm";
}

function typeYavl()
{
  return "10";
}

function init(haveUi){
    new TForecastPinus400up(haveUi);
}

function TForecastPinus400up(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastPinus400up.prototype.initSrc = function(h) {
//  h.addCombo("Срок прогноза","srok","Истребительно-бомбардировочная;Дальняя;Военно-транспортная;Вертолеты");

  h.addField("Точка адвекции 850 гПа (6 ч)","hh",0,850,100,-6,"adv");
  h.parent("Нижняя граница слоя","down");
  h.addSimple("Высота, м","lowerSloy",7000);
  h.addFunction("Скорость ветра, м/с","getDataFF","Pinus400up.down.lowerSloy","Pinus400up.adv");

  h.addFunction("Направление ветра","getDataDD","Pinus400up.down.lowerSloy","Pinus400up.adv");
  h.addFunction("Температура,°C","getDataT","Pinus400up.down.lowerSloy","Pinus400up.adv");
  h.parent("Верхняя граница слоя","up");
  h.addSimple("Высота, м","upSloy",9000);
  h.addFunction("Скорость ветра, м/с","getDataFF","Pinus400up.up.upSloy","Pinus400up.adv");
  h.addFunction("Направление ветра","getDataDD","Pinus400up.up.upSloy","Pinus400up.adv");
  h.addFunction("Температура,°C","getDataT","Pinus400up.up.upSloy","Pinus400up.adv");


  //Pinus400up(T_high,T_low,ff_high,ff_low,dd_high,dd_low,H_high,H_low)

  h.addResultFunction("Результат","Pinus400","Pinus400up.up.getDataT","Pinus400up.down.getDataT",
                      "Pinus400up.up.getDataFF","Pinus400up.down.getDataFF",
                      "Pinus400up.up.getDataDD","Pinus400up.down.getDataDD",
                      "Pinus400up.up.upSloy","Pinus400up.down.lowerSloy");

}
