function name(){
  return "Slavin"
}

function icon()
{
  return ":/meteo/icons/geopixmap/groza2.png";
}

function title(){
  return "Грозы (метод Славина)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;21";
}

function methodHelpFile()
{
  return "Slavin.htm";
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
    new TForecastSlavin(haveUi);
}

function TForecastSlavin(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}


TForecastSlavin.prototype.initSrc = function(h) {
 h.parent("На уровне станции","st");
     h.add("Величина барической тенденции, ГПа","p",0,1);
     h.addFieldAddValue("Радиус кривизны изобар, км","P","radKriv", 0,0,101,"dT");
     h.addFunction("Тип кривизны изобар","tipKriv","Slavin.st.dT");
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

 h.addResultFunction("Грозовой индекс","getSlavinDopAdd","Slavin.aer500.T",
                     "Slavin.aer700.T","Slavin.aer700.Td","Slavin.aer700.D",
                     "Slavin.aer850.T","Slavin.aer850.Td","Slavin.aer850.D",
                     "Slavin.st.p","Slavin.st.dT","Slavin.st.tipKriv",0);
 h.addResultFunction("Прогноз грозы","getSlavinDopAdd","Slavin.aer500.T",
                     "Slavin.aer700.T","Slavin.aer700.Td","Slavin.aer700.D",
                     "Slavin.aer850.T","Slavin.aer850.Td","Slavin.aer850.D",
                     "Slavin.st.p","Slavin.st.dT","Slavin.st.tipKriv",1);

}
