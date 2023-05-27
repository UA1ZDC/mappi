function name(){
  return "Reshetov12"
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){
  return "Шквалы (метод Решетова 1)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Reshetov.htm";
}

function typeYavl()
{
  return "6";
}

function init(haveUi){
    new TForecastReshetov12(haveUi);
}

TForecastHeader.prototype.addResult = function(itemText, parentItem, itemIndex)
{
  obj.addResultItem(itemText,parentItem, itemIndex);
}

function TForecastReshetov12(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}
TForecastReshetov12.prototype.initSrc = function(h) {

    h.add("Приземное давление в пункте прогноза","P",0,1);
    h.add("Скорость ветра в пункте прогноза","ff",0,1);
    h.addFunction("Температура смоченного термометра,°C","Tsmoch");

    h.addField("Значения на уровне 850 гПа в районе адвекции (по полю 700 гПа)","hh",0,700,100,-12,"adv850");
    h.addFieldValue("Температура на уровне 850 гПа,°C","T",0,850,100,"T850");
    h.addFieldValue("Скорость ветра на уровне 850 гПа,°C","ff",0,850,100,"ff850");
    h.addFunction("Температура на кривой состояния","TsostOnLevel",850);

    h.addField("Значения на уровне 700 гПа в районе адвекции (по полю 700 гПа)","hh",0,700,100,-12,"adv700");
    h.addFieldValue("Температура на уровне 700 гПа,°C","T",0,700,100,"T700");
    h.addFieldValue("Скорость ветра на уровне 700 гПа","ff",0,700,100,"ff700");
    h.addFunction("Температура на кривой состояния,°C","TsostOnLevel",700);

    h.addField("Значения на уровне 600 гПа в районе адвекции (по полю 700 гПа)","hh",0,700,100,-12,"adv600");
    h.addFieldValue("Температура на уровне 600 гПа,°C","T",0,600,100,"T600");
    h.addFunction("Температура на кривой состояния,°C","TsostOnLevel",600);

    h.addField("Значения на уровне 500 гПа в районе адвекции (по полю 700 гПа)","hh",0,700,100,-12,"adv500");
    h.addFieldValue("Температура на уровне 500 гПа,°C","T",0,500,100,"T500");
    h.addFieldValue("Скорость ветра на уровне 500 гПа ","ff",0,500,100,"ff500");
    h.addFunction("Температура на кривой состояния,°C","TsostOnLevel",500);

    h.addResultFunction("Прогноз шквала","Reshetov","Reshetov12.adv850.T850",
                        "Reshetov12.adv700.T700","Reshetov12.adv600.T600","Reshetov12.adv500.T500",
                        "Reshetov12.P","Reshetov12.adv850.TsostOnLevel",
                        "Reshetov12.adv700.TsostOnLevel","Reshetov12.adv600.TsostOnLevel",
                        "Reshetov12.adv500.TsostOnLevel",
                        "Reshetov12.ff","Reshetov12.adv850.ff850",
                        "Reshetov12.adv700.ff700","Reshetov12.adv500.ff500","Reshetov12.Tsmoch");

    h.addResultFunction("Максимальная скорость ветра при шквале, м/с","windSpeedMax");

}
