function name(){
  return "SparyshCloud"
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){
  return "Высота нижней границы слоистой облачности при отсутствии адвекции (метод З.А.Спарышкиной)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "VNGOSparishkina.htm";
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
    new TForecastSparyshCloud(haveUi);
}

function TForecastSparyshCloud(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastSparyshCloud.prototype.initSrc = function(h) {
    h.addFieldValue("Прогноз приземной температуры на 6 ч,°С","T",6,0,1,"TF6");

    h.add("Приземная температура,°С", "T",0,1);

    h.addFunction("Температура на верхней границе инверсии/изотермии (или на уровне 925 гПа),°С", "getTinvers");
    h.addFunction("Высота верхней границы инверсии/изотермии (или высота уровня 925 гПа), м", "getHinvers");
    h.addFunction("Вертикальный градиент температуры в слое от земли до верхней границы инверсии/изотермии", "gradT",
                    "SparyshCloud.T", "SparyshCloud.getTinvers","SparyshCloud.getHinvers");

    h.addResultFunction("Высота нижней границы слоистой облачности, м:","NGOSparyshkinoi","SparyshCloud.TF6","SparyshCloud.gradT");//"20013"




}

