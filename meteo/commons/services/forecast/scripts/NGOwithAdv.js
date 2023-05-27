function name(){
  return "NGOwithAdv";
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){
  return "Высота нижней границы слоиcтой облачности при адвекции (метод З.А.Спарышкиной)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;12";
}

function methodHelpFile()
{
  return "SparishkinaAdv.htm";
}

function season()
{
  return "0;1;3";
}

function synsit()
{
  return "-1";
}

function typeYavl()
{
  return "3";
}

function init(haveUi){
    new TForecastNGOwithAdv(haveUi);
}

function TForecastNGOwithAdv(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

//заблаговременность прогноза 6-12 часов, значит нужно связать заблаговременность с адвекцией
//+ не понятно как выбирать срок для полей ???

TForecastNGOwithAdv.prototype.initSrc = function(h) {

//  h.addFieldValue("Прогноз температуры на 6 ч,°С","T",6,0,1,"TF6");
//  h.addFieldValue("Прогноз температуры на 12 ч,°С","T",12,0,1,"TF12");

  h.parent("В пункте прогноза","surf");
    h.add("Температура,°С","T",0,1);
    h.add("Температура точки росы,°С","Td",0,1);
    h.addFieldValue("Прогноз температуры на 6 ч,°С","T",6,0,1,"TF6");
    h.addFieldValue("Прогноз температуры на 12 ч,°С","T",12,0,1,"TF12");

  h.addField("Поле адвекции по 700 гПа(6 ч)","hh",0,700,100,-6,"progn6");
    h.addFieldValue("Температура на уровне земли,°С","T",0,0,1,"T6");
    h.addFieldValue("Температура точки росы на уровне земли,°С","Td",0,0,1,"Td6");

  h.addField("Поле адвекции по 700 гПа (12 ч)","hh",0,700,100,-12,"progn12");
    h.addFieldValue("Температура на уровне земли,°С","T",0,0,1,"T12");
    h.addFieldValue("Температура точки росы на уровне земли,°С","Td",0,0,1,"Td12");
    //h.add("Температура на уровне земли,°С","T",0,1);
    //h.add("Температура точки росы на уровне земли,°С","Td",0,1);


  h.addResultFunction("Предварительная оценка облачности через 6 ч","nalObl","NGOwithAdv.T",
                        "NGOwithAdv.progn6.T6");
  h.addResultFunction("Предварительная оценка облачности через 12 ч","nalObl","NGOwithAdv.T",
                      "NGOwithAdv.progn12.T12");

  h.addResultFunction("ВНГО слоистой облачности через 6 ч","VGNOAdv6","NGOwithAdv.Td",
                      "NGOwithAdv.progn6.Td6","NGOwithAdv.TF6",
                      "NGOwithAdv.T","NGOwithAdv.progn6.T6");
  h.addResultFunction("ВНГО слоистой облачности через 12 ч","VGNOAdv12","NGOwithAdv.Td",
                      "NGOwithAdv.progn12.Td12","NGOwithAdv.TF12",
                      "NGOwithAdv.T", "NGOwithAdv.progn12.T12");
  // h.addResultFunction("10002","Ниже 300 м:","nalObl","NGOwithAdv.surf.T","NGOwithAdv.progn.T");


}

