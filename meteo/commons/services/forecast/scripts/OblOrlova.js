function name(){
  return "OblOrlova"
}

function icon()
{
  return ":/meteo/icons/geopixmap/osadki.png";
}

function title(){
  return "Обложные осадки (метод Е.М. Орловой)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;3;6;9;12;15;18;21";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "11;12";
}

function methodHelpFile()
{
  return "OrlovaObl.htm";
}

function typeYavl()
{
  return "1";
}

function init(haveUi){
    new TForecastOblOrlova(haveUi);
}

function TForecastOblOrlova(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastOblOrlova.prototype.initSrc = function(h) {
//D_850a,tau850_pr
  h.parent("В пункте прогноза ","insit");
  h.addFieldValue("Прогноз температуры воздуха на уровне земли,°C","T",12,0,1,"T12");
  h.addFieldValue("Прогноз температуры воздуха на уровне 850 гПа,°C","T",12,850,100,"T12850");
  h.addFieldValue("Прогноз температуры воздуха на уровне 700 гПа,°C","T",12,700,100,"T12700");
  h.addFieldValue("Прогноз геопотенциала на уровне 850 гПа","hh",12,850,100,"hh850");
  h.addFieldValue("Прогноз геопотенциала на уровне 1000 гПа","hh",12,1000,100,"hh1000");
  h.addFieldValue("Прогноз скорости вертикальных движений на уровне 850 гПа, гПа/12ч","ww2",12,850,100,"ww850");
  h.addFieldValue("Прогноз скорости вертикальных движений на уровне 700 гПа, гПа/12ч","ww2",12,700,100,"ww700");

  h.addField("Поле адвекции 700 гПа (12 ч.)","hh",0,700,100,-12,"700adv12");
    h.add("Дефицит точки росы на уровне 850 гПа,°C","D",850,100);

    h.addResultFunction("Наличие облачности","orlovaOblOsNal","OblOrlova.700adv12.D","OblOrlova.insit.ww850");
    h.addResultFunction("Фаза осадков","orlovaOblFaza","OblOrlova.hh850","OblOrlova.insit.hh1000","OblOrlova.insit.T12",
                        "OblOrlova.700adv12.D","OblOrlova.insit.ww850");
    h.addResultFunction("Интенсивность осадков, мм/ч","getOrlovaOblIntens","OblOrlova.insit.T12850","OblOrlova.insit.T12700","OblOrlova.insit.ww850",
                        "OblOrlova.insit.ww700","OblOrlova.700adv12.D");
//getOrlovaOblIntens(T_850pr,tau850_pr,tau700_pr,tau500_pr,D_850a)
}
