function name(){
  return "RubinCloud"
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){
  return "Появление (сохранение) облачности с нижней границей менее 100м (метод М.В.Рубинштейна)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
 // return "0;3;6;9;12;15;18;24";
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;6";
}

function methodHelpFile()
{
  return "Rubinshtein.htm";
}

function typeYavl()
{
  return "3";
}

function init(haveUi){
    new TForecastRubinCloud(haveUi);
}

function TForecastRubinCloud(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastRubinCloud.prototype.initSrc = function(h) {
  h.parent("На уровне земли","surf");

  var srok = 0,
      hour = dataObj.getHourData();

  h.add("Температура,°С", "T",0,1);
  h.add("Дефицит точки росы,°C", "D",0,1);
  h.add("Скорость ветра на уровне флюгера, м/с", "ff",0,1);

 h.parent("На уровне 600 м","ur600");
 h.addFunction("Скорость ветра, м/с", "getWindVelocityFromZond");

 h.parent("На уровне первой особой точки по температуре","osob");
  h.addFunction("Высота, м", "getHpoint");
  h.addFunction("Температура,°С", "TOsobPoint");
  h.addFunction("Дефицит точки росы,°С", "DOsobPoint");
 h.parent("Вертикальный градиент температуры","grad");
  h.addFunction("°С/100м","gradientT","RubinCloud.surf.T","RubinCloud.osob.TOsobPoint","RubinCloud.osob.getHpoint");//gradientT(T0,Tpoint)

h.addResultFunction("Облачность:","NalOblRubinsh","RubinCloud.surf.ff", "RubinCloud.ur600.getWindVelocityFromZond",
                    "RubinCloud.osob.DOsobPoint","RubinCloud.surf.D","RubinCloud.grad.gradientT");

  //T      - температура на уровне станции
  //Tpoint - температура на высоте первой особой точки по температуре
  //Tdpoint- температура точки росы на высоте первой особой точки
  //VgradT - вертикальный градиент температуры в слое от поверхности земли до первой особой точки
  //D0     - дефицит точки росы у поверхности земли
  //Dpoint - дефицит точки росы на высоте первой особой точки  (DD1Temp)


}

