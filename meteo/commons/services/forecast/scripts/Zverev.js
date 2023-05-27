function name(){
  return "Zverev"
}

function icon()
{
  return ":/meteo/icons/geopixmap/tuman.png";
}

function title(){
  return "Радиационный туман (метод А.С. Зверева)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "12;18";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Zverev.htm";
}

function typeYavl()
{
  return "2";
}

function init(haveUi){
    new TForecastZverev(haveUi);
}

function TForecastZverev(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastZverev.prototype.initSrc = function(h) {


    h.addFunction("Срок прогноза на ночь","srok3");

    h.addCombo("Тип облачности на ночь(3 ч)","tip",
               "Верхняя тонкая;Верхняя плотная;Средняя тонкая;Средняя плотная;Нижняя");
    h.add("Прогноз облачности на ночь(3 ч), %","N",0,1,"Zverev.srok3");
    h.add("Прогноз ветра на ночь(3 ч)","ff" ,0,1, "Zverev.srok3");

    h.addTime("Время восхода Cолнца, UTC","sunrise");
    h.addTime("Время захода Cолнца, UTC","sunset");
    h.addFunction("Срок прогноза к заходу","sunsetsrok","Zverev.sunset");
    h.parent("В момент захода Солнца","insunset");
    h.add("Температура,°C","T",0,1,"Zverev.sunsetsrok" );
    h.add("Дефицит точки росы,°C","D", 0,1,"Zverev.sunsetsrok");
    h.add("Температура точки росы,°C","Td",0,1,"Zverev.sunsetsrok");
    h.addFunction("Относительная влажность","getHumiditySunset",
                  "Zverev.insunset.T","Zverev.insunset.D","Zverev.insunset.Td");
    h.parent("В исходный срок (18 ч)","inish19");
        h.addFunction("Срок прогноза на 18ч","srok18");
        h.add("Температура,°C","T" ,0,1, "Zverev.inish19.srok18");
        h.add("Дефицит точки росы,°C","D" ,0,1,"Zverev.inish19.srok18");
        h.add("Температура точки росы,°C","Td" ,0,1,"Zverev.inish19.srok18");
        h.addFunction("Относительная влажность, %","getHumiditySunset",
                      "Zverev.inish19.T","Zverev.inish19.D","Zverev.inish19.Td");
        h.addFunction("Ночное понижение температуры воздуха,°C","getdTn","Zverev.inish19.getHumiditySunset","Zverev.inish19.T",19);
    h.parent("В исходный срок (12 ч)","inish13");
        h.addFunction("Срок прогноза на 12ч","srok12");
        h.add("Температура,°C","T" ,0,1, "Zverev.inish13.srok12");
        h.add("Дефицит точки росы,°C","D" ,0,1,"Zverev.inish13.srok12");
        h.add("Температура точки росы,°C","Td" ,0,1,"Zverev.inish13.srok12");
        h.addFunction("Относительная влажность, %","getHumiditySunset",
                      "Zverev.inish13.T","Zverev.inish13.D","Zverev.inish13.Td");
        h.addFunction("Ночное понижение температуры воздуха,°C","getdTn",
                      "Zverev.inish13.getHumiditySunset","Zverev.inish13.T",13);
    h.addResultFunction("Прогноз минимальной температуры,°C","T_minZverev",
                        "Zverev.inish19.getdTn","Zverev.inish13.getdTn",
                        "Zverev.ff","Zverev.insunset.T",
                        "Zverev.N","Zverev.tip");  // Значение индекса К

    h.addResultFunction("Прогноз наличия тумана","getPrognozZverev",
                        "T_minZverev","Zverev.insunset.getHumiditySunset","Zverev.insunset.T");

    h.addResultFunction("Прогноз времени образования тумана","getTime_tum",
                        "Zverev.insunset.getHumiditySunset","Zverev.insunset.T","T_minZverev",
                        "Zverev.sunrise","Zverev.sunset");
    h.addResultFunction("Прогноз видимости в тумане","getVidimost_tum",
                        "Zverev.insunset.T","Zverev.insunset.D","Zverev.insunset.Td","Zverev.insunset.getHumiditySunset");

}
