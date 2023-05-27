function name(){
  return "VertDvij"
}

function icon()
{
  return ":/meteo/icons/geopixmap/veter.png";
}

function title(){
  return "Скорости вертикальных движений";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;12";
}

function methodHelpFile()
{
  return "vertic.htm";
}

function typeYavl()
{
  return "6";
}


function init(haveUi){
    new TForecastVertDvij(haveUi);
}

function TForecastVertDvij(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();
  
}

TForecastVertDvij.prototype.initSrc = function(h) {

  h.parent("В пункте прогноза","insit");

    h.addFieldAddValue("Лапласиан на уровне земли","hh","lapl", 0,1000,100,"lap0");
    h.addFieldAddValue("Лапласиан на уровне 850 гПа","hh","lapl", 0,850,100,"lap850");
    h.addFieldAddValue("Лапласиан на уровне 700 гПа","hh","lapl", 0,700,100,"lap700");
    h.addFieldAddValue("Лапласиан на уровне 500 гПа","hh","lapl", 0,500,100,"lap500");


  h.addField("Точка адвекции 700 гПа (12 ч.)","hh",0,700,100,-12,"700adv12");
   h.addFieldAddValue("Лапласиан на уровне земли","hh","lapl", 0,1000,100,"lap0");
    h.addFieldAddValue("Лапласиан на уровне 850 гПа","hh","lapl", 0,850,100,"lap850");
    h.addFieldAddValue("Лапласиан на уровне 700 гПа","hh","lapl", 0,700,100,"lap700");
    h.addFieldAddValue("Лапласиан на уровне 500 гПа","hh","lapl", 0,500,100,"lap500");


    h.addResultFunction("Вертикальная скорость (гПа/12ч) на уровне 925 гПа","SkorVertDvig925","VertDvij.insit.lap0","VertDvij.700adv12.lap0");
    h.addResultFunction("Вертикальная скорость (гПа/12ч) на уровне 850 гПа","SkorVertDvig850","VertDvij.insit.lap0","VertDvij.700adv12.lap0");
    h.addResultFunction("Вертикальная скорость (гПа/12ч) на уровне 700 гПа","SkorVertDvig700","VertDvij.insit.lap700","VertDvij.insit.lap850","VertDvij.insit.lap0","VertDvij.700adv12.lap0","VertDvij.700adv12.lap700","VertDvij.700adv12.lap850");
    h.addResultFunction("Вертикальная скорость (гПа/12ч) на уровне 500 гПа","SkorVertDvig500","VertDvij.insit.lap700","VertDvij.insit.lap500","VertDvij.insit.lap850","VertDvij.insit.lap0","VertDvij.700adv12.lap0","VertDvij.700adv12.lap700","VertDvij.700adv12.lap850","VertDvij.700adv12.lap500");


    //SkorVertDvig500(Lapla700,Lapla500,Lapla850,Lapla0,LaplaAdv)

}
