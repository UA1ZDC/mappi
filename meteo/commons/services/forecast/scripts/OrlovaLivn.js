function name(){
  return "OrlovaLivn"
}

function icon()
{
  return ":/meteo/icons/geopixmap/osadki.png";
}

function title(){
  return "Ливневые осадки (Орловой)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;21";
}

function methodHelpFile()
{
  return "OrlovaLivn.htm";
}

function season()
{
  return "1;2;3";
}

function typeYavl()
{
  return "1";
}

function init(haveUi){
    new TForecastOrlovaLivn(haveUi);
}

function TForecastOrlovaLivn(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}
TForecastOrlovaLivn.prototype.initSrc = function(h) {
    // h.parent("Расчеты","calc");


      //h.addFunction("Прогностическое значение максимальной температуры воздуха у земли в точке прогноза, Tmax, C","TmaxAdv");

      h.addFieldValue("Прогностическое значение температуры воздуха на уровне 850 гПа в точке прогноза через 12 ч","T",12,850,100,"T_12");

      h.addField("Поле адвекции на уровне 850 гПа","hh",0,850,100,-12,"progn");
        // h.addFieldValue("скорость","ff",0,700,100,"ff_adv");
        h.addFunction("Средняя скорость ветра вдоль 12-часовой траектории на уровне 700 гПа, м/с","ff700");
        h.addFunction("Средняя скорость ветра вдоль 12-часовой траектории на уровне 500 гПа, м/с","ff500");
        h.addFunction("Средняя длина траектории в слое 1000-850 гПа, км, за интервал времени 12 час","traekt1000850");

    // h.parent("Потенциально неустойчивый слой","calc");
      h.addFunction("Верхняя  граница Потенциально неустойчивого слоя, ГПа","Pv");
          h.addFunction("Нижняя граница ПНС, ГПа","Pn");
          h.addFunction("Температура на верхней границе ПНС,°C","Tpv");
          h.addFunction("Температура на нижней границе ПНС,°C","Tpn");
          h.addFunction("Температура на кривой состояния на верхней границе ПНС,°C","Tsostv","Pv");
          h.addFunction("Температура на кривой состояния на нижней границе ПНС,°C","Tsostn","Pn");
    // h.parent("Исходные данные","calc1");



    // h.parent("Расчетные данные","calc2");
        h.addFunction("Масса воздуха в КНС (m), г","calcM","Pv","Pn");
        h.addFunction("Средняя температура  на кривой стратификации в КНС,°C",
                      "calcT","Tpv","Tpn");
        h.addFunction("Средняя температура  на кривой состояния в КНС,°C",
                      "calcTsost","Tsostv","Tsostn");
        h.addFunction("Средняя разность между температурой на кривой состояния и на кривой стратификации в КНС,°C",
                      "calcDT","calcTsost","calcT");
        h.addFunction("Средняя скорость вертикальных токов конвекции в КНС, м/с",
                      "calcWk","calcDT","Pv","Pn");
        h.addFunction("Средняя скорость вертикальных движений в конвективном облаке, м/с",
                      "calcW","calcM","calcWk","Pv","Pn");
        h.addFunction("Прогностическое (на 12 ч) значение максимальной упругости водяного пара на уровне 850 гПа в точке прогноза, г/кг","calcQmax850","T_12");



        h.addResultFunction("Средняя скорость перемещения конвективных облаков, км/ч","calcV","OrlovaLivn.ff700","OrlovaLivn.ff500");
        h.addResultFunction("Продолжительность ливневых осадков","floatToHourHuman","OrlovaLivn.traekt1000850","calcV"); //"15007"
        h.addResultFunction("Интенсивность ливневых осадков, мм/ч","calcI","OrlovaLivn.calcQmax850","OrlovaLivn.calcW");
        h.addResultFunction("Количество осадков за ливень, мм","calcQl","calcI"); //"13011"

//        h.addFunction("Продолжительность ливневых осадков, ч","calcTl","OrlovaLivn.progn.traekt1000850","OrlovaLivn.calc2.calcV");//
//        h.addFunction("Количество ливневых осадков, мм","calcQl","calc2.calcI","calc2.calcTl");

       // h.addResultFunction("15007","Продолжительность ливневых осадков, ч","OrlovaLivnPeriod");
//        h.addResultFunction("Интенсивность осадков, мм/ч","OrlovaLivnIntens");//"13055"
//    h.parent("Прогноз","calc3");
//        h.addFunction("Количество осадков за ливень мм","OrlovaLivnKolvo");
//        h.addFunction("Интенсивность осадков, мм/ч","OrlovaLivnIntens");
//        h.addFunction("Продолжительность осадков, ч","OrlovaLivnPeriod");

//        h.addFunction("Ср. длина траектории в слое 1000-850","OrlovaLivnTraekt");
}
