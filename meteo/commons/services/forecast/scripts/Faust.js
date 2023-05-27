function name(){
  return "Faust";
}

function icon()
{
  return ":/meteo/icons/geopixmap/groza2.png";
}

function title(){
  return "Грозы (метод Фауста)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Faust.htm";
}

function season()
{
  return "1;2;3";
}

function typeYavl()
{
  return "0";
}

function init(haveUi){
    new TForecastFaust(haveUi);
}


function TForecastFaust(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastFaust.prototype.initSrc = function(h) {
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
        h.addFieldAddValue("Радиус кривизны изобар,км","P","radKriv", 0,0,101,"FaustD");
    h.addResultFunction("Степень конвективной неустойчивости","FaustK",
                        "Faust.aer500.T","Faust.aer500.Td","Faust.aer500.D",
                        "Faust.aer700.T","Faust.aer700.Td","Faust.aer700.D",
                        "Faust.aer850.T","Faust.aer850.Td","Faust.aer850.D",
                        "Faust.aer500.FaustD");  // Значение индекса К
    h.addResultFunction("Прогноз грозы","FaustDescr","FaustK");  // Формулировка
}
