var A1 = [2.1, -1.5, 0.4];


//TypeProc - тип синоптической ситуации; FormaObl - форма облачности; srok -срок прогноза
function oneHourVNG015( FormaObl, TypeProc, period,h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7 )
{
  if( !isValidNum( h_ngo1 )   ||
      !isValidNum( h_ngo2 )   ||
      !isValidNum( h_ngo3 )   ||
      !isValidNum( h_ngo4 )   ||
      !isValidNum( h_ngo5 )   ||
      !isValidNum( h_ngo6 )   ||
      !isValidNum( h_ngo7 ) )
  {
    return "Метод не работает с заданными входными значениями";
  }

  if( h_ngo1 == 0.    &&
      h_ngo2 == 0.    &&
      h_ngo3 == 0.    &&
      h_ngo4 == 0.    &&
      h_ngo5 == 0.    &&
      h_ngo6 == 0.    &&
      h_ngo7 == 0.  )
  {
    return "Значение ВНГО равны 0. Метод предназначен для расчета ВНГО менее 300м (и более 0)";
  }
  var p, i,j,formul;
  var hSr = [0 , 0, 0];
  var hF = [0, 0, 0, 0];
  var A = [0, 0, 0];
  var h_ngo = [h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7];


  A = koeff_A(TypeProc,period,FormaObl);
  p = koeff_P(TypeProc,period,FormaObl);

  
  if( !isValidNum( p )  )
  {
      return "Метод не работает с заданными входными значениями";
  }
  if( !isValidNum( A[0] )   )
  {
      return "Метод не работает с заданными входными значениями";
  }


  for(j=0;j<3;j++)
  {
    for(i=j;i<(parseFloat(p)+parseFloat(j));i++)
    {
        hSr[j] += parseFloat(h_ngo[i]);
        hSr[j] /= p;
    }
  }

  hF[0] = ftoi_norm(parseFloat(A[0]*hSr[0])+parseFloat(A[1]*hSr[1])+parseFloat(A[2]*hSr[2]));

  if( hF[0] < 0.)
  {
     return "Ошибка! Недопустимые значения ретроспективного ряда ВНГО";
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, hF[0], hF[0], 0,200);

  return hF[0];

}

function oneHourVNG030( FormaObl, TypeProc, period,h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7 )//TypeProc - тип синоптической ситуации; FormaObl - форма облачности; srok -срок прогноза
{
  if( !isValidNum( h_ngo1 )   ||
      !isValidNum( h_ngo2 )   ||
      !isValidNum( h_ngo3 )   ||
      !isValidNum( h_ngo4 )   ||
      !isValidNum( h_ngo5 )   ||
      !isValidNum( h_ngo6 )   ||
      !isValidNum( h_ngo7 ) )
  {
    return "Метод не работает с заданными входными значениями";
  }

  if( h_ngo1 == 0.    &&
      h_ngo2 == 0.    &&
      h_ngo3 == 0.    &&
      h_ngo4 == 0.    &&
      h_ngo5 == 0.    &&
      h_ngo6 == 0.    &&
      h_ngo7 == 0.  )
  {
    return "Значение ВНГО равны 0. Метод предназначен для расчета ВНГО менее 300м (и более 0)";
  }
  var p, i,j,formul;
  var hSr = [0 , 0, 0];
  var hF = [0, 0, 0, 0];
  var A = [0, 0, 0];
  var h_ngo = [h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7];

  A = koeff_A(TypeProc,period,FormaObl);
  p = koeff_P(TypeProc,period,FormaObl);
  if( !isValidNum( p )  )
  {
    return "Метод не работает с заданными входными значениями";
  }
  if( !isValidNum( A[0] )   )
  {
    return "Метод не работает с заданными входными значениями";
  }

  for(j=0;j<3;j++)
  {
    for(i=j;i<(parseFloat(p)+parseFloat(j));i++)
    {
        hSr[j] += parseFloat(h_ngo[i]);
        hSr[j] /= p;
    }
  }

  hF[0] = ftoi_norm(parseFloat(A[0]*hSr[0])+parseFloat(A[1]*hSr[1])+parseFloat(A[2]*hSr[2]));
  hF[1] = ftoi_norm(parseFloat(A[0]*hF[0])+parseFloat(A[1]*hSr[0])+parseFloat(A[2]*hSr[1]));

  if( hF[1] < 0.)
  {
     return "Ошибка! Недопустимые значения ретроспективного ряда ВНГО";
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, hF[1], hF[1], 0,200);
  return hF[1];
}

function oneHourVNG045( FormaObl, TypeProc, period,h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7 )//TypeProc - тип синоптической ситуации; FormaObl - форма облачности; srok -срок прогноза
{
  if( !isValidNum( h_ngo1 )   ||
      !isValidNum( h_ngo2 )   ||
      !isValidNum( h_ngo3 )   ||
      !isValidNum( h_ngo4 )   ||
      !isValidNum( h_ngo5 )   ||
      !isValidNum( h_ngo6 )   ||
      !isValidNum( h_ngo7 ) )
  {
    return "Метод не работает с заданными входными значениями";
  }

  if( h_ngo1 == 0.    &&
      h_ngo2 == 0.    &&
      h_ngo3 == 0.    &&
      h_ngo4 == 0.    &&
      h_ngo5 == 0.    &&
      h_ngo6 == 0.    &&
      h_ngo7 == 0.  )
  {
    return "Значение ВНГО равны 0. Метод предназначен для расчета ВНГО менее 300м (и более 0)";
  }
  var p, i,j,formul;
  var hSr = [0 , 0, 0];
  var hF = [0, 0, 0, 0];
  var A = [0, 0, 0];
  var h_ngo = [h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7];

  A = koeff_A(TypeProc,period,FormaObl);
  p = koeff_P(TypeProc,period,FormaObl);
  if( !isValidNum( p )  )
  {
    return "Метод не работает с заданными входными значениями";
  }
  if( !isValidNum( A[0] )   )
  {
    return "Метод не работает с заданными входными значениями";
  }

  for(j=0;j<3;j++)
  {
    for(i=j;i<(parseFloat(p)+parseFloat(j));i++)
    {
        hSr[j] += parseFloat(h_ngo[i]);
        hSr[j] /= p;
    }
  }

  hF[0] = ftoi_norm(parseFloat(A[0]*hSr[0])+parseFloat(A[1]*hSr[1])+parseFloat(A[2]*hSr[2]));
  hF[1] = ftoi_norm(parseFloat(A[0]*hF[0])+parseFloat(A[1]*hSr[0])+parseFloat(A[2]*hSr[1]));
  hF[2] = ftoi_norm(parseFloat(A[0]*hF[1])+parseFloat(A[1]*hF[0])+parseFloat(A[2]*hSr[0]));


  if( hF[2] < 0.)
  {
     return "Ошибка! Недопустимые значения ретроспективного ряда ВНГО";
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, hF[2], hF[2], 0,200);
  return hF[2];
}

function oneHourVNG060( FormaObl, TypeProc, period,h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7 )//TypeProc - тип синоптической ситуации; FormaObl - форма облачности; srok -срок прогноза
{
  if( !isValidNum( h_ngo1 )   ||
      !isValidNum( h_ngo2 )   ||
      !isValidNum( h_ngo3 )   ||
      !isValidNum( h_ngo4 )   ||
      !isValidNum( h_ngo5 )   ||
      !isValidNum( h_ngo6 )   ||
      !isValidNum( h_ngo7 ) )
  {
    return "Метод не работает с заданными входными значениями";
  }

  if( h_ngo1 == 0.    &&
      h_ngo2 == 0.    &&
      h_ngo3 == 0.    &&
      h_ngo4 == 0.    &&
      h_ngo5 == 0.    &&
      h_ngo6 == 0.    &&
      h_ngo7 == 0.  )
  {
    return "Значение ВНГО равны 0. Метод предназначен для расчета ВНГО менее 300м (и более 0)";
  }
  var p, i,j,formul;
  var hSr = [0 , 0, 0];
  var hF = [0, 0, 0, 0];
  var A = [0, 0, 0];
  var h_ngo = [h_ngo1,h_ngo2,h_ngo3,h_ngo4,h_ngo5,h_ngo6,h_ngo7];

  A = koeff_A(TypeProc,period,FormaObl);
  p = koeff_P(TypeProc,period,FormaObl);
  if( !isValidNum( p )  )
  {
    return "Метод не работает с заданными входными значениями";
  }
  if( !isValidNum( A[0] )   )
  {
    return "Метод не работает с заданными входными значениями";
  }

  for(j=0;j<3;j++)
  {
    for(i=j;i<(parseFloat(p)+parseFloat(j));i++)
    {
        hSr[j] += parseFloat(h_ngo[i]);
        hSr[j] /= p;
    }
  }

  hF[0] = ftoi_norm(parseFloat(A[0]*hSr[0])+parseFloat(A[1]*hSr[1])+parseFloat(A[2]*hSr[2]));
  hF[1] = ftoi_norm(parseFloat(A[0]*hF[0])+parseFloat(A[1]*hSr[0])+parseFloat(A[2]*hSr[1]));
  hF[2] = ftoi_norm(parseFloat(A[0]*hF[1])+parseFloat(A[1]*hF[0])+parseFloat(A[2]*hSr[0]));
  hF[3] = ftoi_norm(parseFloat(A[0]*hF[2])+parseFloat(A[1]*hF[1])+parseFloat(A[2]*hF[0]));


  if( hF[3] < 0.)
  {
     return "Ошибка! Недопустимые значения ретроспективного ряда ВНГО";
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, hF[3], hF[3], 0,200);

  return hF[3];
}


function koeff_A( TypeProc, Period, FormaObl)
{
  var A = [0, 0, 0];

  if ( Period=="Холодный" ){

    if(FormaObl=="Ns"||FormaObl=="Sc")
    {
       if(TypeProc=="Теплый фронт" || TypeProc=="Внутримассовый процесс")
       {
         A = [1.9, -1.1, 0.2];
         return A;
       }
       if(TypeProc=="Холодный фронт")
       {
         A = [2.1, -1.4, 0.3];
         return A;
       }
     }
     if(FormaObl=="St")
     {
       if(TypeProc=="Теплый фронт"|| TypeProc=="Внутримассовый процесс")
       {
         A = [1.9, -1.1, 0.2];
         return A;
       }
       if(TypeProc=="Холодный фронт")
       {
         A = [1.8, -0.9, 0.1];
         return A;
       }
     }
     else
     {
         return "Метод не работает с заданными значениями формы облачности";
     }

  }else if (Period=="Теплый"){
    if(TypeProc=="Теплый фронт")//Теплый фронт
       {
         A = [2.1, -1.5, 0.4];
         return A1;
       }
       if(TypeProc=="Холодный фронт")//Холодный фронт
       {
         A =[2.1, -1.4, 0.3];
         return A;
       }
       if(TypeProc=="Внутримассовый процесс")//Внутримассовый процесс
       {
         A = [1.8, -0.9, 0.1];
         return A;
       }
       else
       {
         return  "Метод не работает с заданными входными значениями";
       }
  }

}

function koeff_P( TypeProc, Period, FormaObl)
{
  var p;
  switch(Period)
  {
    case "Холодный":
      if(FormaObl=="Ns"||FormaObl=="Sc")
      {
        if(TypeProc=="Теплый фронт")
        {
          p=5;
          return p;
        }
        if(TypeProc=="Холодный фронт")
        {
          p=5;
          return p;
        }
        if(TypeProc=="Внутримассовый процесс")
        {
          p=3;
          return p;
        }
      }
      else
      {
        if(TypeProc=="Теплый фронт")//Теплый фронт
        {
          p=3;
          return p;
        }
        if(TypeProc=="Холодный фронт")//Холодный фронт
        {
          p=3;
          return p;
        }
        if(TypeProc=="Внутримассовый процесс")//Внутримассовый
        {
          p=3;
          return p;
        }
      }
      break;

    default:
      if(TypeProc=="Теплый фронт" )//Теплый фронт
      {
        p=3;
        return p;
      }
      if(TypeProc=="Холодный фронт" )//Холодный фронт
      {
        p=5;
        return p;
      }
      if(TypeProc=="Внутримассовый процесс" )//Внутримассовый процесс
      {
        p=5;
        return p;
      }
      break;
  }

}

