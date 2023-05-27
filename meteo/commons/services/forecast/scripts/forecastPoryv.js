
    //  var Vmax;// максимальная скорость ветра от земли до высоты 2 км (м/с)
    //  var dh;//толщина слоя (суммарная толщина слоев) в пределах которых скорость ветра превышает 15 м/с
    //при Vmax<30 из dh слои изотермий и инверсий исключают

function getA(Vmax,dh,grad)
{
  if(!isValidNum(Vmax) ||
     !isValidNum(grad) ||
     !isValidNum(dh) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var res;
  var val_res = 0;

  if(grad <= -0.4 ||grad > 0.9  )
  {
    val_res = -1;
    res = "Градиент температуры вне области значений: -0.3<=grad<=0.9 ";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11041, val_res, res, 0,1);
    return res;
  }


  if(Vmax< 15 ){
    res = "Порывы ветра не ожидаются";
  } else {
    var A;
    A = Vmax*dh/1000.;
    val_res = graphic(A,grad);
    if(val_res != badvalue()){
      val_res = Math.round( val_res );
    }
    res = "Ожидаются порывы ветра "+ val_res +" м/с";
  }
  if(val_res != badvalue()){
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11041, val_res, res, 0,1);

  }

  return res;
}

function gradientT(temp1,forctemp,ur)
{
//  var ur = zondObj.oprHmaxVFastV();

//  var forctemp = getDataFromHeight(ur,12,2);
  if(!isValidNum(temp1) ||
     !isValidNum(forctemp) ||
     !isValidNum(ur)|| (ur === 0))
  {
    return "Метод не работает с заданными значениями параметров";
  }
//  print(temp1,forctemp,ur);
  var grad = (parseFloat(temp1) - parseFloat(forctemp))/ur*100.;

  return grad;

}


function urovMaxWind(srok)
{
  if(!isValidNum(srok) || srok<0 )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var ur =  zondObj.oprHmaxWind(obj.getStationData(),0,2000,srok);
  // print(ur);
  if(!isValidNum(ur) )
  {
    return "Нет данных";
  }
  return ur;
}

function tempUrov(ur)
{
  if(!isValidNum(ur) )
  {
    return "Метод не работает с заданными значениями уровня";
  }
  ur = parseFloat(ur);

  var forctemp = zondObj.getDataFromHeight(obj.getStationData(),ur,12,2);

//  print(forctemp);
  if(!isValidNum(forctemp) )
  {
    return "Метод не работает с заданными значениями температуры";
  }

  return forctemp;


}



function maxspeed()
{
//    максимальная скорость ветра от земли до высоты 2 км (м/с)
  var ff;
  ff = zondObj.oprMaxWind(obj.getStationData(),0,2000,0);
  return ff;
}

function hmaxspeed()
{
  return zondObj.oprHmaxWind(obj.getStationData(),0,2000,0);
}


function dh(maxWind)
{

  if(!isValidNum(maxWind) )
  {
    return "Метод не работает с заданными значениями максимального ветра";
  }
  var predel = 15;
  if(maxWind < predel){
    return 0;
  }
  var ddh = zondObj.oprHSloyWind(obj.getStationData(),predel,0,2000,0);
  return ddh;
}


function graphic(A,grad)//скорость максимальных порывов ветра
{
  // A - комплексный показатель
  //grad - прогностический вертикальный градиент температуры
  //в слое от земли до уровня максимального ветра
  if(!isValidNum(A) ||
     !isValidNum(grad) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  var res = badvalue();
  if(grad <= 0.9 && grad > 0.8)
  {
    res = (((parseFloat(A)-0)*(parseFloat(25.8)-12.))/(parseFloat(68.)-0.))+parseFloat(12.);

  }
  if(grad <= 0.8 && grad > 0.7)
  {
    res = (((parseFloat(A)-0)*(parseFloat(25.)-parseFloat(11.7)))/(parseFloat(66.)-0))+parseFloat(11.7);
  }
  if(grad <= 0.7 && grad > 0.6)
  {
    res = (((parseFloat(A)-0)*(parseFloat(23.)-parseFloat(11.)))/(parseFloat(62.)-parseFloat(0.)))+parseFloat(11.);
  }
  if(grad <= 0.6 && grad > 0.5)
  {
    res = (((parseFloat(A)-0)*(parseFloat(22.)-parseFloat(10.8)))/(parseFloat(66.)-parseFloat(0)))+parseFloat(10.8);
  }
  if(grad <= 0.5 && grad > 0.4)
  {
    res = (((parseFloat(A)-0)*(parseFloat(20.2)-parseFloat(10.2)))/(parseFloat(66.)-parseFloat(0)))+parseFloat(10.2);
  }
  if(grad <= 0.4 && grad > 0.3)
  {
    res = (((parseFloat(A)-0)*(parseFloat(20.2)-parseFloat(10.)))/(parseFloat(66.)-0))+parseFloat(10.);
  }
  if(grad <= 0.3 && grad > 0.2)
  {
    res = (((parseFloat(A)-0)*(parseFloat(18.1)-parseFloat(9.8)))/(parseFloat(62.)-0))+parseFloat(9.8);
  }
  if(grad <= 0.2 && grad > 0.1)
  {
    res = (((parseFloat(A)-0)*(parseFloat(12.3)-parseFloat(9.)))/(parseFloat(66.)-0))+parseFloat(9.);
  }
  if(grad <= 0.1 && grad > 0.0)
  {
    res = (((parseFloat(A)-0)*(parseFloat(16)-parseFloat(8.6)))/(parseFloat(62)-0))+parseFloat(8.6);

  }
  if(grad <= 0.0 && grad > -0.1)
  {
    res = (((parseFloat(A)-0)*(parseFloat(15.)-parseFloat(8.)))/(parseFloat(66.)-0))+parseFloat(8.);

  }
  if(grad <= -0.1 && grad > -0.2)
  {
    res = (((parseFloat(A)-5.)*(parseFloat(13.6)-0))/(parseFloat(62.)-parseFloat(5.)));

  }
  if(grad <= -0.2 && grad > -0.3)
  {
    res = (((parseFloat(A)-10)*(parseFloat(12.5)-0))/(parseFloat(66)-parseFloat(10)));

  }
  if(grad <= -0.3 && grad > -0.4)
  {
    res = (((parseFloat(A)-15.)*(parseFloat(11.)-0))/(parseFloat(62.)-parseFloat(15.)));

  }
  if(grad <= -0.4 ||grad > 0.9  )
  {
    res = -1;
  }
//11112
  return res;

}

