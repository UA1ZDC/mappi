//12.2 Внутримассовый гололед

function MassGololedIntens(T,T850adv,time) //адвективная т у землии на уровне 850 гПа
{

  if( !isValidNum( T )   ||
      !isValidNum( T850adv ) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  var Tcp;
  var ret;
  var hour;
  if(time==6)
  {
    hour = "0;6";
  }
  if(time==9)
  {
    hour = "0;9";
  }
  if(time==12)
  {
      hour = "0;12";
  }

  Tcp=(parseFloat(T)+parseFloat(T850adv))*0.5;
  if (Tcp>-8. && Tcp<4. && T<0.)
  {
    ret = "Внутримассовый гололед";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 227, ret,0,1);
  }
  else
  {
    ret = "Без внутримассового гололеда";
   // dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 227, ret,0,1);
  }
  return ret;

}

//12.3 Гололедица
function GololedicaNal(T0pr,Tdpr)
{
  var D850adv;  //прогностические значения T и D у поверхности земли
  var ret_val, rezPrognoz;
  var ret;

  ret_val=pic121(T0p,D850adv);
  if( ret_val===badvalue() )
  {
      return "Метод не работает с заданными значениями параметров";
  }

  if( ret_val=== "err1")
  {
      return "Исходные данные вне области допустимых значений метода: ожидаемое значение температуры воздуха T>5 или T<-25";
  }
  if( ret_val=== "err2")
  {
      return "Исходные данные вне области допустимых значений метода: ожидаемое значение дефицита точки росы D>8 или D<0";
  }
  if(ret_val===0)
  {
    return "Гололедица";
  }
  if(ret_val===1)
  {
    return "Гололедицы нет";
  }

//  if(ret_val==0)
//  {
//      if (rezPrognoz==0)      ret="Ожидается гололедица";
//      else      ret="Гололедица не ожидается";
//  }
  else {return "Гололедица не ожидается";}
}

//Функция для прогноза гололедицы по ожидаемым значениям температуры воздуха T и дефицита точки росы dT у поверхности земли. рис.121
function pic121( T,  d)
{
  if( !isValidNum( T )   ||
      !isValidNum( d ) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var a=[-0.108, -0.01];
  var b=[-0.546, -0.492];
  var c=[6.11, -3.12];
  var y2,y1;

  y1=a[0]*T*T+b[0]*T+c[0];
  y2=a[1]*T*T+b[1]*T+c[1];

  if(T>5. || T<-25. ) return "err1";//Vne oblasti ODZ!
  if(d>8. || d<0.) return "err2";//Vne oblasti ODZ!

  if(T>=-2.5 && T<=5.)
  {
    if(d<=y1)
    {
      return 0;//printf("\nGololedica\n\n");
    }
    if(d>y1)
    {
      return 1;//printf("\nGololedica otsutstvuet\n\n");
    }
  }
  if(T<-2.5 && T>=-25.)
  {
    if(d>=y2 && d<=7.)
    {
      return 0;//printf("\nGololedica\n\n");
    }
    else
    {
      return 1;//printf("\nGololedica otsutstvuet\n\n");
    }
  }

}

// 12.1 Фронтальный гололед (Р. А. Ягудин)
function GololedIntens(T850,Ht10,T)
{
  var type_intens,gololed,ret_val;
  var ret;
//  if(kolChiselVString(inStr)!=3)  {inStr=""; return false;}
//  sscanf(inStr.ascii(),"%f %f %f",&T850,&Ht10,&T);
  Ht10=Ht10/1000.;

  gololed=pic120(1,T850, Ht10, T);
  if( gololed === badvalue() )
  {
    return "Исходные данные вне области допустимых значений метода: T850 от 0 до 9, Та от -8 до 2, H10 от 1 до 5км";
  }
  type_intens=pic120(2,T850, Ht10, T);
  if( type_intens === badvalue() )
  {
    return "Исходные данные вне области допустимых значений метода: T850 от 0 до 9, Та от -8 до 2, H10 от 1 до 5км";
  }

  if(gololed===1)
  {
    if(type_intens===0)  {return "Ожидается умеренный фронтальный гололёд";}
    if(type_intens===1)  {return "Ожидается слабый фронтальный гололёд";}
    if(type_intens===2)  {return "Ожидается фронтальный гололёд";}
    else {return "Ожидается фронтальный гололёд";}
  }
  else {return "Фронтальный гололёд не ожидается";}


}

// Функция для расчета возможности образования фронтального гололёда по прогностическим данным на момент прохождения фронта: температуры воздуха на уровне 850 гПа, высоты изотермы -10 градусов и температуры воздуха у поверхности земли.

function pic120(tip, T850,  H,  Tz )// добавлено в данный проект ЛКА 12.03.2007
{
  if(T850<0 || T850>9 || Tz>2 || Tz<-8 || H<1 || H>5)  //Znacheniya vne ODZ.
  {
//    return 1;
    return "Метод не работает с заданными значениями параметров";
  }
/////////////////////////
  var a=[0.15,-1.75,3.724,3.707,-4.483,4.77,1.468,0.686,-8.526];
  var b=[-0.686,24.95,-27.274,-21.316,24.215,-35.091,-9.412,-2.735,66.401];
  var c=[-6.828,-88.1,41.969,22.204,-31.7,60.821,11.855,0.666,-128.934];
  var y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,k,d;
  var gololed, intense;
   //levij grafik
  y1=parseFloat(a[0]*T850*T850)+parseFloat(b[0]*T850)+parseFloat(c[0]);
  y2=parseFloat(a[1]*T850*T850)+parseFloat(b[1]*T850)+parseFloat(c[1]);
  y3=1.0;

    //praviy grafik
  y4=parseFloat(a[2]*H*H)+parseFloat(b[2]*H)+parseFloat(c[2]);
  y5=parseFloat(a[3]*H*H)+parseFloat(b[3]*H)+parseFloat(c[3]);
  y6=parseFloat(a[4]*H*H)+parseFloat(b[4]*H)+parseFloat(c[4]);
  y7=0.8;
  k=-2.22;
  d=9.68;
  y8=parseFloat(k*H)+parseFloat(d);
  y9=parseFloat(a[5]*H*H)+parseFloat(b[5]*H)+parseFloat(c[5]);
  y10=parseFloat(a[6]*H*H)+parseFloat(b[6]*H)+parseFloat(c[6]);
  y11=parseFloat(a[7]*H*H)+parseFloat(b[7]*H)+parseFloat(c[7]);
  y12=parseFloat(a[8]*H*H)+parseFloat(b[8]*H)+parseFloat(c[8]);


  if(T850<=6.4 && T850>=0)
  {
    if (Tz<y1 || Tz>y3) // Bez gololeda
    {
      gololed=0;// printf(" Bez gololeda\n\n");
    }
  }
  if(T850>6.4 && Tz>=-2)
  {
    if (Tz>y2) // Bez gololeda
    {
      gololed=0;//printf(" Bez gololeda\n\n");
    }
  }
  if(T850>6.4 && Tz<-2)
  {
    if (Tz<y1) // Bez gololeda
    {
      gololed=0;// printf(" Bez gololeda\n\n");
    }
  }


  if(T850<=6.4 && T850>=0)
  {
    if (Tz<=y3 && Tz>=y1) //Gololed
    {
      gololed=1;// printf("Gololed\n\n");
    }
  }
  if(T850>6.4 && T850<=8.4)
  {
    if (Tz<=y2 && Tz>=y1) //Gololed
    {
      gololed=1;// printf("Gololed\n\n");
    }
  }


  if (Tz<=-2. && Tz>=y1 || Tz>-2 && Tz<=1. && T850<=6.4 || T850>6.4 && Tz>-2 && Tz<=y2)
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

    if (H>4.36 && H<=5)
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
    if (H>3 && H<4)//slabij gololed
    {
      if (Tz<=y7 && Tz>y12 && Tz>y11)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
    if (H>=3.35 && H<4)//slabij gololed
    {
      if (Tz>=y4 && Tz<y9)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
    if (H<3.35 && H>3)//slabij gololed
    {
      if (Tz>=y5 && Tz<y10)
      {
        intense=1;// printf("Intensivnosnj gololeda - slabij gololed \n\n");
      }
    }
  }
  if( tip === 1)
  {
    return gololed;
  }
  if( tip === 2)
  {
    return intense;
  }

}



