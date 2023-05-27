function badRet(){
  return "Неверные входные параметры метода";
}

//Функция для расчета температуры туманообразования для метода Берлянда
 function getT_tum(Td0_18, Td0_12, Td0, Td0_6)
 {
  var t_is = dataObj.getHourData();
   if( t_is != 12 && t_is != 18){
     return "Исходные данные должны быть за 12 или 18 часов";
   }
   if( !isValidNum( Td0_18 )   ||
       !isValidNum( Td0_12 )   ||
       !isValidNum( Td0 ) ||
       !isValidNum( Td0_6 ) )
   {
     return badRet();
   }



   if( t_is == 12 || t_is == 18)
   {
     var ret,vs_str ;
     ret = (parseFloat(2.*Td0)+parseFloat(Td0_6)+parseFloat(Td0_12)+parseFloat(Td0_18))/5.;
     return ret;
   }
   else return "Исходные данные должны быть за 12 или 18 часов";

 }

 function sunrise()
 {
   var time = dataObj.getSunRiseSet(obj.getStationData(),1);

   return time;
 }

//--/////////--///////////--/////////////--//////////////--////////////////--///////////////--/////////////--//////////////--/////////

 // Функция расчета упругости насыщения
 function getUprugAll( T)
 {
   if( !isValidNum( T )  )
   {
     return badRet();
   }
   var tmp, E0, E;
   E0=6.1078;
   tmp=(241.9+parseFloat(T));
   if(tmp==0.) return badRet();
   E=(E0*Math.pow(10,(7.63*T/(parseFloat(241.9)+parseFloat(T)))));
   return E;
 }
//type_pochv:
//Сухая
//Влажная
//Мокрая
//t_is - срок исходных данных
//vs_s - время восхода солнца
//t_is_vs - Промежуток времени от исходного срока до восхода Солнца

//T0,T0_6,T0_12,T0_18 - данные о температуре воздуха за четыре срока наблюдений, отстоящие друг от друга на 6 ч
////".ish1",".ish2",".ish3",".ish4",".ishtd1",".Nhi",".Nmid",".Nh",".skorPr",".tip",".sunrise"


 function T_minBer(T0, T0_6, T0_12, T0_18, Td0, N_high, N_midle, N_low, ff, type_pochv,t_vs)
 {
     if( !isValidNum( T0 ) ||
         !isValidNum( T0_6 )||
         !isValidNum( T0_12 )||
         !isValidNum( T0_18 )||
         !isValidNum( Td0 )||
         !isValidNum( N_high )||
         !isValidNum( N_midle)||
         !isValidNum( N_low )||
         !isValidNum( ff ) )
     {
       return badRet();
     }

print (t_vs);
  // return T0;

   var Tsr,N;
   var ret, type_N, vs_str ;
   var e0;
   var month,ret_val,t_is_vs,colVo;
   var T_min;
//   char time[32],date[32],year_ch,*month_ch,*t_is_ch,vs_s[32];
   var year;
   var vs_d;
   month = dataObj.getMonth();

   Tsr=(2.*T0 +parseFloat(T0_6)+parseFloat(T0_12)+parseFloat(T0_18))/5.;
   e0 = getUprugAll(Td0);
   if(e0 == badvalue()) {return "Упругость водяного пара вне области допустимых значений метода";}

  //Исправлено 15 10 07
   if (N_low>=8 && N_low<=10)
   {
     N=N_low;
     type_N="n";
//    goto m2;
   }
   if (N_midle>=8 && N_midle<=10)
   {
     N=N_midle;
     type_N="s";
//     goto m2;
   }
   if (N_low==6 || N_low==7)
   {
     N=N_low;
     type_N="n";
//     goto m2;
   }
   if (N_midle==6  || N_midle==7)
   {
     N=N_midle;
     type_N="s";
//     goto m2;
   }
   if (N_low==4 || N_low==5 && N_high>=8 && N_high<=10)
   {
     N=N_low;
     type_N="n";
//     goto m2;
   }
   if (N_midle==4  || N_midle==5 && N_high==6 || N_high==7)
   {
     N=N_midle;
     type_N="s";
//     goto m2;
   }
   if (N_low==2 || N_low==3)
   {
     N=N_low;
     type_N="n";
//     goto m2;
   }
   if (N_midle==2 || N_midle==3 && N_high==4 || N_high==5)
   {
     N=N_midle;
     type_N="s";
//     goto m2;
   }
   if (N_high==2 || N_high==3)
   {
     N=N_high;
     type_N="v";
//     goto m2;
   }
   if(N_low==0 && N_midle==0 && N_high==0) {N=0; type_N="o";}
   if(N_low==1 && N_midle==1 && N_high==1) {N=0; type_N="o";}
   if(N_low==1 && N_midle==0 && N_high==0) {N=0; type_N="o";}
   if(N_low==0 && N_midle==1 && N_high==0) {N=0; type_N="o";}
   if(N_low==0 && N_midle==0 && N_high==1) {N=0; type_N="o";}
   if(N_low==1 && N_midle==1 && N_high==0) {N=0; type_N="o";}
   if(N_low==0 && N_midle==1 && N_high==1) {N=0; type_N="o";}
   if(N_low==1 && N_midle==0 && N_high==1) {N=0; type_N="o";}

 //7. оценка увлажненности почвы--------------------

// colVo=vs_str.contains("сухая",true);
// if(colVo>0) type_pochv="s";
//   else
//    {
//       colVo=vs_str.contains("влажная",true);
//       if(colVo>0) type_pochv="v";
//         else
//         {
//            colVo=vs_str.contains("мокрая",true);
//            if(colVo>0) type_pochv="m";
//              else goto m1;
//           }
//     }

   var t_is = dataObj.getHourData();
   if( t_is == 12 || t_is == 18)
   {
     t_is_vs = parseFloat(t_vs) + 24. - parseFloat(t_is);

     var e02 = ftoi_norm(e0);
     T_min = pic_94(t_is, month,Tsr,T0,e02,N,type_N, type_pochv, ff, t_is_vs);


     if( T_min == "err1") {return "Широта пункта вне области допустимых значений метода";}
     if( T_min == "err2") {return "Средняя температура вне области допустимых значений метода";}
     if( T_min == "err3") {return "Зимой метод не применим";}
     if( T_min == "err4") {return "Упругость водяного пара вне области допустимых значений метода";}
     if( T_min == "err5") {return "Количество облачности вне области допустимых значений метода";}
     if( T_min == "err6") {return "Скорость ветра вне области допустимых значений метода";}
     if( T_min == "err7") {return "Промежуток времени от исходного срока до восхода Солнца вне области допустимых значений метода";}
     if( T_min == "err8") {return "Срок исходных данных вне области допустимых значений метода";}

     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 12112, T_min, T_min, 0,1);

     return T_min;
   }
   else return "Исходные данные должны быть за 12 или 18 часов";
}

//t_is - срок исходных данных
//mes - месяц
//Tsr - средняя температура
//e0 - упругость водяного пара
//t_is_vs - Промежуток времени от исходного срока до восхода Солнца
//type_N - облачные уровни (низкий, средний, высокий)

function pic_94 ( t_is,  mes,  Tsr,  T0,  e0,   N,  type_N,  type_poch,  ff,  t_is_vs)
{
  var koef_b=0., kk=0., qq=0., x=0.;
  var Tmin=0.;

//  if(t_is==19 &&(mes==6||mes==7) &&(shir<40 || shir>60)) return "err1";
  if (Tsr<0. || Tsr>23.) return "err2";
  if (mes<3 || mes>11) return "err3";
  if (e0<0 || e0>18)  return "err4";
  if (N<0 || N>10) return "err5";
  if (ff<=0 || ff>5) return "err6";
  if (t_is_vs<4 || t_is_vs>18) return "err7";
  if ((t_is<12 && t_is>14) || (t_is<18 && t_is>20)) return "err8";

  var a=[0.,0.,0.,0.,0.,0.,0.,0.,0];
  var b=[0.0046,0.0051,0.006,0.0071,0.008,0.0087,0.009,0.0094,0.01];
  var c=[0.0325,0.05,0.055,0.065,0.072,0.076,0.081,0.086,0.09];
//Прямые состояния почвы и скорости ветра
  var u=[-50.,-53.33,-55.55,-57.14,-60.,-63.63,-67.85,-71.42,-75.625];
  var r=[0.,0.,0.,0.,0.,0.,0.,0.,0.];
//Прямые для учета месяца и широты за 13 или 19 часов
  var p=[-1.714,-1.375,-1.142,-1.06,-1.,-0.952,-0.9,-0.83];
  var d=[0.,0.,0.,0.,0.,0.,0.,0.];

  var p1=0.,p2=0.,p3=0.,p4=0.,p5=0.,p6=0.,p7=0.,p8=0.,p9=0.;//Облачность N
  var q1=0.,q2=0.,q3=0.,q4=0.,q5=0.,q6=0.,q7=0.,q8=0.,q9=0.;//type_poch,ff

  koef_b = parseFloat(e0)-((-0.2)*T0);//подумать над 0.2
// koef_b будет являться X-ом для входа в график по облачности

//Условия по облачности
  if (N>=8 && N<=10 && type_N=="n")
  {
    p1=a[0]*koef_b*koef_b+(b[0]*koef_b)+(c[0]);
    kk=p1;
  }
  if (N>=8 && N<=10 && type_N=="s")
  {
    p2=a[1]*koef_b*koef_b+(b[1]*koef_b)+(c[1]);
    kk=p2;
  }
  if (N>=8 && N<=10)
  {
    if (type_N=="v")
    {
      p5=a[4]*koef_b*koef_b+(b[4]*koef_b)+(c[4]);
      kk=p5;
    }
  }
  if (N==6 || N==7)
  {
    if (type_N=="n")
    {
      p3=a[2]*koef_b*koef_b+(b[2]*koef_b)+(c[2]);
      kk=p3;
    }
    else
    {
      p4=a[3]*koef_b*koef_b+(b[3]*koef_b)+(c[3]);
      kk=p4;
    }
    if (type_N=="v")
    {
      p6=a[5]*koef_b*koef_b+(b[5]*koef_b)+(c[5]);
      kk=p6;
    }
  }
  if (N==4 || N==5)
  {
    if (type_N=="n")
    {
      p5=a[4]*koef_b*koef_b+(b[4]*koef_b)+(c[4]);
      kk=p5;
    }
    if (type_N=="s")
    {
      p6=a[5]*koef_b*koef_b+(b[5]*koef_b)+(c[5]);
      kk=p6;
    }
    if (type_N=="v")
    {
      p8=a[7]*koef_b*koef_b+(b[7]*koef_b)+(c[7]);
      kk=p8;
    }
  }
  if (N==2 || N==3)
  {
    if (type_N=="n")
    {
      p7=a[6]*koef_b*koef_b+(b[6]*koef_b)+(c[6]);
      kk=p7;
    }
    if (type_N=="s")
    {
      p8=a[7]*koef_b*koef_b+(b[7]*koef_b)+(c[7]);
      kk=p8;
    }
    if (type_N=="v")
    {
      p9=a[8]*koef_b*koef_b+(b[8]*koef_b)+(c[8]);
      kk=p9;
    }
  }
  if (N==0 || N==1)
  {
    if (type_N=="o")
    {
      p9=a[8]*koef_b*koef_b+(b[8]*koef_b)+(c[8]);
      kk=p9;
    }
  }

//-----------------------------------------------
//Условия по увлажнению поверхности и ветру, где p'*'=kk это уже X.
//Мокрая----------------------------------------------------
  if (type_poch=="Мокрая")
  {
    if (ff>=0 && ff<=5)
    {
      q1=u[0]*kk+(r[0]);
      qq=q1;
    }
  }else
//Влажная---------------------------------------------------
  if (type_poch=="Влажная" && ff>=3 && ff<=5)
  {
    q2=u[1]*kk+(r[1]);
    qq=q2;
  }else
  if (type_poch=="Влажная" && ff==2)
  {
    q3=u[2]*kk+(r[2]);
    qq=q3;
  }else
  if (type_poch=="Влажная" && ff<=1)
  {
    q4=u[3]*kk+(r[3]);
    qq=q4;
  }else
//Сухая-----------------------------------------------------
  if (type_poch=="Сухая" && ff>4 && ff<=5)
  {
    q5=u[4]*kk+(r[4]);
    qq=q5;
  }else
  if (type_poch=="Сухая" && ff>3)
  {
    q6=u[5]*kk+(r[5]);
    qq=q6;
  }else
  if (type_poch=="Сухая" && ff>2)
  {
    q7=u[6]*kk+(r[6]);
    qq=q7;
  }else
  if (type_poch=="Сухая" && ff>1)
  {
    q8 = (u[7]*kk)+(r[7]);
    qq = q8;
  }else
  if (type_poch=="Сухая" && ff<=1)
  {
    q9 = u[8]*kk+(r[8]);
    qq = q9;
  }



//-----------------------------------------------------------
//Условия для прямых t_is_vs, и широта для 19 часов-----------------
//-------------------------------------------------
  if (t_is==19)
  {
    if (t_is_vs==8 && mes==6)
    {
      x=((qq)-(d[2]))/(p[2]);
      return (-x)+(Tsr);
    }
    if (t_is_vs==8 && mes==7)
    {
      x=((qq)-(d[2]))/(p[2]);
      return (-x)+(Tsr);
    }
    if (t_is_vs==9 && mes==6)
    {
      x=((qq)-(d[3]))/(p[3]);
      return (-x)+(Tsr);
    }
    if (t_is_vs==9 && mes==7)
    {
      x=((qq)-(d[3]))/(p[3]);
      return (-x)+(Tsr);
    }
    if (t_is_vs==10 || t_is_vs==11)
    {
      if (mes==4 || mes==5 || mes==8 || mes==9)
      {
        x=((qq)-(d[4]))/(p[4]);
        return (-x)+(Tsr);
      }
    }

    if (t_is_vs==12 || t_is_vs==13)
    {
      if (mes==3 || mes==10 || mes==11)
      {
        x=((qq)-(d[5]))/(p[5]);
        return (-x)+(Tsr);
      }
    }
  }

//------------------------------------------------------
  if (t_is==13)
  {
    if (t_is_vs==14 || t_is_vs==15)
    {
      if (mes>=4 && mes<=8)
      {
        x=((qq)-(d[6]))/(p[6]);
        return (-x)+(Tsr);
      }
    }else
    if (t_is_vs>15 && t_is_vs<=18)
    {
      if (mes==3)
      {
        x=((qq)-(d[7]))/(p[7]);
        return (-x)+(Tsr);
      }else
      if (mes>=9 && mes<=11)
      {
        x=((qq)-(d[7]))/(p[7]);
        return (-x)+(Tsr);
      }
    }
  }else
//-------------------------------------------------------
  if (t_is>=16 && t_is<=23)
  {
    if (t_is_vs>=4 && t_is_vs<6)
    {
      x=((qq)-(d[0]))/(p[0]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs>=6 && t_is_vs<8)
    {
      x=((qq)-(d[1]))/(p[1]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==8)
    {
      x=((qq)-(d[2]))/(p[2]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==9)
    {
      x=((qq)-(d[3]))/(p[3]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==10 || t_is_vs==11)
    {
      x=((qq)-(d[4]))/p[4];
      return (-x)+(Tsr);
    }else
    if (t_is_vs==12 || t_is_vs==13)
    {
      x=((qq)-(d[5]))/(p[5]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==14 || t_is_vs==15)
    {
      x=((qq)-(d[6]))/(p[6]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==16 || t_is_vs==17 || t_is_vs==18)
    {
      x=((qq)-(d[7]))/(p[7]);
      return (-x)+(Tsr);
    }
  }



//-------------------------------------------------------
  if (t_is>10 && t_is<16)
  {
    if (t_is_vs>=4 && t_is_vs<6)
    {
      x=((qq)-(d[0]))/(p[0]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs>=6 && t_is_vs<8)
    {
      x=((qq)-(d[1]))/(p[1]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==8)
    {
      x=((qq)-(d[2]))/(p[2]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==9)
    {
      x=((qq)-(d[3]))/(p[3]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==10 || t_is_vs==11)
    {
      x=((qq)-(d[4]))/(p[4]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==12 || t_is_vs==13)
    {
      x=((qq)-(d[5]))/(p[5]);

      return (-x)+(Tsr);
    }else
    if (t_is_vs==14 || t_is_vs==15)
    {
      x=((qq)-(d[6]))/(p[6]);
      return (-x)+(Tsr);
    }else
    if (t_is_vs==16 || t_is_vs==17 || t_is_vs==18)
    {
      x=((qq)-(d[7]))/(p[7]);
      // return (qq);
      return (-x)+(Tsr);
    }
  }

  return badRet();
}


//Функция прогнозирования тумана или дымки  методом Берлянда
function getTumDim(T_tum, T_min)
{
  if( !isValidNum( T_min )   ||
      !isValidNum( T_tum )   )
  {
    return badRet();
  }
  var ret, vs_str;

  T_tum=parseFloat(T_tum);
  T_min=parseFloat(T_min);

  // print(T_min);

  if (T_tum<T_min)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0, "Туман не ожидается", 0,1);

    return "Туман не ожидается";
  }

  if (T_tum>T_min)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 12, "Туман", 0,1);
    return "Туман ожидается";
  }
  if ((parseFloat(T_min)-parseFloat(T_tum))>0. && (parseFloat(T_min)-parseFloat(T_tum))<1.)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 10, "Дымка", 0,1);
    return "Ожидается дымка";
  }
  else return badRet();
}


//Функция прогнозирования видимости в тумане или дымке методом Берлянда
function getVidTumDim(T_min,T_tum)
{
  if( !isValidNum( T_min )   ||
      !isValidNum( T_tum )   )
  {
    return badRet();
  }
  var ret,vs_str;

  if ((parseFloat(T_min)-parseFloat(T_tum))>=1)
  {
    return "Видимость менее 2 км не ожидается";
  }
  if ((parseFloat(T_min)-parseFloat(T_tum))>0 && (parseFloat(T_min)-parseFloat(T_tum))<1)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, "1500", "Видимость в тумане 1000-2000м", 0,1);
    return "Видимость 1-2 км";

  }

  if ((parseFloat(T_min)-parseFloat(T_tum))>=-1 && (parseFloat(T_min)-parseFloat(T_tum))<=0)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, "750", "Видимость в тумане 500-1000м", 0,1);
    return "Видимость 500-1000 м";

  }
  if ((parseFloat(T_min)-parseFloat(T_tum))>=-2 && (parseFloat(T_min)-parseFloat(T_tum))<-1)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, "350", "Видимость в тумане 200-500м", 0,1);
    return "Видимость 200-500 м";

  }
  if ((parseFloat(T_min)-parseFloat(T_tum))<-2)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, "150", "Видимость в тумане 50-200м", 0,1);
    return "Видимость 50-200 м";

  }

  else return badRet();
}

