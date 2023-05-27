function name(){
  return "ObledHPol"
}

function icon()
{
  return ":/meteo/icons/geopixmap/obledenenie2.png";
}

function title(){
  return "Обледенение на высоте полета (метод К.Г.Абрамович и И.А.Горлач)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;6";
}

function methodHelpFile()
{
  return "Obledenenie_1.htm";
}

function typeYavl()
{
  return "9";
}

function init(haveUi){
    new TForecastObledHPol(haveUi);
}

function TForecastObledHPol(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastObledHPol.prototype.initSrc = function(h) {
      //ObledIntens(T_s, V_s,TObled,HObled)
  h.addSimple("Cкорость самолета, км/ч","Vsudn",300);
  h.addSimple("Высота полета, м","Hpol",3000);

    h.addFunction("Температура воздуха на высоте полета,°С","Th_p","ObledHPol.Hpol");
    h.addFunction("Высота нижней границы обледенения,м","hObled","ObledHPol.Vsudn");
    h.addFunction("Температура воздуха на нижней границе обледенения,°С","Th_o",
                  "ObledHPol.hObled");

    h.addFunction("Ближайший уровень, гПа","NearLevel","Hpol");
    h.add("Температура на ближайшем уровне,°C","T","ObledHPol.NearLevel",100);
    h.add("Дефицит точки росы на ближайшем уровне,°C","D","ObledHPol.NearLevel",100);
    //////////////////////////////////////////////////////////////////////////////////////////адвективные за 12 ч
    h.addField("Точка адвекции (6 ч)","hh",0,"ObledHPol.NearLevel",100,-6,"adv6");
    h.addFieldValue("Температура,°C","T",0,"ObledHPol.NearLevel",100,"T");
    h.addFieldValue("Дефицит точки росы,°C","D",0,"ObledHPol.NearLevel",100,"D");


  h.addResultFunction("Наличие обледенения на высоте полета",
                      "obledNal",
                      "ObledHPol.Th_p","ObledHPol.Vsudn",
                      "ObledHPol.Hpol");
  h.addResultFunction("Интенсивность обледенения на высоте полета","ObledIntens",
                      "ObledHPol.Th_p","ObledHPol.Vsudn",
                      "ObledHPol.Th_o","ObledHPol.hObled",
                      "ObledHPol.Hpol",
                      "ObledHPol.T","ObledHPol.adv6.T",
                      "ObledHPol.D","ObledHPol.adv6.D");

}
