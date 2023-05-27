function name(){
  return "VlajSvobAtm";
}

function icon()
{
  return ":/meteo/icons/geopixmap/vlagnost.png";
}

function title(){
  return "Влажность в свободной атмосфере";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "VlagaH.htm";
}

function typeYavl()
{
  return "7";
}


function init(haveUi){
    new TForecastVlajSvobAtm(haveUi);
}

function TForecastVlajSvobAtm(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastVlajSvobAtm.prototype.initSrc = function(h) {

    h.parent("В пункте прогноза","insit");

    h.addFieldValue("Температура на уровне 925 гПа,°C","T",0,925,100,"T925");
    h.addFieldValue("Дефицит точки росы на уровне 925 гПа,°C","D",0,925,100,"DD925");

    h.addFieldValue("Температура на уровне 850 гПа,°C","T",0,850,100,"T850");
    h.addFieldValue("Дефицит точки росы на уровне 850 гПа,°C","D",0,850,100,"DD850");

    h.addFieldValue("Температура на уровне 700 гПа,°C","T",0,700,100,"T700");
    h.addFieldValue("Дефицит точки росы на уровне 700 гПа,°C","D",0,700,100,"DD700");

    h.addFieldValue("Температура на уровне 500 гПа,°C","T",0,500,100,"T500");
    h.addFieldValue("Дефицит точки росы на уровне 500 гПа,°C","D",0,500,100,"DD500");

    h.addFieldValue("Температура на уровне 400 гПа,°C","T",0,400,100,"T400");
    h.addFieldValue("Дефицит точки росы на уровне 400 гПа,°C","D",0,400,100,"DD400");

    h.addFieldValue("Температура на уровне 300 гПа,°C","T",0,300,100,"T300");
    h.addFieldValue("Дефицит точки росы на уровне 300 гПа,°C","D",0,300,100,"DD300");

    h.addFieldAddValue("Лапласиан на уровне земли","hh","lapl", 0,1000,100,"lap0");
    h.addFieldAddValue("Лапласиан на уровне 850 гПа","hh","lapl", 0,850,100,"lap850");
    h.addFieldAddValue("Лапласиан на уровне 700 гПа","hh","lapl", 0,700,100,"lap700");
    h.addFieldAddValue("Лапласиан на уровне 500 гПа","hh","lapl", 0,500,100,"lap500");
    h.addFieldAddValue("Лапласиан на уровне 400 гПа","hh","lapl", 0,400,100,"lap400");
    h.addFieldAddValue("Лапласиан на уровне 300 гПа","hh","lapl", 0,300,100,"lap300");


    h.addField("Точка адвекции 700 гПа (12 ч.)","hh",0,700,100,-12,"700adv12");
    h.addFieldAddValue("Лапласиан на уровне земли","hh","lapl", 0,1000,100,"lap0");
    h.addFieldAddValue("Лапласиан на уровне 850 гПа","hh","lapl", 0,850,100,"lap850");
    h.addFieldAddValue("Лапласиан на уровне 700 гПа","hh","lapl", 0,700,100,"lap700");
    h.addFieldAddValue("Лапласиан на уровне 500 гПа","hh","lapl", 0,500,100,"lap500");
    h.addFieldAddValue("Лапласиан на уровне 400 гПа","hh","lapl", 0,400,100,"lap400");
    h.addFieldAddValue("Лапласиан на уровне 300 гПа","hh","lapl", 0,300,100,"lap300");

//  h.parent("Вертикальные скорости","init");
    h.addFunction("Вертикальная скорость (гПа/12ч) на уровне 925 гПа","SkorVertDvig925","VlajSvobAtm.insit.lap0","VlajSvobAtm.700adv12.lap0");
    h.addFieldValue("Температура на уровне 925 гПа","T",0,925,100,"T925adv");
    h.addFieldValue("Дефицит точки росы на уровне 925 гПа","D",0,925,100,"DD925adv");

    h.addFunction("Вертикальная скорость (гПа/12ч) на уровне 850 гПа","SkorVertDvig850","VlajSvobAtm.insit.lap0","VlajSvobAtm.700adv12.lap0");
    h.addFieldValue("Температура на уровне 850 гПа","T",0,850,100,"T850adv");
    h.addFieldValue("Дефицит точки росы на уровне 850 гПа","D",0,850,100,"DD850adv");

    h.addFunction("Вертикальная скорость (гПа/12ч) на уровне 700 гПа","SkorVertDvig700",
                  "VlajSvobAtm.insit.lap700","VlajSvobAtm.700adv12.lap0",
                  "VlajSvobAtm.insit.lap850","VlajSvobAtm.700adv12.lap850","VlajSvobAtm.700adv12.SkorVertDvig850");
    h.addFieldValue("Температура на уровне 700 гПа","T",0,700,100,"T700adv");
    h.addFieldValue("Дефицит точки росы на уровне 700 гПа","D",0,700,100,"DD700adv");

    h.addFunction("Вертикальная скорость (гПа/12ч) на уровне 500 гПа","SkorVertDvig500",
                  "VlajSvobAtm.insit.lap500","VlajSvobAtm.700adv12.lap500",
                  "VlajSvobAtm.insit.lap700","VlajSvobAtm.700adv12.lap700","VlajSvobAtm.700adv12.SkorVertDvig700");
    h.addFieldValue("Температура на уровне 500 гПа","T",0,500,100,"T500adv");
    h.addFieldValue("Дефицит точки росы на уровне 500 гПа","D",0,500,100,"DD500adv");

    h.addFunction("Вертикальная скорость (гПа/12ч) на уровне 400 гПа","SkorVertDvig400",
                  "VlajSvobAtm.insit.lap400","VlajSvobAtm.700adv12.lap400",
                  "VlajSvobAtm.insit.lap500","VlajSvobAtm.700adv12.lap500","VlajSvobAtm.700adv12.SkorVertDvig500");
    h.addFieldValue("Температура на уровне 400 гПа","T",0,400,100,"T400adv");
    h.addFieldValue("Дефицит точки росы на уровне 400 гПа","D",0,400,100,"DD400adv");

    h.addFunction("Вертикальная скорость (гПа/12ч) на уровне 300 гПа","SkorVertDvig300",
                  "VlajSvobAtm.insit.lap300","VlajSvobAtm.700adv12.lap300",
                  "VlajSvobAtm.insit.lap400","VlajSvobAtm.700adv12.lap400","VlajSvobAtm.700adv12.SkorVertDvig400");
    h.addFieldValue("Температура на уровне 300 гПа","T",0,300,100,"T300adv");
    h.addFieldValue("Дефицит точки росы на уровне 300 гПа","D",0,300,100,"DD300adv");


       h.addResultFunction("Прогноз дефицита точки росы на уровне 925 гПа","DeficSvobAtm",
         925,"VlajSvobAtm.insit.DD925","VlajSvobAtm.insit.T925","VlajSvobAtm.700adv12.T925adv","VlajSvobAtm.700adv12.DD925adv","VlajSvobAtm.700adv12.SkorVertDvig925");
       h.addResultFunction("Прогноз дефицита точки росы на уровне 850 гПа","DeficSvobAtm",
         850,"VlajSvobAtm.insit.DD850","VlajSvobAtm.insit.T850","VlajSvobAtm.700adv12.T850adv","VlajSvobAtm.700adv12.DD850adv","VlajSvobAtm.700adv12.SkorVertDvig850");
       h.addResultFunction("Прогноз дефицита точки росы на уровне 700 гПа","DeficSvobAtm",
         700,"VlajSvobAtm.insit.DD700","VlajSvobAtm.insit.T700","VlajSvobAtm.700adv12.T700adv","VlajSvobAtm.700adv12.DD700adv","VlajSvobAtm.700adv12.SkorVertDvig700");
       h.addResultFunction("Прогноз дефицита точки росы на уровне 500 гПа","DeficSvobAtm",
         500,"VlajSvobAtm.insit.DD500","VlajSvobAtm.insit.T500","VlajSvobAtm.700adv12.T500adv","VlajSvobAtm.700adv12.DD500adv","VlajSvobAtm.700adv12.SkorVertDvig500");
       h.addResultFunction("Прогноз дефицита точки росы на уровне 400 гПа","DeficSvobAtm",
         400,"VlajSvobAtm.insit.DD400","VlajSvobAtm.insit.T400","VlajSvobAtm.700adv12.T400adv","VlajSvobAtm.700adv12.DD400adv","VlajSvobAtm.700adv12.SkorVertDvig400");
       h.addResultFunction("Прогноз дефицита точки росы на уровне 300 гПа","DeficSvobAtm",
         300,"VlajSvobAtm.insit.DD300","VlajSvobAtm.insit.T300","VlajSvobAtm.700adv12.T300adv","VlajSvobAtm.700adv12.DD300adv","VlajSvobAtm.700adv12.SkorVertDvig300");
}
