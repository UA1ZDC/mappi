
function forcTemp(zabl, Ta, T0, Ttr,Tsut)
{
  var dTa = 0.;
  var Tpr = 0.;
    if( !isValidNum(Ta)  ||
        !isValidNum(T0) ||
        !isValidNum(zabl) ||
        !isValidNum(Ttr) ||
        !isValidNum(Tsut) )
    {
      return "Метод не работает с заданными значениями параметров";
    }

  // var Tpr=0., dTa=0.;
  if( zabl < 24.)  {
     dTa = parseFloat(Ta) - parseFloat(T0);
     Tpr = parseFloat(Ta) - parseFloat(0.2*dTa) + parseFloat(Tsut);
     dataObj.savePrognoz(obj.getStationData(),name(),title(),"0;12", 12101, Tpr.toFixed(2), Tpr.toFixed(2), 0,1);
     return Tpr; //Тa - температура в точке адвекции, dTa - адвективное изменение темпрературы (адв минус пункт прогн)
  }

  Tpr = parseFloat(Ta) + parseFloat(Ttr) + parseFloat(Tsut);
  dataObj.savePrognoz(obj.getStationData(),name(),title(),"0;24", 12101, Tpr.toFixed(2), Tpr.toFixed(2), 0,1);
  return Tpr;

}

function transform(Ta,T0,N0,N,ff,tip1,tip2)
{

    if(!isValidNum(Ta)  ||
       !isValidNum(T0) ||
       !isValidNum(N0)  ||
       !isValidNum(ff)  ||
       !isValidNum(N) )
    {
      return "Метод не работает с заданными значениями параметров";
    }

  var dTn;
  var dTa;
  var dN;
  var dTD;
  var alfa;
  var alfa0;
  dTa  = parseFloat(Ta) - parseFloat(T0);
  dN = parseFloat(N) - parseFloat(N0);//разность (в баллах) между ожидаемым количеством облаков
  dN = ftoi_norm(dN/10);
  //нижнего яруса в пункте, для которого составляется прогноз (N), и фактическим количеством
  //облаков в исходный срок в районе, откуда перемещается воздушная частица (N0)
  alfa = albedo(tip1);
  alfa0 = albedo(tip2);
  var time  = dataObj.getHourData();

  if( time == 0 ) //исходный срок прогноза - ночь
  {
    dTn = parseFloat(-0.2*dTa) + parseFloat(0.4*dN);//dTa - advective, dN - also
  } else

  {
    var month  = dataObj.getMonth();
    if( month == badvalue() ) { return "Метод не работает с заданными значениями параметров";}
    var k = coefK(ff);
    var A = a_and_b( 1,month )*k;
    var B = a_and_b( 2,month )*k;
    var dalfa = parseFloat(alfa) - parseFloat(alfa0);//разность между значениями альбедо в пункте,
     //для которого составляется прогноз (a), и в районе, откуда ожидается перемещение
    //воздушной частицы
    dTn = parseFloat(-0.2*dTa)-parseFloat(A*dN)-parseFloat(B*dalfa);
  }
  return dTn;
}

function albedo(tip)
{
  var albedo;
    if(tip == "Почва сухая") albedo =  0.17;
    if(tip == "Почва влажная") albedo = 0.11;
    if(tip == "Песок") albedo = 0.35;
    if(tip == "Трава, сельскохозяйственные культуры")albedo = 0.20;
    if(tip == "Лес лиственный")albedo = 0.18;
    if(tip == "Лес хвойный") albedo = 0.14;
    if(tip == "Снег талый") albedo = 0.60;
    if(tip == "Снег свежий")albedo =  0.85;
    if(tip == "Снег загрязненный") albedo  = 0.45;
    if(tip == "Вода") albedo = 0.22;
  return albedo;
}

function coefK(ff)
{
  var k = 1;
   if(ff > 6 && ff <=8 ){
        k = 0.9;
  }
   if(ff > 8 && ff <= 11 ){
         k = 0.8;
   }
   if(ff > 11 && ff <= 14 ){
      k = 0.65;
   }
   if(ff > 14 ){
      k = 0.5;
   }
 return k;
}

function a_and_b(tip,month)
{
  var A, B;
  if( month == 1)
  {
    A= -0.4;
    B= 4;
  }
  if( month == 2)
  {
    A= -0.3;
    B= 7;
  }
  if( month == 3)
  {
    A= 0;
    B= 12;
  }
  if( month == 4)
  {
    A= 0.4;
    B= 16;
  }
  if( month == 5)
  {
    A= 1;
    B= 18;
  }
  if( month == 6)
  {
    A= 1.2;
    B= 20;
  }
  if( month == 7)
  {
    A= 1.1;
    B= 19;
  }
  if( month == 8)
  {
    A= 0.9;
    B= 17;
  }
  if( month == 9)
  {
    A= 0.6;
    B= 13;
  }
  if( month == 10)
  {
    A= 0.2;
    B= 9;
  }
  if( month == 11)
  {
    A= 0.1;
    B= 5;
  }
  if( month == 12)
  {
    A= -0.4;
    B= 3;
  }


  if(tip==1)
  {
    return A;
  }
  if(tip==2)
  {
    return B;
  }
  else
  {
    return "Метод не работает с заданными значениями параметров";
  }
}

function Tsudhod(zabl)//исходный (начальный) момент времени и момент, на который составляется прогноз
{
  var Tn, Tend;
//  Tn = dataObj.getSutHod(obj.getStationData(), month,begin);
//  Tend = dataObj.getSutHod(obj.getStationData(), month,end);

    var month  = dataObj.getMonth();


    Tn = dataObj.getSutHod(obj.getStationData(), month,0);

    if(zabl==12)
    {
      Tend = dataObj.getSutHod(obj.getStationData(), month,12);
    }
    else
    {
      Tend = dataObj.getSutHod(obj.getStationData(), month,23);
    }


    if( Tn == badvalue() ||
        Tend == badvalue()  ) //исходный срок прогноза - ночь
    {
      return "Метод не работает с заданными значениями параметров";
    }


//   return Tend;
   var result = parseFloat(-Tn) + parseFloat(Tend);

   return result;

}



