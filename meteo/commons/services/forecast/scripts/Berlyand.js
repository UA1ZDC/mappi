function name(){
  return "Berlyand"
}

function icon()
{
  return ":/meteo/icons/geopixmap/tuman.png";
}

function title(){
  return "Дымка при видимости 1-2 км, радиационный туман (метод Берлянда М.Е.)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "12;18";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;12";
}

function methodHelpFile()
{
  return "Berljand.htm";
}

function season()
{
  return "1;2;3";
}

function typeYavl()
{
  return "2";
}

function init(haveUi){
    new TForecastBerlyand(haveUi);
}

function TForecastBerlyand(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastBerlyand.prototype.initSrc = function(h) {
//    h.add("Общее количество облачности","N",0,1);
    h.add("Облачность на нижнем уровне, б","Nh",0,1);
    h.addSimple("Облачность на среднем уровне, б","Nmid", 0);
    h.addSimple("Облачность на верхнем уровне, б","Nhi", 0);

    h.addFieldValue("Ожидаемая скорость ветра","ff",12,0,1,"skorPr");

    h.addCombo("Тип подстилающей поверхности","tip","Сухая;Влажная;Мокрая");

    //h.addFunction("Время восхода солнца","sunrise");
    h.addTime("Время восхода Cолнца, UTC","sunrise");

   h.parent("Температура за 4 срока","temp");
    h.addFieldValue("Температура в исходный срок,°C","T",0,0,1,"ish1");
    h.addFieldValue("Температура за 6 часов до исходного срока,°C","T",-6,0,1,"ish2");
    h.addFieldValue("Температура за 12 часов до исходного срока,°C","T",-12,0,1,"ish3");
    h.addFieldValue("Температура за 18 часов до исходного срока,°C","T",-18,0,1,"ish4");

   h.parent("Температура точки росы за 4 срока","dtemp");
    h.addFieldValue("Температура точки росы в исходный срок,°C","Td",0,0,1,"ishtd1");
    h.addFieldValue("Температура точки росы за 6 часов до исходного срока,°C","Td",-6,0,1,"ishtd2");
    h.addFieldValue("Температура точки росы за 12 часов до исходного срока,°C","Td",-12,0,1,"ishtd3");
    h.addFieldValue("Температура точки росы за 18 часов до исходного срока,°C","Td",-18,0,1,"ishtd4");

    h.addResultFunction("Температурa туманообразования","getT_tum","Berlyand.dtemp.ishtd4","Berlyand.dtemp.ishtd3","Berlyand.dtemp.ishtd1","Berlyand.dtemp.ishtd2");
    h.addResultFunction("Минимальная температура","T_minBer","Berlyand.ish1","Berlyand.ish2","Berlyand.ish3","Berlyand.ish4","Berlyand.ishtd1","Berlyand.Nhi","Berlyand.Nmid","Berlyand.Nh","Berlyand.skorPr","Berlyand.tip","Berlyand.sunrise");
    h.addResultFunction("Прогноз тумана или дымки методом Берлянда","getTumDim","getT_tum","T_minBer");
    h.addResultFunction("Прогноз видимости в тумане или дымке методом Берлянда","getVidTumDim","T_minBer","getT_tum");

    // исходные данные за 13 или 18(19) ч: температура воздуха (Т0), температура точки росы ,
// упругость водяного пара (е0) и визуальная оценка увлажненности почвы (А – сухая, Б – влажная, В – мокрая)
//+  данные о температуре воздуха за четыре срока наблюдений, отстоящие друг от друга на 6 ч
//для 13 ч: 13 07 01 19
//для 18 ч: 18 12 6 00
//Прогноз составляется на период от времени захода Солнца до его восхода плюс один час.


}
