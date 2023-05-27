function name(){
  return "TitovVNGO"
}

function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){
  return "Высота нижней границы внутримассовой конвективной облачности (метод Титова)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0";
}


function hours(){ //период действия прогноза (отсчитывается от times)
  return "12;21"; //TODO время должно быть местное!!
}

function methodHelpFile()
{
  return "VNGOTitov.htm";
}

function season()
{
  return "1;2;3;";
}

function typeYavl()
{
  return "3";
}



TForecastHeader.prototype.addResult = function(itemText, parentItem, itemIndex)
{
  obj.addResultItem(itemText,parentItem, itemIndex);
}

function init(haveUi){
    new TForecastTitovVNGO(haveUi);
}

function TForecastTitovVNGO(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastTitovVNGO.prototype.initSrc = function(h) {
// h.parent("На уровне станции","srf");
 // h.add("Давление, гПа","P",0,1);//добавляет значение по станции (ближайшей к текущим координатам станции )
  // h.add("Давление, гПа","P0",0,1);//добавляет значение по станции (ближайшей к текущим координатам станции )
  // h.add("Температура,°C","T",0,1);
  // h.add("Температура точки росы,°C","Td",0,1);
 // h.add("Дефицит точки росы,°C","D",0,1);

 h.parent("В свободной атмосфере на уровне 900 гПа","aer900");
    h.add("Температура,°C","T",925,100);

 h.parent("В свободной атмосфере на уровне 850 гПа","aer850");
    h.add("Температура,°C","T",850,100);
    h.add("Дефицит точки росы,°C","D",850,100);
    h.add("Температура точки росы,°C","Td",850,100);

 h.parent("В свободной атмосфере на уровне 700 гПа","aer700");
    h.add("Температура,°C","T",700,100);
    h.add("Дефицит точки росы,°C","D",700,100);
    h.add("Температура точки росы,°C","Td",700,100);
  h.parent("Расчетные значения","cal");
    h.addFunction("Максимальная температура,°C","TitovTMax",
                "TitovVNGO.aer700.T","TitovVNGO.aer700.Td","TitovVNGO.aer700.D",
                "TitovVNGO.aer850.T","TitovVNGO.aer850.Td","TitovVNGO.aer850.D",
                "TitovVNGO.aer900.T")

  h.addResultFunction("Количество конвективной облачности,%", "getNTitov","TitovVNGO.cal.TitovTMax");//TODO размерность??? //"20011"

  h.addResultFunction("Высота конвективной облачности, м  ","getLowBorderTitov","TitovVNGO.cal.TitovTMax"); //"20013"
}
