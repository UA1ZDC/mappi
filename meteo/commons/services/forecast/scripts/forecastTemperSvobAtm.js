//18.1 Тумпература в свободной атмосфере
function TempSvobAtmos12(per_goda,T925adv,T925,level)
{

    if (!isValidNum(T925adv) ||
        !isValidNum(T925))
    {
        return "Метод не работает с заданными значениями параметров";
    }
    var time  = dataObj.getHourData();

    var dTadv,Tpr925;
    var dTtr;//трансформационное изменение
    var ret;

    dTadv=T925adv-T925;
    if(per_goda=="Теплый")
    {
          if(time == 0 ){dTtr=-0.32*dTadv+0.4;} //если исходный срок прогноза – 00 ч
          else{dTtr=-0.32*dTadv-0.4;}//если исходный срок прогноза – 12 ч
    }
    if(per_goda=="Холодный")
    {
      if(pic59(dTadv)=="err1")
      {
        return "Исходные данные вне области допустимых значений метода: адвективное изменение температуры dTa<-14 или dTa>20.";
      }
      if(pic59(dTadv)==badvalue())
      {
        return "Метод не работает с заданными значениями параметров";
      }
      dTtr = pic59(dTadv);
    }
    if(per_goda=="500")
    {
      dTtr=0; //трансформационное изменение
    }



    Tpr925=parseFloat(T925adv) + parseFloat(dTtr);

    dataObj.savePrognoz(obj.getStationData(),name(),title(),"0;18", 12101, Tpr925.toFixed(2), Tpr925.toFixed(2), level,100);

    return Tpr925;
}

function TempSvobAtmos24(per_goda,T925adv,T925,level)
{
    if (!isValidNum(T925adv) ||
        !isValidNum(T925))
    {
        return "Метод не работает с заданными значениями параметров";
    }
    var dTadv,Tpr925;
    var dTtr;//трансформационное изменение
    var ret;
    dTadv=T925adv-T925;
//    return 5;
    if(per_goda=="Теплый")
    {
      dTtr=-0.44*dTadv; //трансформационное изменение
    }
    if(per_goda=="Холодный")
    {
      if(pic59(dTadv)=="err1")
      {
        return "Значения вне области допустимых значений метода: адвективное изменение температуры dTa<-14 или dTa>20.";
      }
      if(pic59(dTadv)==badvalue())
      {
        return "Метод не работает с заданными значениями параметров";
      }
      dTtr = pic59(dTadv);
    }
    if(per_goda=="500")
    {
      dTtr=0; //трансформационное изменение
    }


    Tpr925=parseFloat(T925adv)+parseFloat(dTtr);
    dataObj.savePrognoz(obj.getStationData(),name(),title(),"0;21", 12101, Tpr925.toFixed(2), Tpr925.toFixed(2), level,100);

    return Tpr925;
}

function pic59 ( TT )
{
  var a=[-0.0283,-0.016,0.011];
  var b=[-0.616,-0.326,-0.519];
  var c=[0.8812,-1.284,-1.452];

  var y1,y2,y3;

  if (TT<-14.|| TT>20.)
    {
//     return 1;
      return "err1";
    }
  if (TT<0. && TT>-14.)
    {
      y1=parseFloat(a[0]*TT*TT)+parseFloat(b[0]*TT)+parseFloat(c[0]);  return y1;
    }
  if (TT>=0. && TT<8.)
    {
      y2=parseFloat(a[1]*TT*TT)+parseFloat(b[1]*TT)+parseFloat(c[1]);  return y2;
    }
  if (TT>=8. && TT<=20.)
    {
      y3=parseFloat(a[2]*TT*TT)+parseFloat(b[2]*TT)+parseFloat(c[2]);  return y3;
    }
  else
  {
    return "Метод не работает с заданными значениями параметров";
  }
}



function TempSvobAtm_vishe500(T400adv)
{
  var Tpr400;
  if (!isValidNum(T400adv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  Tpr400 = T400adv;
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 12101, Tpr400.toFixed(2), Tpr400.toFixed(2), 400,100);

  return Tpr400;
}




