function name(){
  return "CloudEvo"
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){
  return "Эволюция облачности в слоях";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;3;6;9;12;15;18;21";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "12;18"; //TODO время должно быть местное!!
}

function methodHelpFile()
{
  return "EvoObl.htm";
}


function typeYavl()
{
  return "3";
}

function init(haveUi){
    new TForecastCloudEvo(haveUi);
}

function TForecastCloudEvo(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastCloudEvo.prototype.initSrc = function(h) {
    h.parent("В пункте прогноза на уровне 850 гПа","aer850");
        h.add("Температура,°C","T",850,100);
        h.add("Дефицит точки росы,°C","D",850,100);
    h.parent("В пункте прогноза на уровне 700 гПа","aer700");
        h.add("Температура,°C","T",700,100);
        h.add("Дефицит точки росы,°C","D",700,100);
    h.parent("В пункте прогноза на уровне 500 гПа","aer500");
        h.add("Температура,°C","T",500,100);
        h.add("Дефицит точки росы,°C","D",500,100);
//////////////////////////////////////////////////////////////////////////////////////////адвективные за 12 ч
    h.addField("Точка адвекции 850 гПа (12 ч.)","hh",0,850,100,-12,"850adv12");
    h.addFieldValue("Температура на уровне 850 гПа,°C","T",0,850,100,"850adv_T850_12");
    h.addFieldValue("Дефицит точки росы на уровне 850 гПа,°C","D",0,850,100,"850adv_D850_12");

    h.addField("Точка адвекции 700 гПа (12 ч.)","hh",0,700,100,-12,"700adv12");
    h.addFieldValue("Температура на уровне 700 гПа,°C","T",0,700,100,"700adv_T700_12");
    h.addFieldValue("Дефицит точки росы на уровне 700 гПа,°C","D",0,700,100,"700adv_D700_12");

    h.addField("Точка адвекции 500 гПа (12 ч.)","hh",0,500,100,-12,"500adv12");
    h.addFieldValue("Температура на уровне 500 гПа,°C","T",0,500,100,"500adv_T500_12");
    h.addFieldValue("Дефицит точки росы на уровне 500 гПа,°C","D",0,500,100,"500adv_D500_12");
//////////////////////////////////////////////////////////////////////////////////////////адвективные за 18 ч
    h.addField("Точка адвекции 850 гПа (18 ч.)","hh",0,850,100,-18,"850adv18");
    h.addFieldValue("Температура на уровне 850 гПа,°C","T",0,850,100,"850adv_T850_18");
    h.addFieldValue("Дефицит точки росы на уровне 850 гПа,°C","D",0,850,100,"850adv_D850_18");

    h.addField("Точка адвекции 700 гПа (18 ч.)","hh",0,700,100,-18,"700adv18");
    h.addFieldValue("Температура на уровне 700 гПа,°C","T",0,700,100,"700adv_T700_18");
    h.addFieldValue("Дефицит точки росы на уровне 700 гПа,°C","D",0,700,100,"700adv_D700_18");

    h.addField("Точка адвекции 500 гПа (18 ч.)","hh",0,500,100,-18,"500adv18");
    h.addFieldValue("Температура на уровне 500 гПа,°C","T",0,500,100,"500adv_T500_18");
    h.addFieldValue("Дефицит точки росы на уровне 500 гПа,°C","D",0,500,100,"500adv_D500_18");

    h.addResultFunction("Эволюция облачности в слое 1-2 км, прогноз на 12 ч","cloudSt",850,"CloudEvo.aer850.T","CloudEvo.850adv12.850adv_T850_12","CloudEvo.aer850.D","CloudEvo.850adv12.850adv_D850_12",12);
    h.addResultFunction("Эволюция облачности в слое 2-4 км, прогноз на 12 ч","cloudSt",700,"CloudEvo.aer700.T","CloudEvo.700adv12.700adv_T700_12","CloudEvo.aer700.D","CloudEvo.700adv12.700adv_D700_12",12);
    h.addResultFunction("Эволюция облачности в слое 4-6 км, прогноз на 12 ч","cloudSt",500,"CloudEvo.aer500.T","CloudEvo.500adv12.500adv_T500_12","CloudEvo.aer500.D","CloudEvo.500adv12.500adv_D500_12",12);
 /////////////////////////////////18 часов
    h.addResultFunction("Эволюция облачности в слое 1-2 км, прогноз на 18 ч","cloudSt",850,"CloudEvo.aer850.T","CloudEvo.850adv18.850adv_T850_18","CloudEvo.aer850.D","CloudEvo.850adv18.850adv_D850_18",18);
    h.addResultFunction("Эволюция облачности в слое 2-4 км, прогноз на 18 ч","cloudSt",700,"CloudEvo.aer700.T","CloudEvo.700adv18.700adv_T700_18","CloudEvo.aer700.D","CloudEvo.700adv18.700adv_D700_18",18);
    h.addResultFunction("Эволюция облачности в слое 4-6 км, прогноз на 18 ч","cloudSt",500,"CloudEvo.aer500.T","CloudEvo.500adv18.500adv_T500_18","CloudEvo.aer500.D","CloudEvo.500adv18.500adv_D500_18",18);

//    h.addResultFunction("Эволюция облачности в слое 1-2 км, прогноз на 18 ч","cloudSt","CloudEvo.aer850.T","CloudEvo.850adv18.T850_18","CloudEvo.aer850.DD1","CloudEvo.850adv18.D850_18");
//    h.addResultFunction("Эволюция облачности в слое 2-4 км, прогноз на 18 ч","cloudSt","CloudEvo.aer700.T","CloudEvo.700adv18.T700_18","CloudEvo.aer700.DD1","CloudEvo.700adv18.D700_18");
//    h.addResultFunction("Эволюция облачности в слое 4-6 км, прогноз на 18 ч","cloudSt","CloudEvo.aer500.T","CloudEvo.500adv18.T500_18","CloudEvo.aer500.DD1","CloudEvo.500adv18.D500_18");

}
