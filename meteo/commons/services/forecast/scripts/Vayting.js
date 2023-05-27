function name(){
  return "Vayting"
}

function icon()
{
  return ":/meteo/icons/geopixmap/groza2.png";
}

function title(){
  return "Грозы (метод Вайтинга)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;21";
}

function methodHelpFile()
{
  return "Wayting.htm";
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
 // var title;
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

     h.addResultFunction("Степень конвективной неустойчивости","VaytingK","Vayting.aer500.T",
                         "Vayting.aer700.T","Vayting.aer700.Td","Vayting.aer700.D",
                         "Vayting.aer850.T","Vayting.aer850.Td","Vayting.aer850.D");  // Значение индекса К
     h.addResultFunction("Прогноз грозы","VaytingDescr","VaytingK");  // Формулировка
}
