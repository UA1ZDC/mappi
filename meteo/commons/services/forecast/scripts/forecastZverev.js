var pic93V_x = [2.0, 4.0, 5.0, 8.0,16.0,20.0,  // v=0
                      3.0, 4.5,10.0,16.0,20.0, // v=0.5
                  2.5, 6.0,14.0,16.0,20.0,    //1
                  3.0, 6.0,10.0,14.0,16.0,20.0,    //2
                  3.5, 5.0,12.0,16.0,20.0,    //3
                  4.0,10.5,16.0,20.0,    //5
                  3.0, 4.0,12.0,16.0,20.0,    //10
                  2.0,10.0,14.0,20.0];   //15
var pic93V_y = [2.0, 4.0, 5.0, 8.0,16.0,20.0,  // v=0
                2.8, 4.0, 8.8,14.0,16.5, // v=0.5
            2.0, 4.8,10.8,12.1,14.0,   //1
            2.0, 4.0,6.2, 8.2,9.1,10.0,   //2
            2.0, 2.8, 5.8, 6.6, 6.9,    //3
            1.9, 4.0, 5.0, 4.8,    //5
            1.0, 1.2, 3.0, 3.5, 3.1,    //10
            0.2, 1.2, 1.8, 1.3];   //15
var pic93V_z = [0.0, 0.0, 0.0, 0.0,0.0,0.0,  // v=0
                0.5, 0.5, 0.5, 0.5,0.5,  // v=0.5
            1.0, 1.0, 1.0, 1.0,1.0,
            2.0, 2.0,2.0, 2.0, 2.0,2.0,
            3.0, 3.0, 3.0, 3.0,3.0,
            5.0, 5.0, 5.0, 5.0,
           10.0,10.0,10.0,10.0,10.0,
           15.0,15.0,15.0,15.0];

function sunsetsrok(Timezax)
{
  if(!isValidNum( Timezax )  )
  {
    return "Ошибка";
  }
 var hour = dataObj.getHourData();
 var srok = Timezax - hour;
 return ftoi_norm(srok);
}

// это сколько часов осталось до 3-х часов ночи
function srok3(){
    var srok = 0;
    var hour = dataObj.getHourData();
    var offset = zondObj.offsetFromUtc();
    srok = (24-hour+offset);
    if(srok>24) srok = srok-24;
    return srok;
}

// это сколько часов осталось до 12- часов
function srok12(){
    var hour = dataObj.getHourData();
    var offset = zondObj.offsetFromUtc();
    var srok12 = 12 - hour- offset;
    return srok12;
}
// это сколько часов осталось до 18- часов
function srok18(){
    var hour = dataObj.getHourData();
    var offset = zondObj.offsetFromUtc();
    var srok18 = 18 - hour- offset;
    return srok18;
}

//Функция прогноза для метода Zvereva
//f0zax,T0zax - температура воздуха и относительная влажность воздуха в момент захода Солнца
function getPrognozZverev(T_min,f0zax,T0zax)
{

  if( !isValidNum( T_min )   ||
      !isValidNum( f0zax )     ||
      !isValidNum( T0zax )  )
  {
    return "Метод не применим с такими входными параметрами";
  }

  var ret,vs_str ;
  var dTtum,dTdim;
  var ret_val,ot;
  var Ttum,Tdim;

  ot=ftoi_norm( f0zax);
//  ret_val=pic89(T0zax, ot, &dTtum, &dTdim);
  dTtum = pic89(1,T0zax, ot);//необходимое понижение температуры воздуха для  начала образования тумана
//  return dTtum;
  dTdim = pic89(2,T0zax, ot);//необходимое понижение температуры воздуха для  начала образования дымки
  if (dTtum==badvalue())  {return "Исходные данные вне области допустимых значений метода";}
  if (dTdim==badvalue())  {return "Исходные данные вне области допустимых значений метода";}

  Ttum=parseFloat(T0zax)-parseFloat(dTtum);// температура начала образования тумана
  Tdim=parseFloat(T0zax)-parseFloat(dTdim);// температура начала образования дымки

  if(T_min<=Ttum)
  {
//      20003
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 12, "Туман", 0,1);
      return "Туман";

  }

  if(T_min>Ttum && T_min<=Tdim)
  {
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 10, "Дымка", 0,1);
      return "Дымка";

  }

  if(T_min>Tdim)
  {
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0,"Без тумана и без дымки", 0,1);
      return "Без тумана и без дымки";
  }
  else return "Исходные данные вне области допустимых значений метода";

}

//Функция pic89 для определения NPT-ночного понижения температуры T, необходимого для образования тумана Tt  (рис.89 сплошные линии).
//tip 1 - dTtum; 2 - dTdim
function pic89( tip, T,  f )
{
  var dTtum, dTdim;
  var a=[0.001,0.0007,0.0011,0.001,0.0018,0.0016,0.0008,/*Tyman*/0.0008,0.0004,0.0006,0.0008,0.0006,0.0003,0.0006 /*Dimka*/];
  var b=[0.045,0.01,0.004,-0.03,-0.038,-0.052,-0.06,/*Tyman*/0.077,0.063,0.043,0.025,0.015,0.004,-0.006 /*Dimka*/];
  var c=[13.5,11.036,8.5,6.35,4.3,2.56,1.2,/*Tyman*/12.59,9.8,7.25,4.97,3.01,1.71,0.08 /*Dimka*/];

  var y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,y13,y14;
//----------------------------------------------------------------------
  if (T<-30. || T>30.)
    {return badvalue();}
  if (f<30. || f>100.)
    {return badvalue();}
//-----------------------------------------------------------------------
  if (f>30. && f<=40.)
  {
    y1=parseFloat(a[0]*T*T)+parseFloat(b[0]*T)+parseFloat(c[0]);
    dTtum=y1;
  }
  if (f>40. && f<=50.)
  {
    y2=parseFloat(a[1]*T*T)+parseFloat(b[1]*T)+parseFloat(c[1]);
    dTtum=y2;
  }
  if (f>50. && f<=60.)
  {
    y3=parseFloat(a[2]*T*T)+parseFloat(b[2]*T)+parseFloat(c[2]);
    dTtum=y3;
  }
  if (f>60. && f<=70.)
  {
    y4=parseFloat(a[3]*T*T)+parseFloat(b[3]*T)+parseFloat(c[3]);
    dTtum=y4;
  }
  if (f>70. && f<=80.)
  {
    y5=parseFloat(a[4]*T*T)+parseFloat(b[4]*T)+parseFloat(c[4]);
    dTtum=y5;
  }
  if (f>80. && f<=90.)
  {
    y6=parseFloat(a[5]*T*T)+parseFloat(b[5]*T)+parseFloat(c[5]);
    dTtum=y6;
  }
  if (f>90. && f<=100.)
  {
    y7=parseFloat(a[6]*T*T)+parseFloat(b[6]*T)+parseFloat(c[6]);
    dTtum=y7;
  }
///////////////////////////////////////////////
  if(f>30. && f<=40.)
  {
    y8=parseFloat(a[7]*T*T)+parseFloat(b[7]*T)+parseFloat(c[7]);
    dTdim=y8;
  }
  if (f>40. && f<=50.)
  {
    y9=parseFloat(a[8]*T*T)+parseFloat(b[8]*T)+parseFloat(c[8]);
    dTdim=y9;
  }
  if (f>50. && f<=60.)
  {
    y10=parseFloat(a[9]*T*T)+parseFloat(b[9]*T)+parseFloat(c[9]);
    dTdim=y10;
  }
  if (f>60. && f<=70.)
  {
    y11=parseFloat(a[10]*T*T)+parseFloat(b[10]*T)+parseFloat(c[10]);
    dTdim=y11;
  }
  if (f>70. && f<=80.)
  {
    y12=parseFloat(a[11]*T*T)+parseFloat(b[11]*T)+parseFloat(c[11]);
    dTdim=y12;
  }
  if (f>80. && f<=90.)
  {
    y13=parseFloat(a[12]*T*T)+parseFloat(b[12]*T)+parseFloat(c[12]);
    dTdim=y13;
  }
  if (f>90. && f<=100.)
  {
    y14=parseFloat(a[13]*T*T)+parseFloat(b[13]*T)+parseFloat(c[13]);
    dTdim=y14;
  }
//  else
//  {
//    dTtum = badvalue();
//    dTdim = badvalue();
//  }

  if( tip == 1)
  {
    return dTtum;
  }

  if( tip == 2)
  {
    return dTdim;
  }
}


function getHumiditySunset(T,D,Td)
{
  var f0zax;
  var ret,vs_str;
  if((!isValidNum(D)&&!isValidNum(Td))|| !isValidNum(T) ) return badvalue();
  if(!isValidNum(Td)){
   Td = parseFloat(T) - parseFloat(D);
  }
  f0zax=getVlaznostMagnus(T,Td);
  return ftoi_norm(f0zax*100.);
}


function getVlaznostMagnus( T, Td )
 {
   if(!isValidNum(T) ||
      !isValidNum(Td) )
      return badvalue();

   T = parseFloat(T);
   Td = parseFloat(Td);

   var tmp1,tmp2;
   var  a,b;
   var result;


   if (T<-10.)
   {
     a=9.5;
     b=265.5;
   }
   else
   {
     a=7.63;
     b=241.9;
   }

   tmp1=(a*Td)/(parseFloat(b)+parseFloat(Td));
   tmp2=(a*T)/(parseFloat(b)+parseFloat(T));
//   return tmp2;

   tmp1=parseFloat(tmp1)-parseFloat(tmp2);
   tmp2= Math.pow(10.,tmp1);
   if (tmp2<0.)
//      return tmp2=0.;
     result = 0;
   if (tmp2>1.)
//      return tmp2=1.;
     result = 1;
   else
//      return tmp2;
     result = tmp2;
   return result;

 }

//2
//Функция для расчета ночного понижения температуры воздуха для метода Zvereva
//Т и f за 13 (19) ч ±1 ч определяется возможное понижение температуры  от исходного срока
//при ясном небе без учета скорости ветра
function getdTn(f0,T,t_is)
{
//    return "f0";
  if( !isValidNum(f0) )
     return "Метод не работает с такими значеними влажности в исходный срок";
  if( !isValidNum(T) )
      return "Метод не работает с такими значениями температуры";


  var ret,vs_str;
  var ret_val;
  var dTn;
  t_is = parseFloat(t_is);


  if (t_is>=12. && t_is<=14.)
  {
     dTn=pic91_13(T, f0);
     if (dTn=="err1") return "Исходные данные вне области допустимых значений метода";
  }
  if (t_is>=18. && t_is<=20.)
  {
    dTn=pic91_19(T, f0);
    if (dTn=="err1") return "Исходные данные вне области допустимых значений метода";
  }
  return dTn;
}


//Функция pic91a для определения ночного понижения теперетуры NPT по значениям температуры воздуха T и относительной влажности f за 13 часов(рис. 91(a)).
function pic91_13( T13,  f )
{
  var a=[0.003,0.003,0.0025,0.0033,0.0028,0.0032,0.004,0.0025,0.0029,0.003,/**/0.0035,0.0031,0.002,0.002,0.0017,0.0015,0.0018,0.0018,0.0036,0.0026];
  var b=[0.129,0.11,0.094,0.093,0.081,0.079,0.094,0.035,0.032,0.013,/**/0.097,0.085,0.084,0.073,0.066,0.043,0.026,0.005,-0.044,-0.0278];
  var c=[9.998,9.332,8.694,7.883,7.209,6.475,5.85,4.875,3.98,3.1,/**/9.823,9.205,8.604,7.815,7.17,6.32,5.56,4.875,3.98,3.1];

  var y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,y13,y14,y15,y16,y17,y18,y19,y20;
  var dT;
//----------------------------------------------------------------------
  if (T13<-30. || T13>30.)
    {return "err1";}
  if (f<1. || f>100.)
    {return "err1";}
//-----------------------------------------------------------------------
  if (T13<=-5.)
  {
     if (f>=1. && f<=10.)
     {
       y1=parseFloat(a[0]*T13*T13)+parseFloat(b[0]*T13)+parseFloat(c[0]);
       dT=y1;
     }
     if (f>10. && f<=20.)
     {
       y2=parseFloat(a[1]*T13*T13)+parseFloat(b[1]*T13)+parseFloat(c[1]);
       dT=y2;
     }
     if (f>20. && f<=30.)
     {
       y3=parseFloat(a[2]*T13*T13)+parseFloat(b[2]*T13)+parseFloat(c[2]);
       dT=y3;
     }
     if (f>30. && f<=40.)
     {
       y4=parseFloat(a[3]*T13*T13)+parseFloat(b[3]*T13)+parseFloat(c[3]);
       dT=y4;
     }
     if (f>40. && f<=50.)
     {
       y5=parseFloat(a[4]*T13*T13)+parseFloat(b[4]*T13)+parseFloat(c[4]);
       dT=y5;
     }
     if (f>50. && f<=60.)
     {
       y6=parseFloat(a[5]*T13*T13)+parseFloat(b[5]*T13)+parseFloat(c[5]);
       dT=y6;
     }
     if (f>60. && f<=70.)
     {
       y7=parseFloat(a[6]*T13*T13)+parseFloat(b[6]*T13)+parseFloat(c[6]);
       dT=y7;
     }
     if (f>70. && f<=80.)
     {
       y8=parseFloat(a[7]*T13*T13)+parseFloat(b[7]*T13)+parseFloat(c[7]);
       dT=y8;
     }
  }
  if (T13<=0)
  {
    if (f>80. && f<=90.)
    {
      y9=parseFloat(a[8]*T13*T13)+parseFloat(b[8]*T13)+parseFloat(c[8]);
      dT=y9;
    }
    if (f>90. && f<=100.)
    {
      y10=parseFloat(a[9]*T13*T13)+parseFloat(b[9]*T13)+parseFloat(c[9]);
      dT=y10;
    }
  }
//----------------------------------------------
///////////////////////////////////////////////
  if (T13>-5)
  {
    if(f>=1. && f<=10.)
    {
      y11=parseFloat(a[10]*T13*T13)+parseFloat(b[10]*T13)+parseFloat(c[10]);
      dT=y11;
    }
    if(f>10. && f<=20.)
    {
      y12=parseFloat(a[11]*T13*T13)+parseFloat(b[11]*T13)+parseFloat(c[11]);
      dT=y12;
    }
    if(f>20. && f<=30.)
    {
      y13=parseFloat(a[12]*T13*T13)+parseFloat(b[12]*T13)+parseFloat(c[12]);
      dT=y13;
    }
    if(f>30. && f<=40.)
    {
      y14=parseFloat(a[13]*T13*T13)+parseFloat(b[13]*T13)+parseFloat(c[13]);
      dT=y14;
    }
    if (f>40. && f<=50.)
    {
      y15=parseFloat(a[14]*T13*T13)+parseFloat(b[14]*T13)+parseFloat(c[14]);
      dT=y15;
    }
    if (f>50. && f<=60.)
    {
      y16=parseFloat(a[15]*T13*T13)+parseFloat(b[15]*T13)+parseFloat(c[15]);
      dT=y16;
    }
    if (f>60. && f<=70.)
    {
      y17=parseFloat(a[16]*T13*T13)+parseFloat(b[16]*T13)+parseFloat(c[16]);
      dT=y17;
    }
    if (f>70. && f<=80.)
    {
      y18=parseFloat(a[17]*T13*T13)+parseFloat(b[17]*T13)+parseFloat(c[17]);
      dT=y18;
    }
  }
  if (T13>0)
  {
    if (f>80. && f<=90.)
    {
      y19=parseFloat(a[18]*T13*T13)+parseFloat(b[18]*T13)+parseFloat(c[18]);
      dT=y19;
    }
    if (f>90. && f<=100.)
    {
      y20=parseFloat(a[19]*T13*T13)+parseFloat(b[19]*T13)+parseFloat(c[19]);
      dT=y20;
    }
  }
  else dT = badvalue();

  return dT;
}

//Функция pic91b для определения ночного понижения теперетуры NPT по значениям температуры воздуха T и относительной влажности f за 19 часов(рис. 91(b)).

function pic91_19( T19,  f )
{
  var a=[0.003,0.008,0.0023,0.0023,0.0018,0.0013,0.0013,0.0023,0.002,0.001,/**/0.0046,0.0061,0.0023,0.0022,0.0018,0.0014,0.0017,0.0017,0.001,0.0017];
  var b=[0.136,0.33,0.087,0.097,0.052,0.032,0.032,0.049,0.025,-0.01,/**/0.144,0.037,0.116,0.089,0.085,0.084,0.049,0.029,0.025,-0.012];
  var c=[11.955,13.15,10.378,9.928,8.875,8.128,7.627,6.987,6.125,5.275,/**/11.965,11.732,10.522,9.89,9.15,8.375,7.712,6.912,6.1,5.25];

  var y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,y13,y14,y15,y16,y17,y18,y19,y20;
  var dT;
//----------------------------------------------------------------------
  if (T19<-30. || T19>30.)
    {return "err1";}
  if (f<1. || f>100.)
    {return "err1";}
//-----------------------------------------------------------------------
  if (T19<=-5.)
  {
    if (f>=1. && f<=10.)
    {
      y1=parseFloat(a[0]*T19*T19)+parseFloat(b[0]*T19)+parseFloat(c[0]);
      dT=y1;
    }
    if (f>10. && f<=20.)
    {
      y2=parseFloat(a[1]*T19*T19)+parseFloat(b[1]*T19)+parseFloat(c[1]);
      dT=y2;
    }
    if (f>20. && f<=30.)
    {
      y3=parseFloat(a[2]*T19*T19)+parseFloat(b[2]*T19)+parseFloat(c[2]);
      dT=y3;
    }
    if (f>30. && f<=40.)
    {
      y4=parseFloat(a[3]*T19*T19)+parseFloat(b[3]*T19)+parseFloat(c[3]);
      dT=y4;
    }
    if (f>40. && f<=50.)
    {
      y5=parseFloat(a[4]*T19*T19)+parseFloat(b[4]*T19)+parseFloat(c[4]);
      dT=y5;
    }
    if (f>50. && f<=60.)
    {
      y6=parseFloat(a[5]*T19*T19)+parseFloat(b[5]*T19)+parseFloat(c[5]);
      dT=y6;
    }
    if (f>60. && f<=70.)
    {
      y7=parseFloat(a[6]*T19*T19)+parseFloat(b[6]*T19)+parseFloat(c[6]);
      dT=y7;
    }
    if (f>70. && f<=80.)
    {
      y8=parseFloat(a[7]*T19*T19)+parseFloat(b[7]*T19)+parseFloat(c[7]);
      dT=y8;
    }
    if (f>80. && f<=90.)
    {
      y9=parseFloat(a[8]*T19*T19)+parseFloat(b[8]*T19)+parseFloat(c[8]);
      dT=y9;
    }
    if (f>90. && f<=100.)
    {
      y10=parseFloat(a[9]*T19*T19)+parseFloat(b[9]*T19)+parseFloat(c[9]);
      dT=y10;
    }
  }
//----------------------------------------------
///////////////////////////////////////////////
  if (T19>-5)
  {
    if(f>=1. && f<=10.)
    {
      y11=parseFloat(a[10]*T19*T19)+parseFloat(b[10]*T19)+parseFloat(c[10]);
      dT=y11;
    }
    if(f>10. && f<=20.)
    {
      y12=parseFloat(a[11]*T19*T19)+parseFloat(b[11]*T19)+parseFloat(c[11]);
      dT=y12;
    }
    if(f>20. && f<=30.)
    {
      y13=parseFloat(a[12]*T19*T19)+parseFloat(b[12]*T19)+parseFloat(c[12]);
      dT=y13;
    }
    if(f>30. && f<=40.)
    {
      y14=parseFloat(a[13]*T19*T19)+parseFloat(b[13]*T19)+parseFloat(c[13]);
      dT=y14;
    }
    if (f>40. && f<=50.)
    {
      y15=parseFloat(a[14]*T19*T19)+parseFloat(b[14]*T19)+parseFloat(c[14]);
      dT=y15;
    }
    if (f>50. && f<=60.)
    {
      y16=parseFloat(a[15]*T19*T19)+parseFloat(b[15]*T19)+parseFloat(c[15]);
      dT=y16;
    }
    if (f>60. && f<=70.)
    {
      y17=parseFloat(a[16]*T19*T19)+parseFloat(b[16]*T19)+parseFloat(c[16]);
      dT=y17;
    }
    if (f>70. && f<=80.)
    {
      y18=parseFloat(a[17]*T19*T19)+parseFloat(b[17]*T19)+parseFloat(c[17]);
      dT=y18;
    }
    if (f>80. && f<=90.)
    {
      y19=parseFloat(a[18]*T19*T19)+parseFloat(b[18]*T19)+parseFloat(c[18]);
      dT=y19;
    }
    if (f>90. && f<=100.)
    {
      y20=parseFloat(a[19]*T19*T19)+parseFloat(b[19]*T19)+parseFloat(c[19]);
      dT=y20;
    }
  }
  return dT;
}

//3
//m учитывает влияние облачности на ночное понижение температуры
//type_N,N - прогноз данных об облачности на ночь
function get_m(type_N,N)
{
  var ret_val;
  var ret, vs_str;
  var m;
  var type;

//  type_N=5;//"нижняя"
//  type_N=4;//"средняя плотная"
//  type_N=3;//"средняя тонкая"
//  type_N=2;//"верхняя плотная"
//  type_N=1;//"верхняя тонкая"
// N от 1 до 10
    if(type_N == "Верхняя тонкая")
    {
      type = 1.;
    }
    if(type_N == "Верхняя плотная")
    {
      type = 2.;
    }
    if( type_N == "Средняя тонкая")
    {
      type = 3.;
    }
    if(type_N == "Cредняя плотная")
    {
      type = 4.;
    }
    if(type_N == "Нижняя")
    {
      type = 5.;
    }
//  print(type);
  N = ftoi_norm(N*0.1);
  m=pic92(N, type);
  if (m=="err1") {return "Ошибка: Количество облачности < 0 или > 10. Повторите ввод";}
  if (m=="err2") {return "Такого яруса нет. Повторите ввод";}
  return m;
}

function pic92 ( n,  f)
//для данной функции f-номер яруса от 1 до 5
//Номера ярусов облачности f:
//1-верхняя тонкая
//2-верхняя плотная
//3-средняя тонкая
//4-средняя плотная
//5-нижняя

//n - количество облаков
{
  var k=[-0.01,-0.02,-0.04,-0.09,-0.086];
  var b=[1.,1.,1.,1.02,0.992];
  var y1, y2, y3, y4, y5;
  var m;



//граничные условия
  var x_str=0., x_end=10.;
  if (n<x_str || n>x_end)
  {
    return "err1";
  }
  if (f!=1 && f!=2 && f!=3 && f!=4 && f!=5)
  {
    return "err2";
  }
  if (f==1)
  {
    y1=parseFloat(k[0]*n)+parseFloat(b[0]);
    m=y1;
  }
  if (f==2)
  {
    y2=parseFloat(k[1]*n)+parseFloat(b[1]);
    m=y2;
  }
  if (f==3)
  {
    y3=parseFloat(k[2]*n)+parseFloat(b[2]);
    m=y3;
  }
  if (f==4)
  {
    y4=parseFloat(k[3]*n)+parseFloat(b[3]);
    m=y4;
  }
  if (f==5)
  {
    y5=parseFloat(k[4]*n)+parseFloat(b[4]);
    m=y5;
  }
  if (m<0 && m>1) {return badvalue();}

  return m;
}

//4. результат идет в 1
//Минимальная температура Зверев
//по значениям ночного понижения температуры, ff и m определяется
//возможное понижение температуры воздуха от исходного срока с учетом этих факторов
//ff - прогноз ветра на ночь
//dTn - температура воздуха в исходный срок 13 или 19 ч (±1 ч).
//T0 - температура воздуха в срок захода.
function T_minZverev(dTn19,dTn13,ff_pr,T0,N,type_N)
{
  var Tmin=0.;
  var ret=0.,vs_str=0.;
  var ret_val=0.;
  var dTn_p=0.;
  N = ftoi_norm(N*0.1);
  var m = get_m( type_N ,N);
 var dTn;
  if(isValidNum(dTn19)){
     dTn =  dTn19;
  } else {
      if(isValidNum(dTn13)){
         dTn =  dTn13;
      } else return badvalue();
  }

  dTn_p=pic93V(dTn,ff_pr, m);
  if(dTn_p == badvalue())
  {
    return badvalue();
  }

  Tmin=(T0)-(dTn_p);
  if( isValidNum(Tmin))
  {
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 12112, Tmin, Tmin, 0,1);
  }

  return Tmin;
}

function pic93V( dTn,  V,  m)
{
  var y_str=0.0;
  var y_end=20.0, y_min;
  var ret_val;
  var X_is;

  y_min=goldDivision_pic93V(y_str, y_end, dTn, V, 0.01);
  if( y_min == badvalue())
  {
    return badvalue();
  }
  X_is=(y_min*m);

  return X_is;
}

//Функция для правила Золотого сечения
function goldDivision_pic93V( y_str,  y_end,  dTn,  V,  eps)
{
  var a,b,x1,x2;
  var k=(Math.sqrt(5.)-1.)/2.;
  var f1,f2;

  a=y_str;
  b=y_end;
  x1=parseFloat(a)+(1-k)*(b-a);
  x2=parseFloat(a)+k*(b-a);
  f1 = F_Y_pic93V(x1, dTn, V);
  if( f1 == badvalue())
  {
    return badvalue();
  }
  f1=f1*(-1.);
  f2 = F_Y_pic93V(x2, dTn, V);
  if( f2 == badvalue())
  {
    return badvalue();
  }
  f2=f2*(-1.);


  while(Math.abs(x2-x1)>eps)
  {
//   if(f1>f2)
    if(f1<f2)
    {
      a=x1;
      x1=x2;
      f1=f2;
      x2=parseFloat(a)+k*(b-a);
      f2 = F_Y_pic93V(x2, dTn, V);
      if( f2 == badvalue())
      {
        return badvalue();
      }
      f2=f2*(-1.);
    }
    else
    {
      b=x2;
      x2=x1;
      f2=f1;
      x1=parseFloat(a)+(1-k)*(b-a);
      f1 = F_Y_pic93V(x1, dTn, V);
      if( f1 == badvalue())
      {
        return badvalue();
      }
      f1=f1*(-1.);
    }
  }
  var y_min;
  return y_min=(parseFloat(x1)+parseFloat(x2))/2.;
//return 0;
}


function F_Y_pic93V(  Y,  dTn,  V)
{
  var Z;
  var y_str=0.0, y_end=20.0, x_str=2.0, x_end=20.0;

  var F;
  if(dTn<x_str||dTn>x_end)
  {
    return 1;
  }
  if(Y<y_str||Y>y_end)
  {
    return 1;
  }

  var kol_point=40;  //40
  var ret_val;
  var eps=1.e-10;
//  bool pok;
//poprAlex = zondObj.pic(pic9_20b_x,pic9_20b_y,pic9_20b_z,ff_max, HSloy);
  Z = zondObj.pic(pic93V_x,pic93V_y,pic93V_z,dTn,Y);
  if( Z == badvalue())
  {
    return badvalue();
  }

//  ret_val=interpolHaos(kol_point,x,y,z,eps,&dTn,&Y,&Z,&pok,1,1);
//  if(ret_val==125) ret_val=0;

  F = Math.abs(Z-V);
  //*F=Z-V;

  return F;
}

function pic95( Tem, tip ) //температура возникновения тумана/ночное понижение температуры
{
  var a=0.67;
  var b=0.271;
  var c=0.0053;
  var y;

  if (Tem<0. || Tem>1.)
  {
    if(tip == 2)
    {
      return 1;
    }
  }
  if (Tem>=0. && Tem<=1.)
  {
    y=parseFloat(a*Tem*Tem)+parseFloat(b*Tem)+parseFloat(c);
    return y;
  }

  if(tip == 2)
  {
    return 0;
  }
}

function sunrise()
{
  return dataObj.getSunRiseSet(obj.getStationData(),1);
}


function sunset()
{
  return dataObj.getSunRiseSet(obj.getStationData(),2);
}


//9.3 Функция для расчета времени образования тумана по методу Зверева
function getTime_tum(f0zax,T0zax,T_min,time_vs,time_zx)
{
    if( !isValidNum( f0zax )   ||
        !isValidNum( T0zax )     ||
        !isValidNum( T_min )  ||
        !isValidNum( time_vs )   ||
        !isValidNum( time_zx )  )
    {
      return badvalue();
    }


  var ot=ftoi_norm( f0zax);
  //  ret_val=pic89(T0zax, ot, &dTtum, &dTdim);
  var dTtum = pic89(1,T0zax, ot);//необходимое понижение температуры воздуха для  начала образования тумана
//    return dTtum;
  var dTdim = pic89(2,T0zax, ot);//необходимое понижение температуры воздуха для  начала образования дымки
  if (dTtum==badvalue())  {return "Некорректное значение необходимого понижения температуры воздуха для начала образования тумана: повторите ввод данных влажности и температуры в момент захода Солнца";}
  if (dTdim==badvalue())  {return "Некорректное значение необходимого понижения температуры воздуха для начала образования дымки: повторите ввод данных влажности и температуры в момент захода Солнца";}

  var Ttum=parseFloat(T0zax)-parseFloat(dTtum);// температура начала образования тумана
  var Tdim=parseFloat(T0zax)-parseFloat(dTdim);// температура начала образования дымки

  var tt;
  var n_d;
//  var time_vs = sunriseset(1);
//  var time_zx =  sunriseset(2);
  if(time_vs<time_zx) time_vs+=24;
  var tn = parseFloat(time_vs) - parseFloat(time_zx);
  var TdTn = (parseFloat(T0zax)-parseFloat(Tdim)) / (parseFloat(T0zax)-parseFloat(T_min));

  var TtTn = (parseFloat(T0zax) - parseFloat(Ttum)) / (parseFloat(T0zax) - parseFloat(T_min)); //T0zax - температура в момент захода Солнца.
  var n_t = pic95(TtTn,1);
  var ret = pic95(TtTn,2);
  if (ret != 1) { tt = parseFloat(time_zx) + parseFloat(n_t*tn);}
  else
  {
    n_d=pic95(TdTn,1);
    ret=pic95(TdTn,2);
    if (ret != 0 ) {return "Не определено";}
    else { tt = parseFloat(time_zx) + parseFloat(n_d*tn);}

  }
  if(tt>24) tt=tt-24;
  var hour = parseInt(tt);
  var minutes = parseInt( (tt - hour) * 60);
  return hour + " часов " + minutes+ " минут";
}

function getVidimost_tum(T0zax,D0zax,Tdzax,f0zax)
{
  var T0,D0,Td0,T_min,dTtum,dTdim,Ttum, dTd;
print(T0zax,D0zax,f0zax);
    if( !isValidNum( T0zax )   ||
        (!isValidNum( D0zax )&&!isValidNum( Tdzax ) )  ||
        !isValidNum( f0zax )  )
    {
      return badvalue();
    }

    if( T0zax == 0.   &&
        D0zax == 0.   )
    {
      return badvalue();
    }
  if(!isValidNum( Tdzax )){
      Tdzax = parseFloat(T0zax) - parseFloat(D0zax);
   }

  var ot=ftoi_norm(f0zax);
   dTtum=pic89(1, T0zax, ot);
   dTdim=pic89(2, T0zax, ot);

  if (dTtum==badvalue())  {return "Некорректное значение необходимого понижения температуры воздуха для начала образования тумана: повторите ввод данных влажности и температуры в момент захода Солнца";}
  if (dTdim==badvalue())  {return "Некорректное значение необходимого понижения температуры воздуха для начала образования дымки: повторите ввод данных влажности и температуры в момент захода Солнца";}

  else Ttum=parseFloat(T0zax)-parseFloat(dTtum);

  if(T_min<=Ttum)
  {
//    Td0=T0-D0;
//    dTd=Td0-T_min;
      return "Не определена";
  }
  else return "600-1000 м";
}

