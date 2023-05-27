//Фронтальный гололед (Р. А. Ягудин)
function GololedIntens(T850,T,H10,time) //T850 - температура воздуха на уровне 850 гПа, Ht10 - высоты изотермы -10 градусов и T - температурa воздуха у поверхности земли
{
//  return Ht10;
  if( !isValidNum( T850 )   ||
      !isValidNum( T ) ||
      !isValidNum( H10 ) )
    {
      return "Метод не работает с заданными значениями параметров";
    }

  var type_intens,gololed,ret_val,Ht10;
  var ret, ret2;

  var hour;
  if(time==6)
  {
    hour = "0;6";
  }
  if(time==12)
  {
    hour = "0;12";
  }
  if(time==18)
  {
    hour = "0;18";
  }

  T850=parseFloat(T850);
  T=parseFloat(T);

  Ht10=parseFloat(H10)/1000.;
  gololed=pic120(1,T850, Ht10, T);
 // print(T850, Ht10,H10, T,gololed);

  if( gololed == badvalue() )
  {
    return "Исходные данные вне области допустимых значений метода: температура воздуха на уровне 850 гПа от -9 до 0, температурa воздуха у поверхности земли от -8 до 2, высотa изотермы -10 C от 1 до 5км";
  }
  type_intens=pic120(2,T850, Ht10, T);
//  print(T850, Ht10,H10, T,type_intens,gololed);

//  return type_intens;

  if( type_intens == badvalue() )
  {
    return "Исходные данные вне области допустимых значений метода: температура воздуха на уровне 850 гПа от -9 до 0, температурa воздуха у поверхности земли от -8 до 2, высотa изотермы -10 C от 1 до 5км";
  }



  if(gololed==1)
  {
    ret = "Фронтальный гололед";
    ret2 = "Фронтальный гололед не ожидается";
    if(type_intens==0)
    {
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 227, ret, 0,1);
      return "Ожидается умеренный или сильный фронтальный гололёд";
    }else
    if(type_intens==1)
    {
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 227, ret, 0,1);
      return "Ожидается слабый фронтальный гололёд";
    }else
    if(type_intens==2)
    {
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 227, ret, 0,1);
      return "Ожидается фронтальный гололёд";
    }
  }
  else { dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 227, ret2, 0,1);
         return "Фронтальный гололёд не ожидается";}

//  return ret;



}

function H10height( srok )//в часах
{
  var Ht10;
  Ht10 = zondObj.opredHPoT(obj.getStationData(),-10.,srok); //в декаметрах???

  if( !isValidNum( Ht10 )   )
  {
    return "Не удалось получить данные вертикального зондирования";
  }
  return Ht10;
}


// Функция для расчета возможности образования фронтального гололёда по прогностическим данным на момент прохождения фронта: температуры воздуха на уровне 850 гПа, высоты изотермы -10 градусов и температуры воздуха у поверхности земли.

function pic120(tip, T850,  H,  Tz )// добавлено в данный проект ЛКА 12.03.2007
{

   T850 = parseFloat(T850);
   Tz = parseFloat(Tz);
   H = parseFloat(H);
   if(T850<-9. || T850>0. || Tz>2. || Tz<-8. || H<1. || H>5.)  //Znacheniya vne ODZ.
   {
   //  print(tip, T850,  H,  Tz );
     return badvalue();
   }

/////////////////////////
  var a=[0.15,-1.75,3.724,3.707,-4.483,4.77,1.468,0.686,-8.526];
  var b=[-0.686,24.95,-27.274,-21.316,24.215,-35.091,-9.412,-2.735,66.401];
  var c=[-6.828,-88.1,41.969,22.204,-31.7,60.821,11.855,0.666,-128.934];
  var y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,k,d;
  var gololed = 0, intense = 2;
   //levij grafik
  y1=(a[0]*T850*T850)+(b[0]*T850)+(c[0]);
  y2=(a[1]*T850*T850)+(b[1]*T850)+(c[1]);
  y3=1.0;

    //praviy grafik
  y4=(a[2]*H*H)+(b[2]*H)+(c[2]);
  y5=(a[3]*H*H)+(b[3]*H)+(c[3]);
  y6=(a[4]*H*H)+(b[4]*H)+(c[4]);
  y7=0.8;
  k=-2.22;
  d=9.68;
  y8=(k*H)+(d);
  y9=(a[5]*H*H)+(b[5]*H)+(c[5]);
  y10=(a[6]*H*H)+(b[6]*H)+(c[6]);
  y11=(a[7]*H*H)+(b[7]*H)+(c[7]);
  y12=(a[8]*H*H)+(b[8]*H)+(c[8]);



  if(T850>=-6.4 && T850<=0) //исправлено значение
  {
    if (Tz<y1 || Tz>y3) // Bez gololeda
    {
      gololed=0;// printf(" Bez gololeda\n\n");
    }
  }
  if(T850<-6.4 && Tz>=-2.)
  {
    if (Tz>y2) // Bez gololeda
    {
      gololed=0;//printf(" Bez gololeda\n\n");
    }
  }
  if(T850<-6.4 && Tz<-2.)
  {
    if (Tz<y1) // Bez gololeda
    {
      gololed=0;// printf(" Bez gololeda\n\n");
    }
  }
  if(T850>=-6.4 && T850<=0)  //Proverkaaaa!!!
  {
    if (Tz<=y3 && Tz>=y1) //Gololed
    {
      gololed=1;// printf("Gololed\n\n");
    }
  }
  if(T850<-6.4 && T850>=-8.4)
  {
    if (Tz<=y2 && Tz>=y1) //Gololed
    {
      gololed=1;// printf("Gololed\n\n");
    }
  }

/////////////////////////////////////////////////////////////////////////////////////////

  if ( (Tz<=-2. && Tz>=y1) || (Tz>-2. && Tz<=1. && T850>=-6.4) || (T850<-6.4 && Tz>-2. && Tz<=y2))
  {

    if (H>=3.35 && H<=4.36)//umerennij gololed
    {
      if (Tz<=y11 && Tz<=y12 && Tz>=y9)
      {
        intense=0;//printf("Intensivnosnj gololeda - umerennij gololed \n\n");
      }
    }
    if (H<3.35 && H>=2.29)//umerennij gololed
    {
      if (Tz<=y11 && Tz>=y10)
      {
        intense=0;//printf("Intensivnosnj gololeda - umerennij gololed \n\n");
      }
    }

    if (H>=4. && H<=4.36)//slabij gololed
    {
      if (Tz>=y4 && Tz<y9)
      {
        intense=1;//printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
    if (H>=4. && H<=4.36)//slabij gololed
    {
      if (Tz>y12 && Tz<=y8)
      {
        intense=1;//printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }

    if (H>4.36 && H<=5.)
    {
      if (Tz<=y8 && Tz>=y4)
      {
        intense=1;//printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }


    if (H<=3. && H>=1.71 && Tz<=-2)//slabij gololed
    {
      if (Tz>=y5 && Tz<y10)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
    if (H<=3. && H>=1.71 && Tz>-2)//slabij gololed
    {
      if (Tz>y11 && Tz<=y6)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
    if (H>3. && H<4.)//slabij gololed
    {
      if (Tz<=y7 && Tz>y12 && Tz>y11)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
    if (H>=3.35 && H<4.)//slabij gololed
    {
      if (Tz>=y4 && Tz<y9)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
    if (H<3.35 && H>3.)//slabij gololed
    {
      if (Tz>=y5 && Tz<y10)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
  }


  if( tip == 1)
  {
 //   print ("gololed",gololed);
    return gololed;
  }
  if( tip == 2)
  {
  //  print ("intense",intense);
    return intense;
  }

}


