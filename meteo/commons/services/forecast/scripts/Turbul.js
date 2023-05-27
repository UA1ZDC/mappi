function name(){
  return "Turbul"
}

function title(){
  return "Турбулентность ясного неба";
}

function icon()
{
  return ":/meteo/icons/geopixmap/boltanka2.png";
}


function times(){ //сроки запуска метода, час - время, за которое будет осуществляться отбор исходных данных для прогнозирования
  return "0;12";
}

function hours(){ //период действия прогноза (отсчитывается от times)
  return "0;24";
}

function methodHelpFile()
{
  return "turbul.htm";
}

function typeYavl()
{
  return "10";
}


function init(haveUi){
    new TForecastTurbul(haveUi);
}

function TForecastTurbul(haveUi){
  var h;
  h = new TForecastHeader(haveUi, title());this.initSrc(h);obj.reset();

}

TForecastTurbul.prototype.initSrc = function(h) {

   h.parent("На уровне 850 гПа","850hpa");
   h.add("Скорость ветра","ff",850,100);
   h.add("Высота уровня","hh",850,100);
   h.add("Значение u компоненты ветра","u" ,850,100);
   h.add("Значение v компоненты ветра","v", 850,100);
   h.addFieldAddValue("Горизонтальный градиент u по зональному направлению","u","gradx", 0,850,100,"gr");
   h.addFieldAddValue("Горизонтальный градиент u по меридиональному направлению","u","grady", 0,850,100,"gr1");
   h.addFieldAddValue("Горизонтальный градиент v по зональному направлению","v","gradx", 0,850,100,"gr2");
   h.addFieldAddValue("Горизонтальный градиент v по меридиональному направлению","v","grady", 0,850,100,"gr3");
   h.addFunction("Деформация ветра (DEF)","turb","Turbul.850hpa.gr","Turbul.850hpa.gr1","Turbul.850hpa.gr2","Turbul.850hpa.gr3");

   h.parent("На уровне 700 гПа","700hpa");
   h.add("Скорость ветра","ff",700,100);
   h.add("Высота уровня","hh",700,100);
   h.add("Значение u компоненты ветра","u",700,100);
   h.add("Значение v компоненты ветра","v",700,100);
   h.addFieldAddValue("Горизонтальный градиент u (по зональному направлению)","u","gradx", 0,700,100,"gr700");
   h.addFieldAddValue("Горизонтальный градиент u (по меридиональному направлению)","u","grady", 0,700,100,"gr1700");
   h.addFieldAddValue("Горизонтальный градиент v (по зональному направлению)","v","gradx", 0,700,100,"gr2700");
   h.addFieldAddValue("Горизонтальный градиент v (по меридиональному направлению)","v","grady", 0,700,100,"gr3700");

   h.addFunction("Вертикальный градиент u в слое 850-700 гПа","funcDx_u",
                 "Turbul.850hpa.u","Turbul.700hpa.u",
                 "Turbul.850hpa.hh","Turbul.700hpa.hh");
   h.addFunction("Вертикальный градиент v в слое 850-700 гПа","funcDx_v",
                 "Turbul.850hpa.v","Turbul.700hpa.v",
                 "Turbul.850hpa.hh","Turbul.700hpa.hh");

   h.addFunction("Деформация ветра (DEF)","turb","Turbul.700hpa.gr700","Turbul.700hpa.gr1700","Turbul.700hpa.gr2700","Turbul.700hpa.gr3700");
   h.addFunction("Вертикальный сдвиг ветра (VS)","vs","Turbul.700hpa.funcDx_u","Turbul.700hpa.funcDx_v");
   h.addFunction("Индекс турбулентности ясного неба (DVSI)","dvsi","Turbul.700hpa.turb","Turbul.700hpa.vs","Turbul.700hpa.ff");

   h.parent("На уровне 500 гПа","500hpa");
   h.add("Скорость ветра","ff",500,100);
   h.add("Высота уровня","hh",500,100);
   h.add("Значение u компоненты ветра","u" ,500,100);
   h.add("Значение v компоненты ветра","v", 500,100);
   h.addFieldAddValue("Горизонтальный градиент u (по зональному направлению)","u","gradx", 0,500,100,"gr500");
   h.addFieldAddValue("Горизонтальный градиент u (по меридиональному направлению)","u","grady", 0,500,100,"gr1500");
   h.addFieldAddValue("Горизонтальный градиент v (по зональному направлению)","v","gradx", 0,500,100,"gr2500");
   h.addFieldAddValue("Горизонтальный градиент v (по меридиональному направлению)","v","grady", 0,500,100,"gr3500");

   h.addFunction("Вертикальный градиент u в слое 700-500 гПа","funcDx_u",
                 "Turbul.700hpa.u","Turbul.500hpa.u",
                 "Turbul.700hpa.hh","Turbul.500hpa.hh");
   h.addFunction("Вертикальный градиент v в слое 700-500 гПа","funcDx_v",
                 "Turbul.700hpa.v","Turbul.500hpa.v",
                 "Turbul.700hpa.hh","Turbul.500hpa.hh");

   h.addFunction("Деформация ветра (DEF)","turb","Turbul.500hpa.gr500","Turbul.500hpa.gr1500","Turbul.500hpa.gr2500","Turbul.500hpa.gr3500");
   h.addFunction("Вертикальный сдвиг ветра (VS)","vs","Turbul.500hpa.funcDx_u","Turbul.500hpa.funcDx_v");
   h.addFunction("Индекс турбулентности ясного неба (DVSI)","dvsi","Turbul.500hpa.turb","Turbul.500hpa.vs","Turbul.500hpa.ff");

   h.parent("На уровне 400 гПа","400hpa");
   h.add("Скорость ветра","ff",400,100);
   h.add("Высота уровня","hh",400,100);
   h.add("Значение u компоненты ветра","u" ,400,100);
   h.add("Значение v компоненты ветра","v", 400,100);
   h.addFieldAddValue("Горизонтальный градиент u (по зональному направлению)","u","gradx", 0,400,100,"gr400");
   h.addFieldAddValue("Горизонтальный градиент u (по меридиональному направлению)","u","grady", 0,400,100,"gr1400");
   h.addFieldAddValue("Горизонтальный градиент v (по зональному направлению)","v","gradx", 0,400,100,"gr2400");
   h.addFieldAddValue("Горизонтальный градиент v (по меридиональному направлению)","v","grady", 0,400,100,"gr3400");

   h.addFunction("Вертикальный градиент u в слое 500-400 гПа","funcDx_u",
                 "Turbul.500hpa.u","Turbul.400hpa.u",
                 "Turbul.500hpa.hh","Turbul.400hpa.hh");
   h.addFunction("Вертикальный градиент v в слое 500-400 гПа","funcDx_v",
                 "Turbul.500hpa.v","Turbul.400hpa.v",
                 "Turbul.500hpa.hh","Turbul.400hpa.hh");

   h.addFunction("Деформация ветра (DEF)","turb","Turbul.400hpa.gr400","Turbul.400hpa.gr1400","Turbul.400hpa.gr2400","Turbul.400hpa.gr3400");
   h.addFunction("Вертикальный сдвиг ветра (VS)","vs","Turbul.400hpa.funcDx_u","Turbul.400hpa.funcDx_v");
   h.addFunction("Индекс турбулентности ясного неба (DVSI)","dvsi","Turbul.400hpa.turb","Turbul.400hpa.vs","Turbul.400hpa.ff");

   h.parent("На уровне 300 гПа","300hpa");
   h.add("Скорость ветра","ff",300,100);
   h.add("Высота уровня","hh",300,100);
   h.add("Значение u компоненты ветра","u" ,300,100);
   h.add("Значение v компоненты ветра","v", 300,100);
   h.addFieldAddValue("Горизонтальный градиент u (по зональному направлению)","u","gradx", 0,300,100,"gr300");
   h.addFieldAddValue("Горизонтальный градиент u (по меридиональному направлению)","u","grady", 0,300,100,"gr1300");
   h.addFieldAddValue("Горизонтальный градиент v (по зональному направлению)","v","gradx", 0,300,100,"gr2300");
   h.addFieldAddValue("Горизонтальный градиент v (по меридиональному направлению)","v","grady", 0,300,100,"gr3300");

   h.addFunction("Вертикальный градиент u в слое 400-300 гПа","funcDx_u",
                 "Turbul.400hpa.u","Turbul.300hpa.u",
                 "Turbul.400hpa.hh","Turbul.300hpa.hh");
   h.addFunction("Вертикальный градиент v в слое 400-300 гПа","funcDx_v",
                 "Turbul.400hpa.v","Turbul.300hpa.v",
                 "Turbul.400hpa.hh","Turbul.300hpa.hh");

   h.addFunction("Деформация ветра (DEF)","turb","Turbul.300hpa.gr300","Turbul.300hpa.gr1300","Turbul.300hpa.gr2300","Turbul.300hpa.gr3300");
   h.addFunction("Вертикальный сдвиг ветра (VS)","vs","Turbul.300hpa.funcDx_u","Turbul.300hpa.funcDx_v");
   h.addFunction("Индекс турбулентности ясного неба (DVSI)","dvsi","Turbul.300hpa.turb","Turbul.300hpa.vs","Turbul.300hpa.ff");

   h.parent("На уровне 200 гПа","200hpa");
   h.add("Скорость ветра","ff",200,100);
   h.add("Высота уровня","hh",200,100);
   h.add("Значение u компоненты ветра","u" ,200,100);
   h.add("Значение v компоненты ветра","v", 200,100);
   h.addFieldAddValue("Горизонтальный градиент u (по зональному направлению)","u","gradx", 0,200,100,"gr200");
   h.addFieldAddValue("Горизонтальный градиент u (по меридиональному направлению)","u","grady", 0,200,100,"gr1200");
   h.addFieldAddValue("Горизонтальный градиент v (по зональному направлению)","v","gradx", 0,200,100,"gr2200");
   h.addFieldAddValue("Горизонтальный градиент v (по меридиональному направлению)","v","grady", 0,200,100,"gr3200");

   h.addFunction("Вертикальный градиент u в слое 300-200 гПа","funcDx_u",
                 "Turbul.300hpa.u","Turbul.200hpa.u",
                 "Turbul.300hpa.hh","Turbul.200hpa.hh");
   h.addFunction("Вертикальный градиент v в слое 300-200 гПа","funcDx_v",
                 "Turbul.300hpa.v","Turbul.200hpa.v",
                 "Turbul.300hpa.hh","Turbul.200hpa.hh");

   h.addFunction("Деформация ветра (DEF)","turb","Turbul.200hpa.gr200","Turbul.200hpa.gr1200","Turbul.200hpa.gr2200","Turbul.200hpa.gr3200");
   h.addFunction("Вертикальный сдвиг ветра (VS)","vs","Turbul.200hpa.funcDx_u","Turbul.200hpa.funcDx_v");
   h.addFunction("Индекс турбулентности ясного неба (DVSI)","dvsi","Turbul.200hpa.turb","Turbul.200hpa.vs","Turbul.200hpa.ff");

   h.parent("На уровне 100 гПа","100hpa");
   h.add("Скорость ветра","ff",100,100);
   h.add("Высота уровня","hh",100,100);
   h.add("Значение u компоненты ветра","u" ,100,100);
   h.add("Значение v компоненты ветра","v", 100,100);
   h.addFieldAddValue("Горизонтальный градиент u (по зональному направлению)","u","gradx", 0,100,100,"gr100");
   h.addFieldAddValue("Горизонтальный градиент u (по меридиональному направлению)","u","grady", 0,100,100,"gr1100");
   h.addFieldAddValue("Горизонтальный градиент v (по зональному направлению)","v","gradx", 0,100,100,"gr2100");
   h.addFieldAddValue("Горизонтальный градиент v (по меридиональному направлению)","v","grady", 0,100,100,"gr3100");

   h.addFunction("Вертикальный градиент u в слое 200-100 гПа","funcDx_u",
                 "Turbul.200hpa.u","Turbul.100hpa.u",
                 "Turbul.200hpa.hh","Turbul.100hpa.hh");
   h.addFunction("Вертикальный градиент v в слое 200-100 гПа","funcDx_v",
                 "Turbul.200hpa.v","Turbul.100hpa.v",
                 "Turbul.200hpa.hh","Turbul.100hpa.hh");

   h.addFunction("Деформация ветра (DEF)","turb","Turbul.100hpa.gr100","Turbul.100hpa.gr1100","Turbul.100hpa.gr2100","Turbul.100hpa.gr3100");
   h.addFunction("Вертикальный сдвиг ветра (VS)","vs","Turbul.100hpa.funcDx_u","Turbul.100hpa.funcDx_v");
   h.addFunction("Индекс турбулентности ясного неба (DVSI)","dvsi","Turbul.100hpa.turb","Turbul.100hpa.vs","Turbul.100hpa.ff");

   h.parent("На уровне 70 гПа","70hpa");
   h.add("Скорость ветра","ff",70,100);
   h.add("Высота уровня","hh",70,100);
    h.add("Значение u компоненты ветра","u" ,70,100);
    h.add("Значение v компоненты ветра","v", 70,100);
    h.addFieldAddValue("Горизонтальный градиент u (по зональному направлению)","u","gradx", 0,70,100,"gr70");
    h.addFieldAddValue("Горизонтальный градиент u (по меридиональному направлению)","u","grady", 0,70,100,"gr170");
    h.addFieldAddValue("Горизонтальный градиент v (по зональному направлению)","v","gradx", 0,70,100,"gr270");
    h.addFieldAddValue("Горизонтальный градиент v (по меридиональному направлению)","v","grady", 0,70,100,"gr370");

    h.addFunction("Вертикальный градиент u в слое 100-70 гПа","funcDx_u",
                  "Turbul.100hpa.u","Turbul.70hpa.u",
                  "Turbul.100hpa.hh","Turbul.70hpa.hh");
    h.addFunction("Вертикальный градиент v в слое 100-70 гПа","funcDx_v",
                  "Turbul.100hpa.v","Turbul.70hpa.v",
                  "Turbul.100hpa.hh","Turbul.70hpa.hh");

    h.addFunction("Деформация ветра (DEF)","turb","Turbul.70hpa.gr70","Turbul.70hpa.gr170","Turbul.70hpa.gr270","Turbul.70hpa.gr370");
    h.addFunction("Вертикальный сдвиг ветра (VS)","vs","Turbul.70hpa.funcDx_u","Turbul.70hpa.funcDx_v");
    h.addFunction("Индекс турбулентности ясного неба (DVSI)","dvsi","Turbul.70hpa.turb","Turbul.70hpa.vs","Turbul.70hpa.ff");

    h.addResultFunction("В слое 850-700 гПа","layerRes","850","700","Turbul.850hpa.hh","Turbul.700hpa.hh","Turbul.700hpa.dvsi");
    h.addResultFunction("В слое 700-500 гПа","layerRes","700","500","Turbul.700hpa.hh","Turbul.500hpa.hh","Turbul.500hpa.dvsi");
    h.addResultFunction("В слое 500-400 гПа","layerRes","500","400","Turbul.500hpa.hh","Turbul.400hpa.hh","Turbul.400hpa.dvsi");
    h.addResultFunction("В слое 400-300 гПа","layerRes","400","300","Turbul.400hpa.hh","Turbul.300hpa.hh","Turbul.300hpa.dvsi");
    h.addResultFunction("В слое 300-200 гПа","layerRes","300","200","Turbul.300hpa.hh","Turbul.200hpa.hh","Turbul.200hpa.dvsi");
    h.addResultFunction("В слое 200-100 гПа","layerRes","200","100","Turbul.200hpa.hh","Turbul.100hpa.hh","Turbul.100hpa.dvsi");
    h.addResultFunction("В слое 100-70 гПа","layerRes","100","70","Turbul.100hpa.hh","Turbul.70hpa.hh","Turbul.70hpa.dvsi");


}
