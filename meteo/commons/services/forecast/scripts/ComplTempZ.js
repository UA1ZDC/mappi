function name(){
  return "ComplTempZ";
}

function icon()
{
  return ":/meteo/icons/geopixmap/temperatura.png";
}

function title(){
  return "Температура воздуха у поверхности земли (на высоте 2 м)";
}

function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;3;6;9;12;15;18;21";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "12;24";
}

function methodHelpFile()
{
  return "temperZ.htm";
}

function typeYavl()
{
  return "4";
}

function init(haveUi){
    new TForecastComplTempZ(haveUi);
}

function TForecastComplTempZ(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastComplTempZ.prototype.initSrc = function(h) {

    h.parent("В пункте прогноза","insit");
      h.addCombo("Тип подстилающей поверхности","tip1","Почва сухая;Почва влажная;Песок;Трава, сельскохозяйственные культуры;Лес лиственный;Лес хвойный;Снег талый;Снег свежий;Снег загрязненный;Вода");
      h.addFunction("Отклонение от суточного хода температуры (для прогноза на 12ч)","Tsudhod12",12);
      h.addFunction("Отклонение от суточного хода температуры (для прогноза на 24ч)","Tsudhod24",24);
      h.add("Температура,°C","T",0,1);
      h.add("Количество облачности, %","N",0,1);
      //h.addSimple("Количество облачности в баллах","N",0);

    h.addField("Точка адвекции 850 гПа (12 ч.)","hh",0,850,100,-12,"850adv12");
      //h.addFieldValue("Температура","T",0,0,1,"850adv_T0_12");
      //h.addFieldValue("Количество облачности, балл","N",0,0,200,"850adv_N0_12");
      h.add("Температура,°C","T",0,1);
      h.add("Количество облачности, %","N",0,1);
      //h.addSimple("Количество наблюдаемой облачности в баллах","850adv_N0_12",0);
      h.addCombo("Тип подстилающей поверхности","tip2","Почва сухая;Почва влажная;Песок;Трава, сельскохозяйственные культуры;Лес лиственный;Лес хвойный;Снег талый;Снег свежий;Снег загрязненный;Вода");

    h.addField("Точка адвекции 850 гПа (24 ч.)","hh",0,850,100,-24,"850adv24");
      h.add("Температура,°C","T",0,1);
      //h.addFieldValue("Температура","T",0,0,1,"850adv_T0_24");
      //h.addFieldValue("Количество облачности, балл","N",0,0,200,"850adv_N0_24");
      h.add("Количество облачности, %","N",0,1);
      h.add("Скорость ветра","ff",0,1);
      //h.addSimple("Количество наблюдаемой облачности в баллах","850adv_N0_24",0);
      //h.addCombo("Тип подстилающей поверхности","tip3","Почва сухая;Почва влажная;Песок;Трава, сельскохозяйственные культуры;Лес лиственный;Лес хвойный;Снег талый;Снег свежий;Снег загрязненный;Вода");



    h.parent("Дополнительные параметры","dop");
//    h.addFunction("Трансформационные изменения температуры воздуха (для прогноза на 12 ч)","transform",0,
//                "ComplTempZ.850adv12.850adv_T0_12","ComplTempZ.T","ComplTempZ.850adv12.850adv_N0_12","ComplTempZ.N","ComplTempZ.insit.tip1","ComplTempZ.850adv12.tip2");
//    h.addFunction("Трансформационные изменения температуры воздуха (для прогноза на 24 ч)","transform2",0,
//                "ComplTempZ.850adv24.850adv_T0_24","ComplTempZ.T","ComplTempZ.850adv24.850adv_N0_24","ComplTempZ.N","ComplTempZ.insit.tip1","ComplTempZ.850adv12.tip2");
  //  h.addFunction("Трансформационные изменения температуры воздуха (для прогноза на 12 ч)","transform",
  //              "ComplTempZ.850adv12.T","ComplTempZ.T","ComplTempZ.850adv12.N","ComplTempZ.N","ComplTempZ.insit.tip1","ComplTempZ.850adv12.tip2");
    h.addFunction("Трансформационные изменения температуры воздуха (для прогноза на 24 ч)","transform",
                "ComplTempZ.850adv24.T","ComplTempZ.T","ComplTempZ.850adv24.N","ComplTempZ.850adv24.ff","ComplTempZ.N","ComplTempZ.insit.tip1","ComplTempZ.850adv24.tip3");

    h.addResultFunction("Прогноз температуры воздуха у поверхности земли на 12 ч", "forcTemp",12,"ComplTempZ.850adv12.T","ComplTempZ.T","ComplTempZ.dop.transform","ComplTempZ.insit.Tsudhod12");
    h.addResultFunction("Прогноз температуры воздуха у поверхности земли на 24 ч", "forcTemp",24,"ComplTempZ.850adv24.T","ComplTempZ.T","ComplTempZ.dop.transform","ComplTempZ.insit.Tsudhod24");

}
