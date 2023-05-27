function name(){
  return "TitovTmax"
}

function icon()
{
  return ":/meteo/icons/geopixmap/temperatura.png";
}

function title(){
 return  "Максимальная температура за день (метод Титова)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0";
}


function hours(){ //период действия прогноза (отсчитывается от times)
  return "12;18"; //TODO время должно быть местное!!
}

function methodHelpFile()
{
  return "TMaxTitov.htm";
}

function season()
{
  return "1;2;3";
}

function typeYavl()
{
  return "4";
}


function init(haveUi){
    new TForecastTitovTmax(haveUi);
}

function TForecastTitovTmax(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastTitovTmax.prototype.initSrc = function(h) {

  h.add("Количество облачности CL или CM, б","Nh",0,1);
  h.parent("В свободной атмосфере на уровне 900 гПа","aer900");
    h.add("Температура,°C","T",925,100);
  h.parent("В свободной атмосфере на уровне 850 гПа","aer850");
    h.add("Температура,°C","T",850,100);
    h.add("Температура точки росы,°C","Td",850,100);
    h.add("Дефицит точки росы,°C","D",850,100);
  h.parent("В свободной атмосфере на уровне 700 гПа","aer700");
    h.add("Температура,°C","T",700,100);
    h.add("Температура точки росы,°C","Td",700,100);
    h.add("Дефицит точки росы,°C","D",700,100);
  h.addResultFunction("Максимальная температура,°C","TitovTMax","TitovTmax.Nh","TitovTmax.aer700.T",
                      "TitovTmax.aer700.Td","TitovTmax.aer700.D",
                      "TitovTmax.aer850.T","TitovTmax.aer850.Td","TitovTmax.aer850.D",
                      "TitovTmax.aer900.T");//"12111"

}
