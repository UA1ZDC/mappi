function name(){
  return "Veljtishev"
}

function icon()
{
  return ":/meteo/icons/geopixmap/veter.png";
}

function title(){   
  return "Высота струйного течения и максимальной скорости ветра (метод Н.В. Вельтищева)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;12";
}

function methodHelpFile()
{
  return "Veljtischeva.htm";
}

function typeYavl()
{
  return "6";
}


function init(haveUi){
    new TForecastVeljtishev(haveUi);
}

function TForecastVeljtishev(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();
  
}
TForecastVeljtishev.prototype.initSrc = function(h) {

    h.parent("Исходные данные","calc1", false);
    h.addFunction("Скорость максимального ветра, км/ч","VeljtishevVmax");
    h.addFunction("Высота максимального ветра, км","VeljtishevH");
    h.addFunction("Высота верхней границы слоя, где скорость ветра превышает 0.8*Vmax, км","VeljtishevH2");
    h.addFunction("Высота нижней границы слоя, где скорость ветра превышает 0.8*Vmax, км","VeljtishevH1");
    h.addFunction("Разница толщин слоев с превышением 0.8*Vmax, располагающихся выше и ниже максимума скорости, км","VeljtishevDeltaH","calc1.VeljtishevH","calc1.VeljtishevH1","calc1.VeljtishevH2");
    h.addFunction("Вертикальная протяженность слоя с превышением 0.8*Vmax, км","VeljtishevSize","calc1.VeljtishevH1","calc1.VeljtishevH2");
    h.addResultFunction("Прогноз высоты струйного течения на 12 часов, км","oprVeljtishevHRes",
                        "Veljtishev.calc1.VeljtishevH","Veljtishev.calc1.VeljtishevDeltaH","Veljtishev.calc1.VeljtishevVmax"); //"11002"
    h.addResultFunction("Прогноз максимальной скорости в струйном течении на 12 часов, км/ч","oprVeljtishevVRes",
                        "Veljtishev.calc1.VeljtishevSize","Veljtishev.calc1.VeljtishevVmax");//"11002"

}
