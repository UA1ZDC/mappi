//для метода 3.4-(13) Наклоная дальность видимости
//входные данные ВНГО, МДВ, Кол-во обл-ти

//typeAvia = 1;//истребительно-бомбардировочная
//typeAvia = 2;//дальняя
//typeAvia = 3;//военно-транспортная
//typeAvia = 4;//вертолеты

//typeYavl = 0;//без явлений погоды
//typeYavl = 1;//дымка
//typeYavl = 2;//дождь
//typeYavl = 3;//снег
//typeYavl = 4;//морось



function getYavlAll(VNGO_f,MDV_f,N_f,typeAvia,typeYavl)
{
  if( !isValidNum( VNGO_f ) ||
      !isValidNum( MDV_f )  ||
      !isValidNum( N_f )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var ret_val=0,ndvF=0;
  var ndvK=0,hsam=0,ndv=0;
  var N=0;
  if( N_f > 10)
  {
    N_f = N_f/10.;
  }
  

  if(VNGO_f<=10) {VNGO_f=VNGO_f*1000.;}
  if(MDV_f>10){MDV_f=MDV_f/1000.;}

  

  if ( typeAvia=="Истребительно-бомбардировочная" )
    {
      if((VNGO_f<140.||VNGO_f>650.)||(MDV_f<2.||MDV_f>9.))
        {return "Для истребительно-бомб. авиации допустимые данные: ВНГО от 140м до 650м, МДВ от 2км до 9км";}
  //    else oks=TRUE;
    } else
  if ( typeAvia=="Дальняя" )
    {

       if((VNGO_f<50.||VNGO_f>2000.)||(MDV_f<2.||MDV_f>9.))
         {return "Для дальней авиации допустимые данные: ВНГО от 50м до 2000м, МДВ от 2 до 9км";}
//     else oks=TRUE;
    }else
  if ( typeAvia=="Военно-транспортная") 
    {

      if((VNGO_f<100.||VNGO_f>2000.)||(MDV_f<1.||MDV_f>8.))
        {return "Для военно-транспортной допустимые данные: ВНГО от 100м до 2000м, МДВ от 1000м до 8км";}
//    else oks=TRUE;
    } else
  if ( typeAvia=="Вертолеты")
    {
      if((VNGO_f<50.||VNGO_f>300.)||(MDV_f<0.5||MDV_f>5.)||(N_f>10||N_f<1))
        {return "Для вертолетов допустимые данные: ВНГО от 50м до 300м, МДВ от 500м до 5км, облачность от 1 до 10 б.";}
//    else oks=TRUE;
    } else
    {
      return "Метод не работает с заданными значениями параметров";
    }
  

  if ( typeAvia=="Истребительно-бомбардировочная"){
      ret_val=pic115(VNGO_f,MDV_f,typeYavl);
      if (ret_val == "err1")
      {
        return "Исходные данные вне области допустимых значений метода: ВНГО Hp<50 или Hp>650";
      }else
      if (ret_val == "err2")
      {
        return "Исходные данные вне области допустимых значений метода: горизонтальная видимость у поверхности земли Sm<2 или Sm>9";
      }else
      if (ret_val == "err3")
      {
        return "Тип явления вне области допустимых значений метода";
      }else
      if( ret_val == badvalue())
      {
        return "Метод не работает с заданными значениями параметров";
      }else{
        return ftoi_norm(ret_val*1000);
      }
    } else
  if ( typeAvia=="Дальняя"){
      ret_val=pic116(MDV_f,VNGO_f,typeYavl);

      if( ret_val == "err1")
      {
        return "Исходные данные вне области допустимых значений метода: НГО H<50";
      }else
      if( ret_val == "err2")
      {
        return "Исходные данные вне области допустимых значений метода: горизонтальная дальность видимости Sm<2 или Sm>9";
      }else
      if( !isValidNum(ret_val) )
      {
        return "Метод не работает с заданными значениями параметров";
      }
      return ftoi_norm(ret_val*1000);
  }else
  if ( typeAvia=="Военно-транспортная"){
      ret_val=pic117(MDV_f,VNGO_f);
      if( !isValidNum(ret_val) )
      {
        return "Метод не работает с заданными значениями параметров";
      }else
      if( ret_val == "err1")
      {
        return "Исходные данные вне области допустимых значений метода: посадочная видимость менее 1 км!";
      }else
      if( ret_val == "err2")
      {
        return "Исходные данные вне области допустимых значений метода: посадочная видимость более 8 км!";
      }
      else
      {
        if((typeYavl=="Морось")||(typeYavl=="Снег")||(typeYavl=="Дымка")||(typeYavl=="Дождь"))
        {
          if((ret_val>=0.5)&&(ret_val<=4.0))
          {
            if((VNGO_f>=100)&&(VNGO_f<200))
            {
              ret_val=pic118(MDV_f,typeYavl);
              
              if( ret_val == "err1")
              {
                return "Исходные данные вне области допустимых значений метода: МДВ Sm<0.5 или Sm>4";
              }else
              if( ret_val == "err2")
              {
                return "Тип явления вне вне области допустимых значений метода!";
              }else
              if( !isValidNum(ret_val) )
              {
                return "Метод не работает с заданными значениями параметров";
              }

            }
          }
        }
      }
      return ftoi_norm(ret_val*1000);
      
   }else
    if ( typeAvia=="Вертолеты"){
      ret_val=pic119(MDV_f,VNGO_f);
      if( !isValidNum(ret_val) )
      {
        return "Метод не работает с заданными значениями параметров";
      }
      return ftoi_norm(ret_val*1000);

    }else{
      return "Метод не работает с заданными значениями параметров";
    }

}


//Hp - vngo; Sm - MDV_f; t - typeYavl
//Sp - NDV; Hs - высота облаков, которую определил бы летчик в полете – высота по «самолету»
function pic115 ( Hp,  Sm,  t )
{
  var k=[1.,0.78,0.933,1.];
  var v=[-40.,0.15,-0.033,0.1];

  var a=[0.07,0.0166,0.04375,0.0516,0.05955,0.0556];
  var b=[-0.47,0.1166,-0.05,0.001667,0.063625,0.28018];
  var c=[2.3,1.2008,1.65625,1.6306,1.673175,1.15906];
  var y,y1,y2,y3,y4,y5,y6,u1,u2,u3,u4,u5,u6;

  var Hs, Sp;

  if (Hp<50 || Hp>650)
  {
//    return "ВНГО вне области ОДЗ";
    return "err1";
  }
  if (Sm<2 || Sm>9)
  {
//    return "Горизонтальная видимость у поверхности земли вне области ОДЗ";
    return "err2";
  }

  y=parseFloat(k[0]*Hp)+parseFloat(v[0]);

  Hs=y;



  if (t=="Без явлений погоды"||t=="Морось")
  {
    if (Hs>=100 && Hs<=150)
    {
      y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
      Sp=y1;
    }
    if (Hs>150 && Hs<=200)
    {
      y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
      Sp=y2;
    }
    if (Hs>200 && Hs<=250)
    {
      y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
      Sp=y3;
    }
    if (Hs>250 && Hs<=300)
    {
      y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
      Sp=y4;
    }
    if (Hs>300 && Hs<=400)
    {
      y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
      Sp=y5;
    }
    if (Hs>400)
    {
      y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
      Sp=y6;
    }
 }else

 if (t=="Снег")
 {
   if (Hs>=100 && Hs<=150)
   {
     y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
     u1=parseFloat(k[1]*y1)+parseFloat(v[1]);
     Sp=u1;
   }
   if (Hs>150 && Hs<=200)
   {
     y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
     u2=parseFloat(k[1]*y2)+parseFloat(v[1]);
     Sp=u2;
   }
   if (Hs>200 && Hs<=250)
   {
     y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
     u3=parseFloat(k[1]*y3)+parseFloat(v[1]);
     Sp=u3;
   }
   if (Hs>250 && Hs<=300)
   {
     y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
     u4=parseFloat(k[1]*y4)+parseFloat(v[1]);
     Sp=u4;
   }
   if (Hs>300 && Hs<=400)
   {
     y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
     u5=parseFloat(k[1]*y5)+parseFloat(v[1]);
     Sp=u5;
   }
   if (Hs>400)
   {
     y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
     u6=parseFloat(k[1]*y6)+parseFloat(v[1]);
     Sp=u6;
   }
 }else

 if (t=="Дождь")
 {
   if (Hs>=100 && Hs<=150)
   {
     y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
     u1=parseFloat(k[2]*y1)+parseFloat(v[2]);
     Sp=u1;
   }
   if (Hs>150 && Hs<=200)
   {
     y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
     u2=parseFloat(k[2]*y2)+parseFloat(v[2]);
     Sp=u2;
   }
   if (Hs>200 && Hs<=250)
   {
     y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
     u3=parseFloat(k[2]*y3)+parseFloat(v[2]);
     Sp=u3;
   }
   if (Hs>250 && Hs<=300)
   {
     y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
     u4=parseFloat(k[2]*y4)+parseFloat(v[2]);
     Sp=u4;
   }
   if (Hs>300 && Hs<=400)
   {
     y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
     u5=parseFloat(k[2]*y5)+parseFloat(v[2]);
     Sp=u5;
   }
   if (Hs>400)
   {
     y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
     u6=parseFloat(k[2]*y6)+parseFloat(v[2]);
     Sp=u6;
   }
 }else

 if (t=="Дымка")
 {
   if (Hs>=100 && Hs<=150)
   {
     y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
     u1=parseFloat(k[3]*y1)+parseFloat(v[3]);
     Sp=u1;
   }
   if (Hs>150 && Hs<=200)
   {
     y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
     u2=parseFloat(k[3]*y2)+parseFloat(v[3]);
     Sp=u2;
   }
   if (Hs>200 && Hs<=250)
   {
     y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
     u3=parseFloat(k[3]*y3)+parseFloat(v[3]);
     Sp=u3;
   }
   if (Hs>250 && Hs<=300)
   {
     y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
     u4=parseFloat(k[3]*y4)+parseFloat(v[3]);
     Sp=u4;
   }
   if (Hs>300 && Hs<=400)
   {
     y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
     u5=parseFloat(k[3]*y5)+parseFloat(v[3]);
     Sp=u5;
   }
   if (Hs>400)
   {
     y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
     u6=parseFloat(k[3]*y6)+(v[3]);
     Sp=u6;
   }
 }
 else {return "err3"};
 return Sp;
}

//Функция для определения посадочной видимости Sp для самолетов
//дальней авиации по значениям горизонтальной дальности видимости Sm, высоты НГО H
//(по приборам), и явлениям погоды t. рис.116

function pic116 (Sm, H, t)
{
  var k=[0.666,0.85714,0.9,1.];
  var v=[0.,-0.14285,0.15,0.];
  var a=[0.04375,0.035,0.2333,0.03166,0.064,0.07,0.0799928];
  var b=[-0.1875,-0.075,0.07,0.095,-0.136,-0.07,0.04];
  var c=[1.,1.01,0.2268,1.18336,1.816,1.48,1.7];
  var y1,y2,y3,y4,y5,y6,y7,u1,u2,u3,u4,u5,u6,u7;
  var Sp;

  if (H<50)
  {

    return "err1"; //НГО H (по приборам)вне ОДЗ

  }
  if (Sm<2 || Sm>9)
  {

    return "err2";//горизонтальная дальность видимости вне ОДЗ

  }


  if (t=="Без явлений погоды")
  {
    if (H>50 && H<=100)
    {
      y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
      Sp=y1;
    }
    if (H>100 && H<=150)
    {
      y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
      Sp=y2;
    }
    if (H>150 && H<=200)
    {
      y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
      Sp=y3;
    }
    if (H>200 && H<=250)
    {
      y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
      Sp=y4;
    }
    if (H>250 && H<=300)
    {
      y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
      Sp=y5;
    }
    if (H>300 && H<=400)
    {
      y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
      Sp=y6;
    }
    if (H>400)
    {
      y7=parseFloat(a[6]*Sm*Sm)+parseFloat(b[6]*Sm)+parseFloat(c[6]);
      Sp=y7;
    }
  }else

  if (t=="Морось")
  {
    if (H>50 && H<=100)
    {
      y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
      u1=parseFloat(k[0]*y1)+parseFloat(v[0]);
      Sp=u1;
    }
    if (H>100 && H<=150)
    {
      y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
      u2=parseFloat(k[0]*y2)+parseFloat(v[0]);
      Sp=u2;
    }
    if (H>150 && H<=200)
    {
      y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
      u3=parseFloat(k[0]*y3)+parseFloat(v[0]);
      Sp=u3;
    }
    if (H>200 && H<=250)
    {
      y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+(c[3]);
      u4=parseFloat(k[0]*y4)+parseFloat(v[0]);
      Sp=u4;
    }
    if (H>250 && H<=300)
    {
      y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
      u5=parseFloat(k[0]*y5)+parseFloat(v[0]);
      Sp=u5;
    }
    if (H>300 && H<=400)
    {
      y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+(c[5]);
      u6=parseFloat(k[0]*y6)+parseFloat(v[0]);
      Sp=u6;
    }
    if (H>400)
    {
      y7=parseFloat(a[6]*Sm*Sm)+parseFloat(b[6]*Sm)+parseFloat(c[6]);
      u7=parseFloat(k[0]*y7)+parseFloat(v[0]);
      Sp=u7;
    }
  }else

  if (t=="Снег")
  {
    if (H>50 && H<=100)
    {
      y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
      u1=parseFloat(k[1]*y1)+parseFloat(v[1]);
      Sp=u1;
    }
    if (H>100 && H<=150)
    {
      y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
      u2=parseFloat(k[1]*y2)+parseFloat(v[1]);
      Sp=u2;
    }
    if (H>150 && H<=200)
    {
      y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
      u3=parseFloat(k[1]*y3)+parseFloat(v[1]);
      Sp=u3;
    }
    if (H>200 && H<=250)
    {
      y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
      u4=parseFloat(k[1]*y4)+parseFloat(v[1]);
      Sp=u4;
    }
    if (H>250 && H<=300)
    {
      y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
      u5=parseFloat(k[1]*y5)+parseFloat(v[1]);
      Sp=u5;
    }
    if (H>300 && H<=400)
    {
      y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
      u6=parseFloat(k[1]*y6)+parseFloat(v[1]);
      Sp=u6;
    }
    if (H>400)
    {
      y7=parseFloat(a[6]*Sm*Sm)+parseFloat(b[6]*Sm)+parseFloat(c[6]);
      u7=parseFloat(k[1]*y7)+parseFloat(v[1]);
      Sp=u7;
    }
  }else

  if (t=="Дождь")
  {
    if (H>50 && H<=100)
    {
      y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
      u1=parseFloat(k[2]*y1)+parseFloat(v[2]);
      Sp=u1;
    }
    if (H>100 && H<=150)
    {
      y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
      u2=parseFloat(k[2]*y2)+parseFloat(v[2]);
      Sp=u2;
    }
    if (H>150 && H<=200)
    {
      y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
      u3=parseFloat(k[2]*y3)+parseFloat(v[2]);
      Sp=u3;
    }
    if (H>200 && H<=250)
    {
      y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
      u4=parseFloat(k[2]*y4)+parseFloat(v[2]);
      Sp=u4;
    }
    if (H>250 && H<=300)
    {
      y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
      u5=parseFloat(k[2]*y5)+parseFloat(v[2]);
      Sp=u5;
    }
    if (H>300 && H<=400)
    {
      y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
      u6=parseFloat(k[2]*y6)+parseFloat(v[2]);
      Sp=u6;
    }
    if (H>400)
    {
      y7=parseFloat(a[6]*Sm*Sm)+parseFloat(b[6]*Sm)+parseFloat(c[6]);
      u7=parseFloat(k[2]*y7)+parseFloat(v[2]);
      Sp=u7;
    }
  }else

  if (t=="Дымка")
  {
    if (H>50 && H<=100)
    {
      y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
      u1=parseFloat(k[3]*y1)+parseFloat(v[3]);
      Sp=u1;
    }
    if (H>100 && H<=150)
    {
      y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
      u2=parseFloat(k[3]*y2)+parseFloat(v[3]);
      Sp=u2;
    }
    if (H>150 && H<=200)
    {
      y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
      u3=parseFloat(k[3]*y3)+parseFloat(v[3]);
      Sp=u3;
    }
    if (H>200 && H<=250)
    {
      y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
      u4=parseFloat(k[3]*y4)+parseFloat(v[3]);
      Sp=u4;
    }
    if (H>250 && H<=300)
    {
      y5=parseFloat(a[4]*Sm*Sm)+parseFloat(b[4]*Sm)+parseFloat(c[4]);
      u5=parseFloat(k[3]*y5)+parseFloat(v[3]);
      Sp=u5;
    }
    if (H>300 && H<=400)
    {
      y6=parseFloat(a[5]*Sm*Sm)+parseFloat(b[5]*Sm)+parseFloat(c[5]);
      u6=parseFloat(k[3]*y6)+parseFloat(v[3]);
      Sp=u6;
    }
    if (H>400)
    {
      y7=parseFloat(a[6]*Sm*Sm)+parseFloat(b[6]*Sm)+parseFloat(c[6]);
      u7=parseFloat(k[3]*y7)+parseFloat(v[3]);
      Sp=u7;
    }
  }
  return Sp;
}

//Функция для определения посадочной видимости Sp для самолетов
//военно-транспортой авиации по значениям горизонтальной дальности видимости Sm, высоты //НГО H (по приборам) рис.117.

function pic117( Sm,  H)
{
  var a=[0.0,0.01785,0.0233];
  var b=[0.5,0.52738,0.63];
  var c=[0.5,0.60477,0.5967];
  var y1,y2,y3;
  var Sp;

  if (Sm<1.)
  {
    return "err1";//Посадочная видимость менее 1 км!
  }
  if (Sm>8.)
  {
    return "err2";//Посадочная видимость более 8 км
  }

  if (H<100.)
  {
    return "Значение ВНГО должно быть больше 100м";
  }
  if (H>=100 && H<=200)
  {
    y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
    Sp=y1;
  }
  if (H>200 && H<=300)
  {
    y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
    Sp=y2;
  }
  if (H>300)
  {
    y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
    Sp=y3;
  }

  return Sp;
}

//Функция для определения посадочной видимости Sp при различных явлениях погоды t для //самолетов военно-транспортной авиации по значению горизонтальной видимости
// у земли Sm.рис.118

function pic118( Sm,  t)
{
  var a=[0.0, 0.125, 0.075];
  var b=[0.45, 0.05, 0.3];
  var c=[0.425, 0.844, 0.73];

  var y1,y2,y3;

  if (Sm<0.5 || Sm>4.)
  {

    return "err1";//МДВ вне ОДЗ
  }


  if (t=="Морось")
  {
    y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);//moros
    Sp=y1;
  }
  if (t=="Снег")
  {
    y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);//sneg
    Sp=y2;
  }
  if (t=="Дождь"||t=="Дымка")
  {
    y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);//dimka(dojd)
    Sp=y3;
  }
  else {return "err2"};//Тип явления вне области ОДЗ

  return Sp;
}

//Функция для определения посадочной видимости Sp при различной высоте облачности H для //вертолетов по значению горизонтальной видимости у земли Sm.рис.119

function pic119( Sm,  H )
{
  var Sp;
  var a=[-0.02381, 0.0656, 0.04272, 0.06015];
  var b=[0.41666, 0.1742, 0.4796, 0.55488];
  var c=[0.10714, 0.387, 0.27768, 0.38497];
  var y1,y2,y3,y4;
  if (Sm<0.5 || Sm>5.)
  {
    return "Метод не работает с заданными значениями параметров";
  }


  if (H<50. || H>300.)
  {
      return "Значение ВНГО должно быть больше 50м и меньше 300м";
  }
  if (H>=50 && H<=100)//50<=H<=100
  {
    y1=parseFloat(a[0]*Sm*Sm)+parseFloat(b[0]*Sm)+parseFloat(c[0]);
    Sp=y1;
  }
  if (H>100 && H<=150)//100<H<=150
  {
    y2=parseFloat(a[1]*Sm*Sm)+parseFloat(b[1]*Sm)+parseFloat(c[1]);
    Sp=y2;
  }
  if (H>150 && H<=200)//150<H<=200
  {
    y3=parseFloat(a[2]*Sm*Sm)+parseFloat(b[2]*Sm)+parseFloat(c[2]);
    Sp=y3;
  }
  if (H>200 && H<=300)//200<H<=300
  {
    y4=parseFloat(a[3]*Sm*Sm)+parseFloat(b[3]*Sm)+parseFloat(c[3]);
    Sp=y4;
  }

  return Sp;
}

