
function name(){
  return "CloudAmount"
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){
  return "Количество облаков нижнего, среднего, верхнего ярусов";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "kol_obl.htm";
}

function typeYavl()
{
  return "3";
}

function init(haveUi){
    new TForecastCloudAmount(haveUi);
}

function TForecastCloudAmount(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();


}

TForecastCloudAmount.prototype.initSrc = function(h) {

   //h.parent("Точка адвекции","adv");
  // h.parent("Подстилающая поверхность", "ad");
 //  h.addField("Поле адвекции 850 гПа","hh",24,850,100,-24,"adv850");
 //  h.addFieldValue("Температура в точке адвекции на уровне 850 гПа","T",0,850,100,"T_850");

  //h.parent("Точка адвекции","adv");
   h.addField("Точка адвекции 700 гПа","hh",0,700,100,-24,"adv700");

   h.addFieldValue("Температура на уровне 1000 гПа,°C:","T",0,1000,100,"T1000");
   h.addFieldValue("Дефицит точки росы на уровне 1000 гПа,°C:","D",0,1000,100,"DD1000");

   h.addFieldValue("Температура на уровне 850 гПа,°C:","T",0,850,100,"T850");
   h.addFieldValue("Дефицит точки росы на уровне 850 гПа,°C:","D",0,850,100,"DD850");

    h.addFieldValue("Температура на уровне 500 гПа,°C:","T",0,500,100,"T500");
    h.addFieldValue("Дефицит точки росы на уровне 500 гПа,°C:","D",0,500,100,"DD500");

    h.addFieldValue("Температура на уровне 300 гПа,°C:","T",0,300,100,"T300");
    h.addFieldValue("Дефицит точки росы на уровне 300 гПа,°C:","D",0,300,100,"DD300");

    h.addResultFunction("Количество облачности на нижнем уровне, %", "getFormul_low", "CloudAmount.adv700.T1000",
                        "CloudAmount.adv700.DD1000","CloudAmount.adv700.T850","CloudAmount.adv700.DD850");

    h.addResultFunction("Количество облачности на среднем уровне, %", "getFormul_middle", "CloudAmount.adv700.T850",
                        "CloudAmount.adv700.DD850","CloudAmount.adv700.T500","CloudAmount.adv700.DD500","CloudAmount.adv700.T1000",
                        "CloudAmount.adv700.DD1000");
    h.addResultFunction("Количество облачности на верхнем уровне, %", "getFormul_high", "CloudAmount.adv700.T500","CloudAmount.adv700.DD500",
                        "CloudAmount.adv700.T300","CloudAmount.adv700.DD300");

}
