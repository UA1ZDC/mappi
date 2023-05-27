function name(){
  return "VaytingFaust"
}

function icon()
{
  return ":/meteo/icons/geopixmap/groza2.png";
}

function title(){
  return "Грозы (метод Вайтинга-Фауста)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;21";
}

function methodHelpFile()
{
  return "Wayting_Faust.htm";
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
    new TForecastVayting(haveUi);
}

TForecastHeader.prototype.addResult = function(itemText, parentItem, itemIndex)
{
  obj.addResultItem(itemText,parentItem, itemIndex);
}

function TForecastVayting(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastVayting.prototype.initSrc = function(h) {
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

         h.addFieldAddValue("Радиус кривизны изобар, км","P","radKriv", 0,0,101,"FaustDt");

    h.addResultFunction("Критерий неустойчивости по Вайтингу","VaytingK","aer500.T",
                        "aer700.T","aer700.Td","aer700.D",
                        "aer850.T","aer850.Td","aer850.D");

    h.addResultFunction("Критерий неустойчивости по Фаусту","FaustK",
                        "aer500.T","aer500.Td","aer500.D",
                        "aer700.T","aer700.Td","aer700.D",
                        "aer850.T","aer850.Td","aer850.D",
                        "VaytingFaust.FaustDt");

    h.addResultFunction("Прогноз грозы","VaytingFaustDescr","VaytingK","FaustK");  // Формулировка
}
