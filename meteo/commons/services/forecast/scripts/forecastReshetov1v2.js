function badvalue()
{
  return -9999;
}
function nodata()
{
  return "Нет данных";
}
function emptyvalue()
{
  return "Значение не рассчитано";
}

var speed = emptyvalue();

function Reshetov(T850,T700,T600,T500,P0,Tsost850,Tsost700,Tsost600,Tsost500, V0, V850, V700, V500, Tsmoch)
{
  if(!isValidNum(T850) ||
      !isValidNum(T700) ||
      !isValidNum(T600) ||
      !isValidNum(T500))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  if(!isValidNum(Tsost850)  ||
      !isValidNum(Tsost700) ||
      !isValidNum(Tsost600) ||
      !isValidNum(Tsost500) ||
      !isValidNum(P0) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  if(!isValidNum(V0)  ||
     !isValidNum(V850) ||
     !isValidNum(V700) ||
     !isValidNum(Tsmoch) ||
     !isValidNum(V500)  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var zonds;
  var Tsum  = (parseFloat(Tsost850) -parseFloat(T850)) + (parseFloat(Tsost700) -parseFloat(T700)) + (parseFloat(Tsost600) -parseFloat(T600)) + (parseFloat(Tsost500) -parseFloat(T500)); // сумма отклонений кривой стратификации от кривой состояния
  var T800 = parseFloat(T850) - 50*((parseFloat(T850)-parseFloat(T700))/150);// интерполируем температуру на кривой стратификации между поверхностями 850 и 700 гПа
  var Tmax = (parseFloat(T800)+273.15)*Math.pow((parseFloat(P0)/800.),0.286)-273.15; // расчет максимальной температуры у земли по сухоадиабатическому соотношению
  var dTmax500 = parseFloat(Tmax) - parseFloat(T500); // разность между максимальной температурой воздуха у земли и температурой на поверхности 500 гПа
  var L2 = 0.039*Tsum + 0.025*dTmax500 - 1.162; // дискриминантная функция
  if (L2 > 0)
  {
       // тогда рассчитываем максимальную скорость ветра при шквале
    var Tsm = zondObj.oprTsmUrzem0(obj.getStationData()); // нулевая температура смоченного термометра
    if( Tsm === badvalue())
    {
        Tsm = Tsmoch;
    }

    var dTmax = parseFloat(Tmax) - parseFloat(Tsm);

    var V = 0.25*(parseFloat(V0)+parseFloat(V850)+parseFloat(V700)+parseFloat(V500));
    var Vmax = 0.036*Math.pow((parseFloat(dTmax)+parseFloat(0.25*V)),2.07)+parseFloat(14.3);
    var bal = parseFloat(dTmax)+parseFloat(0.25*V);

    speed = Vmax;
    var dataB = 18;
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, dataB, "Шквалы", 0,1);
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11041, speed, "Максимальная скорость ветра при порыве", 0,1);
    return "Ожидается шквал";
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0, "Шквал не ожидается", 0,1);
  
  return "Шквал не ожидается";
}

function windSpeedMax()
{
  return speed;
}

function TsostOnLevel(level)
{
  if(!isValidNum(level))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  
  var t = zondObj.oprTsost(obj.getStationData(),level);
  
  return t;
}



function Tsmoch()
{
  
  var t = zondObj.oprTsmUrzem0(obj.getStationData());
  
  return t;
}

