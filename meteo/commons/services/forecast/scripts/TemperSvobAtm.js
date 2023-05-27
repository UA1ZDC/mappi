function name(){
  return "TemperSvobAtm"
}

function icon()
{
  return ":/meteo/icons/geopixmap/temperatura.png";
}

function title(){
  return "Температура в свободной атмосфере";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Temperatyra.htm";
}

function typeYavl()
{
  return "4";
}


function init(haveUi){
    new TForecastTemperSvobAtm(haveUi);
}

function TForecastTemperSvobAtm(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastTemperSvobAtm.prototype.initSrc = function(h) {
    h.parent("В пункте прогноза","init");
       h.addComboFunction("Период года","periodGoda");
       h.add("Температура на уровне 925 гПа,°C","T",925,100);
    h.addField("Точка адвекции 925 гПа (12 ч)","hh",0,925,100,-12,"925adv12");
       h.add("Температура на уровне 925 гПа,°C","T",925,100);
    h.addField("Точка адвекции 925 гПа (24 ч)","hh",0,925,100,-24,"925adv24");
       h.add("Температура на уровне 925 гПа,°C","T",925,100);

    h.addField("Точка адвекции 850 гПа (12 ч)","hh",0,850,100,-12,"850adv12");
       h.add("Температура на уровне 850 гПа,°C","T",850,100);
    h.addField("Точка адвекции 850 гПа (24 ч)","hh",0,850,100,-24,"850adv24");
       h.add("Температура на уровне 850 гПа,°C","T",850,100);

    h.addField("Точка адвекции 700 гПа (12 ч)","hh",0,700,100,-12,"700adv12");
       h.add("Температура на уровне 700 гПа,°C","T",700,100);
    h.addField("Точка адвекции 700 гПа (24 ч)","hh",0,700,100,-24,"700adv24");
       h.add("Температура на уровне 700 гПа,°C","T",700,100);

    h.addField("Точка адвекции 500 гПа (12 ч)","hh",0,500,100,-12,"500adv12");
       h.add("Температура на уровне 700 гПа,°C","T",500,100);
    h.addField("Точка адвекции 500 гПа (24 ч)","hh",0,500,100,-24,"500adv24");
       h.add("Температура на уровне 500 гПа,°C","T",500,100);


    h.addResultFunction("Прогноз температуры на уровне 925 гПа (12 ч)",
                       "TempSvobAtmos12","TemperSvobAtm.init.periodGoda","TemperSvobAtm.925adv12.T",
                        "TemperSvobAtm.init.T",925);
    h.addResultFunction("Прогноз температуры на уровне 925 гПа (24 ч)",
                        "TempSvobAtmos24","TemperSvobAtm.init.periodGoda","TemperSvobAtm.925adv24.T",
                        "TemperSvobAtm.init.T",925);

    h.addResultFunction("Прогноз температуры на уровне 850 гПа (12 ч)",
                       "TempSvobAtmos12","TemperSvobAtm.init.periodGoda","TemperSvobAtm.850adv12.T",
                        "TemperSvobAtm.init.T",850);
    h.addResultFunction("Прогноз температуры на уровне 850 гПа (24 ч)",
                        "TempSvobAtmos24","TemperSvobAtm.init.periodGoda","TemperSvobAtm.850adv24.T",
                        "TemperSvobAtm.init.T",850);

    h.addResultFunction("Прогноз температуры на уровне 700 гПа (12 ч)",
                       "TempSvobAtmos12","TemperSvobAtm.init.periodGoda","TemperSvobAtm.700adv12.T",
                        "TemperSvobAtm.init.T",700);
    h.addResultFunction("Прогноз температуры на уровне 700 гПа (24 ч)",
                        "TempSvobAtmos24","TemperSvobAtm.init.periodGoda","TemperSvobAtm.700adv24.T",
                        "TemperSvobAtm.init.T",700);

    h.addResultFunction("Прогноз температуры на уровне 500 гПа (12 ч)",
                       "TempSvobAtmos12","500","TemperSvobAtm.500adv12.T",
                        "TemperSvobAtm.init.T",500);
    h.addResultFunction("Прогноз температуры на уровне 500 гПа (24 ч)",
                        "TempSvobAtmos24","500","TemperSvobAtm.500adv24.T",
                        "TemperSvobAtm.init.T",500);
}
