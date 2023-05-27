function name(){
  return "Reshetov1"
}
function icon()
{
  return ":/meteo/icons/geopixmap/oblachnost.png";
}

function title(){   
  return "Метод прогнозирования шквалов (метод Решетова 1)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}
function init(haveUi){
    new TForecastReshetov1(haveUi);
}

function TForecastReshetov1(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();
  
}
TForecastReshetov1.prototype.initSrc = function(h) {
    h.parent("На уровне станции","st");
        h.add("Давление, мбар","PP0",1500);
//        h.add("Температура, С","TT0",1500);
//        h.add("Точка росы, С","Td0",1500);
        h.add("Температура, С","T",0);
        h.add("Точка росы, С","Td",0);
    h.parent("В свободной атмосфере на уровне 1000 гПа","aer1000");
        h.add("Температура, C","T",1000);
        h.add("Точка росы, C","Td",1000);
        h.add("Высота, м","hh",1000);
    h.parent("В свободной атмосфере на уровне 900 гПа","aer900");
        h.add("Температура, C","T",900);
        h.add("Точка росы, C","Td",900);
        h.add("Высота, м","hh",900);
    h.parent("В свободной атмосфере на уровне 850 гПа","aer850");
        h.add("Температура, C","T",850);
        h.add("Точка росы, C","Td",850);
        h.add("Высота, м","hh",850);
    h.parent("В свободной атмосфере на уровне 700 гПа","aer700");
        h.add("Температура, C","T",700);
        h.add("Точка росы, C","Td",700);
        h.add("Высота, м","hh",700);
    h.parent("В свободной атмосфере на уровне 500 гПа","aer500");
        h.add("Температура, C","T",500);
        h.add("Точка росы, C","Td",500);
        h.add("Высота, м","hh",500);
    h.parent("В свободной атмосфере на уровне 400 гПа","aer400");
        h.add("Температура, C","T",400);
        h.add("Точка росы, C","Td",400);
        h.add("Высота, м","hh",400);    

    h.parent("Прогнозируемые значения","calc1");
        //h.addFunction("Максимальная temp","titovTMax","aer700.T","aer700.Td","aer850.T","aer850.Td","aer900.T");
        h.addFunction("Максимальная скорость ветра при шквале","Reshetov1K","st.PP0","st.T","st.Td","aer1000.T","aer1000.Td","aer1000.hh","aer900.T","aer850.T","aer850.Td","aer850.hh","aer700.T","aer700.Td","aer700.hh","aer500.T","aer500.Td","aer500.hh","aer400.T","aer400.Td","aer400.hh");
        h.addFunction("Прогноз шквала","Reshetov1Descr","calc1.Reshetov1K");  // Формулировка
}
