function name(){
  return "Obled850700"
}

function icon()
{
  return ":/meteo/icons/geopixmap/obledenenie2.png";
}

function title(){
  return "Обледенение на уровнях 850, 700 и 500 гПа (метод К.Г.Абрамович и И.А.Горлач)";

}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;12";
}

function methodHelpFile()
{
  return "Obledenenie_2.htm";
}

function typeYavl()
{
  return "9";
}

function init(haveUi){
    new TForecastObled850700(haveUi);
}

function TForecastObled850700(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastObled850700.prototype.initSrc = function(h) {
//    ObledUr850(T850,D850,progW850,Vs)
    h.addSimple("Скорость полёта самолёта","Vs",300);

    h.addFieldAddValue("Лапласиан в пункте прогноза на уровне земли","hh","lapl", 0,1000,100,"lap0");
    h.addFieldAddValue("Лапласиан в пункте прогноза на уровне 850 гПа","hh","lapl", 0,850,100,"lap850");
    h.addFieldAddValue("Лапласиан в пункте прогноза на уровне 700 гПа","hh","lapl", 0,700,100,"lap700");
    h.addFieldAddValue("Лапласиан в пункте прогноза на уровне 500 гПа","hh","lapl", 0,500,100,"lap500");

    h.addField("Точка адвекции 700 гПа (12 ч)","hh",0,700,100,-12,"700adv12");
    h.addFieldAddValue("Лапласиан на уровне земли","hh","lapl", 0,1000,100,"lap0");


  h.parent("Данные на уровне 850 гПа","init850");
//    h.addSimple("Прогноз скорости упорядоченных вертикальных движений на уровне 850 гПа (на 12ч, гПа/12ч)","W850",5);
    h.add("Температура,°C","T",850,100);
    h.add("Дефицит точки росы,°C","D",850,100);
    h.addFunction("Вертикальная скорость на уровне 850 гПа","SkorVertDvig850","Obled850700.lap0","Obled850700.700adv12.lap0");

  h.parent("Данные на уровне 700 гПа","init700");
//    h.addSimple("Прогноз скорости упорядоченных вертикальных движений на уровне 700 гПа (на 12ч, гПа/12ч)","W700",4);
    h.add("Температура,°C","T",700,100);
    h.add("Дефицит точки росы,°C","D",700,100);
    h.addFunction("Вертикальная скорость на уровне 700 гПа","SkorVertDvig700","Obled850700.lap700","Obled850700.lap850","Obled850700.lap0","Obled850700.700adv12.lap0");

  h.parent("Данные на уровне 500 гПа","init500");
//    h.addSimple("Прогноз скорости упорядоченных вертикальных движений на уровне 500 гПа (на 12ч, гПа/12ч)","W500",6);
    h.add("Температура,°C","T",500,100);
    h.add("Дефицит точки росы,°C","D",500,100);
    h.addFunction("Вертикальная скорость на уровне 500 гПа","SkorVertDvig500","Obled850700.lap700","Obled850700.lap500","Obled850700.lap850","Obled850700.lap0","Obled850700.700adv12.lap0");


//ObledUr850(T850,D850,Vs,W850pr)
//    ObledUr500(T500,D500,Vs,Lapla700,Lapla500,Lapla850,Lapla0,LaplaAdv)
  h.addResultFunction("Прогноз обледенения на уровне 850 гПа","ObledUr850","Obled850700.init850.T","Obled850700.init850.D","Obled850700.Vs","Obled850700.init850.SkorVertDvig850");

  h.addResultFunction("Прогноз обледенения на уровне 700 гПа","ObledUr700","Obled850700.init700.T","Obled850700.init700.D","Obled850700.Vs","Obled850700.init700.SkorVertDvig700");

  h.addResultFunction("Прогноз обледенения на уровне 500 гПа","ObledUr500","Obled850700.init500.T","Obled850700.init500.D","Obled850700.Vs","Obled850700.init500.SkorVertDvig500");

}
