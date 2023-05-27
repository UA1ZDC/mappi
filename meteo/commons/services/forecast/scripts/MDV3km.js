function name(){
  return "MDV3km"
}

function icon()
{
  return ":/meteo/icons/geopixmap/eye.png";
}

function title(){
  return "Метеорологическая дальность видимости менее 3 км";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "Mdv3km.htm";
}

function typeYavl()
{
  return "8";
}

function init(haveUi){
    new TForecastMDV3km(haveUi);
}

function TForecastMDV3km(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();
  
}

TForecastMDV3km.prototype.initSrc = function(h) {
  h.parent("Погода в пункте прогноза","srf");
  h.addComboFunction("Синоптическая ситуация","getSynopFrSit");
    h.addComboFunction("Наличие осадков","getPrecipNal");
  h.parent("МДВ","inp");
  h.add("МДВ в момент прогноза, м","V",0,1);
  h.addSimple("МДВ за 15 мин до момента прогноза, м","Stm15",10000);
  h.addSimple("МДВ за 30 мин до момента прогноза, м","Stm30",10000);
  h.addSimple("МДВ за 45 мин до момента прогноза, м","Stm45",10000);
  
  h.addResultFunction("Значение МДВ (км) через 15 мин","getFormul_MDV15","MDV3km.srf.getSynopFrSit","MDV3km.srf.getPrecipNal","MDV3km.inp.V","MDV3km.inp.Stm15","MDV3km.inp.Stm30","MDV3km.inp.Stm45");
  h.addResultFunction("Значение МДВ (км) через 30 мин","getFormul_MDV30","MDV3km.srf.getSynopFrSit","MDV3km.srf.getPrecipNal","MDV3km.inp.V","MDV3km.inp.Stm15","MDV3km.inp.Stm30","MDV3km.inp.Stm45");
  h.addResultFunction("Значение МДВ (км) через 45 мин","getFormul_MDV45","MDV3km.srf.getSynopFrSit","MDV3km.srf.getPrecipNal","MDV3km.inp.V","MDV3km.inp.Stm15","MDV3km.inp.Stm30","MDV3km.inp.Stm45");
  h.addResultFunction("Значение МДВ (км) через 60 мин","getFormul_MDV60","MDV3km.srf.getSynopFrSit","MDV3km.srf.getPrecipNal","MDV3km.inp.V","MDV3km.inp.Stm15","MDV3km.inp.Stm30","MDV3km.inp.Stm45");
}
