function V_Progn_Ilina(ff_max,ff_adv)
{
  if( !isValidNum( ff_max )   ||
      !isValidNum( ff_adv )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var wstr, V1_V2;
  V1_V2=parseFloat(ff_max)-parseFloat(ff_adv);
  wstr = pic9_21(ff_max, V1_V2);
  if( wstr == "err1")
  {
    return "Исходные данные вне области допустимых значений метода: максимальная скорость в пункте прогноза меньше 0 или больше 300";
  }
  if( wstr == "err2")
  {
    return "Исходные данные вне области допустимых значений метода: разность скоростей меньше -200 или больше 200";
  }

  return wstr;
}

//Функция pic9.21 для прогноза скорости ветра Vp в струйном течении на 24 часа по значениям //скорости ветра в пункте прогноза V1 и в точке, удаленной от пункта прогноза на 1000 км //против потока V2 (рис. 9.21).Vv=V1-V2.
function pic9_21( V,  VV )
{
  var k=[1.19,1.125,1.129,1.16,1.1,1.1,1.1,1.074,1.08,1.09,1.09];
  var b=[-73.8,-102.5,-132.59,-163.2,-182.1,-182.1,-182.1,-232.2,-254.,-276.09,-298.9];

  var y60,y80,y100,y120,y140,y160,y180,y200,y220,y240,y260;
  V=V*3.6;
  VV=VV*3.6;
  if (V<0. || V>300.)
  {
    return "err1";
  }
  if (VV<-200. || VV>200.)
  {
    return "err2";
  }

  y60=parseFloat(k[0]*V)+parseFloat(b[0]);
  y80=parseFloat(k[1]*V)+parseFloat(b[1]);
  y100=parseFloat(k[2]*V)+parseFloat(b[2]);
  y120=parseFloat(k[3]*V)+parseFloat(b[3]);
  y140=parseFloat(k[4]*V)+parseFloat(b[4]);
  y160=parseFloat(k[5]*V)+parseFloat(b[5]);
  y180=parseFloat(k[6]*V)+parseFloat(b[6]);
  y200=parseFloat(k[7]*V)+parseFloat(b[7]);
  y220=parseFloat(k[8]*V)+parseFloat(b[8]);
  y240=parseFloat(k[9]*V)+parseFloat(b[9]);
  y260=parseFloat(k[10]*V)+parseFloat(b[10]);

  var wstr;
  if (VV>y60)
  {
      wstr = 60/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "Менее 60 км/ч";
  }
  if (VV<=y60 && VV>y80)
  {
      wstr = 80/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "60-80 км/ч";
  }

  if (VV<=y80 && VV>y100)
  {
      wstr = 100/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "80-100 км/ч";
  }
  if (VV<=y100 && VV>y120)
  {
      wstr = 120/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "100-120 км/ч";
  }

  if (VV<=y120 && VV>y140)
  {
      wstr = 140/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "120-140 км/ч";
  }

  if (VV<=y140 && VV>y160)
  {
      wstr = 160/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "140-160 км/ч";
  }

  if (VV<=y160 && VV>y180)
  {
      wstr = 180/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "160-180 км/ч";
  }

  if (VV<=y180 && VV>y200)
  {
      wstr = 200/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "180-200 км/ч";
  }

  if (VV<=y200 && VV>y220)
  {
      wstr = 220/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "200-220 км/ч";
  }

  if (VV<=y220 && VV>y240)
  {
      wstr = 240/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "220-240 км/ч";
  }

  if (VV<=y240 && VV>y260)
  {
      wstr = 260/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "240-260 км/ч";
  }

  if (VV<=y260)
  {
      wstr = 260/3.6;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, wstr.toFixed(2), wstr.toFixed(2), 0,200);
      return "Более 260 км/ч";
  }
  else return badvalue();
}


//возвращает максимальный ветер в пункте станции
function veterData()
{
  var v;
  v = zondObj.oprHmaxVFastV(obj.getStationData(),0);//максимальный ветер
  if(!isValidNum(v))
  {
    return "Не удалось получить значение скорости ветра автоматически";
  }
  return v/3.6;
}



function PveterData()
{
  var v,h,t,P;
  h = zondObj.oprHmaxVFastH(obj.getStationData(),0);//высота уровня максимального ветра
  if(!isValidNum(h))
  {
    return "Не удалось получить высоту максимальной скорости ветра автоматически";
  }
  P = zondObj.getDataFromHeight(obj.getStationData(),h*1000.,0,0);//давление уровня максимального ветра
  return P;

}

function NearLevel(p)
{
  if(!isValidNum(p))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var lp=[700,500,300,250,200,150,100];

  if(p<=100){
    return 100;
  }
  if(p>=700){
    return 700;
  }
  var min_dist= 90000;
  var min_i = 0;
  for(i=0;i<7;i++)//для каждого слоя
  {
    var he = Math.abs(p-lp[i]);
    if(min_dist > he)
    {
      min_dist = he;
      min_i = i;
    }
  }
  return lp[min_i];
}


function TveterDate(v)
{
   if(!isValidNum(v))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  t = 1000./v/3.6;//время адвекции
  return ftoi_norm(t);

}




