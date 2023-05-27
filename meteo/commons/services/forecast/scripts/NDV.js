function name(){
  return "NDV"
}

function icon()
{
  return ":/meteo/icons/geopixmap/eye.png";
}

function title(){
  return "Наклонная дальность видимости";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;1";
}

function init(haveUi){
    new TForecastNDV(haveUi);
}

function methodHelpFile()
{
  return "NDV.htm";
}

function typeYavl()
{
  return "8";
}

function TForecastNDV(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastNDV.prototype.initSrc = function(h) {

    h.addCombo("Тип авиации","avia","Истребительно-бомбардировочная;Дальняя;Военно-транспортная;Вертолеты");
    h.addComboFunction("Тип явления","events");

    // h.add("Количество облачности, %","N",0,1);
    h.add("Количество наблюдаемой облачности, %","N",0,1);
    h.add("ВНГО, м","h",0,1);
    h.add("МДВ у земли, м","V",0,1);

    h.addResultFunction("Наклонная дальность видимости, м","getYavlAll","NDV.h","NDV.V","NDV.N","NDV.avia","NDV.events");
    //Hp - vngo (в м); Sm - MDV_f (в км); t - typeYavl
    //Sp - NDV; Hs - высота облаков, которую определил бы летчик в полете – высота по «самолету»

}
