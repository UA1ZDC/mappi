function name(){
  return "PinusPrizem"
}

function icon()
{
  return ":/meteo/icons/geopixmap/boltanka2.png";
}

function title(){
  return "Болтанка для различных типов самолетов на посадке (метод Пинуса Н.З.)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;18";  //прогноз составляется на время исходных данных
}

function methodHelpFile()
{
  return "PinusPrizemn.htm";
}

function typeYavl()
{
  return "10";
}

function init(haveUi){
    new TForecastPinusPrizem(haveUi);
}

function TForecastPinusPrizem(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();
  
}

TForecastPinusPrizem.prototype.initSrc = function(h) {
    h.addCombo("Тип воздушного судна и характер выполняемой задачи","tip",
    "Фронтовая авиация и ИЛ-76 горизонтальный полет;Фронтовая авиация и ИЛ-76 при заходе на посадку;Военно-транспортная и дальняя авиация горизонтальный полет;Военно-транспортная и дальняя авиация при заходе на посадку;Вертолеты");
//    h.add("Фактическая скорость приземного ветра в пункте прогноза","ff",0,100);
    h.addFieldValue("Прогноз приземного ветра в пункте (на 6ч)","ff",6,0,1,"ff_6");
    h.addFieldValue("Прогноз приземного ветра в пункте (на 12ч)","ff",12,0,1,"ff_12");
    h.addFieldValue("Прогноз приземного ветра в пункте (на 18ч)","ff",18,0,1,"ff_18");

   h.addResultFunction("Прогноз болтанки (на 6ч)","PinusPrizemn","PinusPrizem.tip","PinusPrizem.ff_6");
   h.addResultFunction("Прогноз болтанки (на 12ч)","PinusPrizemn","PinusPrizem.tip","PinusPrizem.ff_12");
   h.addResultFunction("Прогноз болтанки (на 18ч)","PinusPrizemn","PinusPrizem.tip","PinusPrizem.ff_18");

}
