function name(){
  return "VeterSvAtm"
}

function icon()
{
  return ":/meteo/icons/geopixmap/veter.png";
}


function init(haveUi){
    new TForecastVeterSvAtm(haveUi);
}

function title(){
  return  "Скорость и направление ветра в свободной атмосфере";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "veterH_P.htm";
}

function typeYavl()
{
  return "6";
}


function TForecastVeterSvAtm(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastVeterSvAtm.prototype.initSrc = function(h) {

 h.parent("В свободной атмосфере на уровне 925 гПа","aer925");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,925,100,"lap925");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,925,100,"gradX925");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,925,100,"gradY925");
 h.parent("В свободной атмосфере на уровне 850 гПа","aer850");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,850,100,"lap850");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,850,100,"gradX850");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,850,100,"gradY850");

 h.parent("В свободной атмосфере на уровне 700 гПа","aer700");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,700,100,"lap700");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,700,100,"gradX700");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,700,100,"gradY700");

 h.parent("В свободной атмосфере на уровне 500 гПа","aer500");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,500,100,"lap500");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,500,100,"gradX500");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,500,100,"gradY500");

 h.parent("В свободной атмосфере на уровне 400 гПа","aer400");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,400,100,"lap400");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,400,100,"gradX400");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,400,100,"gradY400");
 h.parent("В свободной атмосфере на уровне 300 гПа","aer300");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,300,100,"lap300");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,300,100,"gradX300");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,300,100,"gradY300");
 h.parent("В свободной атмосфере на уровне 200 гПа","aer200");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,200,100,"lap200");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,200,100,"gradX200");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,200,100,"gradY200");
 h.parent("В свободной атмосфере на уровне 100 гПа","aer100");
    h.addFieldAddValue("Прогноз лапласиана геопотенциала на 12ч","hh","lapl", 12,100,100,"lap100");
    h.addFieldAddValue("Прогноз градиента геопотенциала OX на 12ч","hh","gradx", 12,100,100,"gradX100");
    h.addFieldAddValue("Прогноз градиента геопотенциала OY на 12ч","hh","grady", 12,100,100,"gradY100");




//skor_veter_Sv_atm(uroven,fi_h, fi_mim, lap_P, Pdelta_0X, Pdelta_0Y)

    h.addResultFunction("Скорость ветра на уровне 925 гПа, м","skor_veter_Sv_atm",925,"VeterSvAtm.aer925.lap925","VeterSvAtm.aer925.gradX925","VeterSvAtm.aer925.gradY925");
    h.addResultFunction("Направление ветра на уровне 925 гПа, градусы","napr_veter_Sv_atm",925,"VeterSvAtm.aer925.lap925","VeterSvAtm.aer925.gradX925","VeterSvAtm.aer925.gradY925");

    h.addResultFunction("Скорость ветра на уровне 850 гПа, м","skor_veter_Sv_atm",850,"VeterSvAtm.aer850.lap850","VeterSvAtm.aer850.gradX850","VeterSvAtm.aer850.gradY850");
    h.addResultFunction("Направление ветра на уровне 850 гПа, градусы","napr_veter_Sv_atm",850,"VeterSvAtm.aer850.lap850","VeterSvAtm.aer850.gradX850","VeterSvAtm.aer850.gradY850");

    h.addResultFunction("Скорость ветра на уровне 700 гПа, м","skor_veter_Sv_atm",700,"VeterSvAtm.aer700.lap700","VeterSvAtm.aer700.gradX700","VeterSvAtm.aer700.gradY700");
    h.addResultFunction("Направление ветра на уровне 700 гПа, градусы","napr_veter_Sv_atm",700,"VeterSvAtm.aer700.lap700","VeterSvAtm.aer700.gradX700","VeterSvAtm.aer700.gradY700");

    h.addResultFunction("Скорость ветра на уровне 500 гПа, м","skor_veter_Sv_atm",500,"VeterSvAtm.aer500.lap500","VeterSvAtm.aer500.gradX500","VeterSvAtm.aer500.gradY500");
    h.addResultFunction("Направление ветра на уровне 500 гПа, градусы","napr_veter_Sv_atm",500,"VeterSvAtm.aer500.lap500","VeterSvAtm.aer500.gradX500","VeterSvAtm.aer500.gradY500");

    h.addResultFunction("Скорость ветра на уровне 400 гПа, м","skor_veter_Sv_atm",400,"VeterSvAtm.aer400.lap400","VeterSvAtm.aer400.gradX400","VeterSvAtm.aer400.gradY400");
    h.addResultFunction("Направление ветра на уровне 400 гПа, градусы","napr_veter_Sv_atm",400,"VeterSvAtm.aer400.lap400","VeterSvAtm.aer400.gradX400","VeterSvAtm.aer400.gradY400");

    h.addResultFunction("Скорость ветра на уровне 300 гПа, м","skor_veter_Sv_atm",300,"VeterSvAtm.aer300.lap300","VeterSvAtm.aer300.gradX300","VeterSvAtm.aer300.gradY300");
    h.addResultFunction("Направление ветра на уровне 300 гПа, градусы","napr_veter_Sv_atm",300,"VeterSvAtm.aer300.lap300","VeterSvAtm.aer300.gradX300","VeterSvAtm.aer300.gradY300");

    h.addResultFunction("Скорость ветра на уровне 200 гПа, м","skor_veter_Sv_atm",200,"VeterSvAtm.aer200.lap200","VeterSvAtm.aer200.gradX200","VeterSvAtm.aer200.gradY200");
    h.addResultFunction("Направление ветра на уровне 200 гПа, градусы","napr_veter_Sv_atm",200,"VeterSvAtm.aer200.lap200","VeterSvAtm.aer200.gradX200","VeterSvAtm.aer200.gradY200");

    h.addResultFunction("Скорость ветра на уровне 100 гПа, м","skor_veter_Sv_atm",100,"VeterSvAtm.aer100.lap100","VeterSvAtm.aer100.gradX100","VeterSvAtm.aer100.gradY100");
    h.addResultFunction("Направление ветра на уровне 100 гПа, градусы","napr_veter_Sv_atm",100,"VeterSvAtm.aer100.lap100","VeterSvAtm.aer100.gradX100","VeterSvAtm.aer100.gradY100");



}
