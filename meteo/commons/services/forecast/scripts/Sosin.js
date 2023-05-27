function name(){
  return "Sosin"
}

function icon()
{
  return ":/meteo/icons/geopixmap/groza2.png";
}

function title(){
  return "Грозы (метод Сосина)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;21"; //Прогноз составляется на период от 12 до 21 часа от местного  времени.
}

function init(haveUi){
    new TForecastSosin(haveUi);
}

function methodHelpFile()
{
  return "Sosin.htm";
}
function season()
{
  return "1;2;3";
}

function typeYavl()
{
  return "0";
}


function TForecastSosin(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastSosin.prototype.initSrc = function(h) {
    h.parent("В свободной атмосфере на уровне 850 гПа","aer850");
        h.add("Температура,°C","T",850,100);
        h.add("Дефицит точки росы,°C","D",850,100);
        h.add("Температура точки росы,°C","Td",850,100);
    h.parent("В свободной атмосфере на уровне 700 гПа","aer700");
        h.add("Температура,°C","T",700,100);
        h.add("Дефицит точки росы,°C","D",700,100);
        h.add("Температура точки росы,°C","Td",700,100);
    h.parent("В свободной атмосфере на уровне 500 гПа","aer500");
        h.add("Температура,°C","T",500,100);
        h.add("Дефицит точки росы,°C","D",500,100);
        h.add("Температура точки росы,°C","Td",500,100);

    h.addResultFunction("Прогноз грозы","SosinDescr",
                        "Sosin.aer500.T","Sosin.aer500.Td","Sosin.aer500.D",
                        "Sosin.aer700.T","Sosin.aer700.Td","Sosin.aer700.D",
                        "Sosin.aer850.T","Sosin.aer850.Td","Sosin.aer850.D");
}
