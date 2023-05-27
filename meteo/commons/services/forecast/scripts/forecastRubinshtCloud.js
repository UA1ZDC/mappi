
function getWindVelocityFromZond() // Функция возвращает скорость ветра на уровне 925 гПа
{
  var ff;
  //zondObj.resetData();
  ff = zondObj.getDataFromHeight(obj.getStationData(),600,0, 6);

  if(!isValidNum( ff ) )
  {
    return "Не удалось получить скорость ветра автоматически";
  }

  return ff;

}

function getWindVelocityFromZond2() // Функция возвращает скорость ветра на уровне 925 гПа
{
  var ff;
  //zondObj.resetData();
  ff = zondObj.getDataFromP(obj.getStationData(),925,0, 6);

  if(!isValidNum( ff ) )
  {
    return "Не удалось получить скорость ветра автоматически";
  }

  return ff;

}


function TOsobPoint() //Функция возвращает значение температуры на уровне первой особой точки по температуре
{
  var T;
  //zondObj.resetData();
  T = zondObj.getUrOsobPoTemp(obj.getStationData(),0,2);
  if(!isValidNum( T ) )
  {
    return "Не удалось получить температуру автоматически";
  }
  return T;
}

function DOsobPoint() //Функция возвращает дефицит точки росы на уровне первой особой точки по температуре
{
  var Td, T, D;
  //zondObj.resetData();
  Td = zondObj.getUrOsobPoTemp(obj.getStationData(),0,3);
  T = TOsobPoint();
  if(!isValidNum( T ) ||
    !isValidNum( Td ) )
  {
    return "Не удалось получить дефицит точки росы автоматически";
  }
  D = T - Td;
  return D;

}

function TUrZeml() // Функция возвращает температуру на уровне земли (по данным радиозонда)
{
  var T;
  //zondObj.resetData();
  T = zondObj.getZondValuePoUrType(obj.getStationData(),1, 0, 2);

  if(!isValidNum( T ))
  {
    return "Не удалось получить температуру автоматически";
  }
  return T;
}

function DUrZeml() //Функция возвращает дефицит точки росы на уровне земли (по данным радиозонда)
{
  var T, Td, D ;
  //zondObj.resetData();
  Td = zondObj.getZondValuePoUrType(obj.getStationData(),1, 0, 3);
//  T =  TUrZeml();

  if(!isValidNum( T ) ||
    !isValidNum( Td ) )
  {
    return "Не удалось получить дефицит точки росы автоматически";
  }

  D = T -Td;
  return D;
}



function getHpoint() //Функция возвращает высоту первой особой точки по температуре
{
  var Hpoint, H;
  //zondObj.resetData();
  Hpoint = zondObj.getUrOsobPoTemp(obj.getStationData(),0,1);

  if(!isValidNum( Hpoint ) )
  {
    return "Не удалось получить высоту особой точки автоматически";
  }
  return Hpoint;
}

function gradientT(T0,Tpoint,Hpoint) //Функция возвращает вертикальный градиент температуры в слое от поверхности земли до первой особой точки по температуре
{
   var grad, coef;


   if( !isValidNum( T0) ||
       !isValidNum( Tpoint) ||
       !isValidNum( Hpoint) )
   {
    return "Метод не работает с заданными значениями";
   }


  grad = 100.*(T0 - Tpoint)/Hpoint;

  if( !isValidNum( grad)  )
  {
    return "Не удалось рассчитать вертикальный градиент тиемпературы автоматически";
  }
  return grad;
}

function NalOblRubinsh( ff0,ff925,Dpoint,D0,VgradT )
{
  var ret;
  //ff0    - скорость ветра на уровне станции,
  //ff925 - скорость ветра на уровне 925 гПа по станции
  //ff925_field - скорость ветра на уровне 925 гПа по полю
  //T      - температура на уровне станции
  //Tpoint - температура на высоте первой особой точки по температуре
  //Tdpoint- температура точки росы на высоте первой особой точки
  //VgradT - вертикальный градиент температуры в слое от поверхности земли до первой особой точки
  //D0     - дефицит точки росы у поверхности земли
  //Dpoint - дефицит точки росы на высоте первой особой точки  (DD1Temp)

  //Примечание: вместо данных из особой точки используются данные на поверхности 925 гПа
  //заблаговременность прогноза - 3-6 часов

  if( !isValidNum( ff0) ||
    !isValidNum(  Dpoint ) ||
    !isValidNum( ff925) ||
    !isValidNum( D0) ||
    !isValidNum( VgradT))
  {
    return "Метод не работает с заданными значениями";
  }

  var df;
  var Tdpoint;

//   = DUrZeml();

//   = gradientT();

  df = ff925-ff0;


  if (df<=6.667 && VgradT>=-0.353 && D0<=1.411 && Dpoint<=1.557)
  {
    ret="Облачность ниже 100м образуется (сохраняется)";

    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 1, ret, 0,200);

  }
  else
  {
    ret="Облачность ниже 100м не образуется (исчезнет)";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 3, ret, 0,200);

//    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20137, ret, ret, 0,1);

  }
  //TODO 20010 или "10002"?
  return ret;
}
