// 13.2 Обледенение на уровнях 850, 700 и 500 гПа

//Функция для прогноза обледенения в облаках Tob по значениям воздушной скорости полёта V и температуры воздуха на высоте полета T. рис.122

function pic122( Vs )
{
  if( !isValidNum( Vs )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var a=-0.00003326;
  var b=0.009939;
  var c=-0.9883;
  var y;

  if (Vs<200. || Vs>1000.)
  {
    return "Метод не работает на скоростях полета менее 200 и более 1000 км/ч";
  }else
  {
    y=parseFloat(a*Vs*Vs)+parseFloat(b*Vs)+parseFloat(c);
    return y;
  }
}

function ObledUr850(T850,D850,Vs,W850pr)
{
  if( !isValidNum( T850 )  ||
      !isValidNum( D850 )  ||
      !isValidNum( W850pr )||
      !isValidNum( Vs ) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var TObled,L;
  var ret;


  TObled = pic122(Vs);

  if ( !isValidNum(TObled) ){
    return "Метод не работает с заданными значениями параметров";
  }

  if(T850<=TObled)
  {
    L=parseFloat(-0.001*T850)+parseFloat(0.881*D850)+parseFloat(0.11*W850pr)+4.524;
    if(L<0)
    {
      ret="Ожидается обледенение";
      var dataB = ftoi_norm (zondObj.getUr(obj.getStationData(),850,1));
      if(isValidNum(dataB))
      {
        ret="Ожидается обледенение на высоте "+dataB+" м";
        
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11032, dataB,ret, 850,100);
      }
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 1, ret, 850,100);// printf ("Obledenenie  otsutstvuet!\n");
  
    }
    else
    {
        ret="Обледенение не ожидается";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 0, ret, 850,100);// printf ("Obledenenie  otsutstvuet!\n");
    }
  }
  else
  {
    ret="Обледенение не ожидается";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 0, ret, 850,100);
  }

  return ret;

}

function ObledUr700(T700,D700,Vs,W700pr)
{
  if( !isValidNum( T700 ) ||
      !isValidNum( D700 ) ||
      !isValidNum( W700pr ) ||
      !isValidNum( Vs ) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var TObled=0,L=0;
  var ret=0;

  TObled = pic122(Vs);

  

  if ( !isValidNum(TObled) ){
    return "Метод не работает с заданными значениями параметров";
  }

  if( T700 <= TObled )
  {
    L=parseFloat(0.005*T700)+parseFloat(0.708*D700)+parseFloat(0.08*W700pr)-3.876;

    if(L<0)
    {
      var dataB = ftoi_norm(zondObj.getUr(obj.getStationData(),700,1));
      if(isValidNum(dataB))
      {
        ret="Ожидается обледенение на высоте "+dataB+" м";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11032, dataB, ret, 700,100);
      }else{
        ret="Ожидается обледенение";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 1, ret, 700,100);
      }
    }
    else
    {
      ret="Обледенение не ожидается";
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 0, ret, 700,100);
    }// printf ("Obledenenie  otsutstvuet!\n");
  }
  else
  {
    ret="Обледенение не ожидается";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 0, ret, 700,100);
  }
  return ret;
}

function ObledUr500(T500,D500,Vs,W500pr)
{
  if( !isValidNum( T500 )  ||
      !isValidNum( D500 ) ||
      !isValidNum( W500pr ) ||
      !isValidNum( Vs )    )
  {
    return "Метод не работает с заданными значениями параметров";
  }


  var TObled,L;
  var ret;

  TObled = pic122(Vs);
  if ( !isValidNum(TObled) ){
    return "Метод не работает с заданными значениями параметров";
  }
  if(T500<=TObled)
  {
    L=parseFloat(0.052*T500)+parseFloat(0.739*D500)-parseFloat(0.31*W500pr)-2.685;
    if(L<0)
    {
        ret="Ожидается обледенение";
        var dataB = ftoi_norm(zondObj.getUr(obj.getStationData(),500,1));
        if(isValidNum(dataB))
        {           
          ret="Ожидается обледенение на высоте "+dataB+" м";
          dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11032, dataB, ret, 500,100);
        }
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 1, ret, 500,100);
        
    }// printf ("Obledenenie !\n");
    else
    {
      ret="Обледенение не ожидается";
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 0, ret, 500,100);
    }// printf ("Obledenenie  otsutstvuet!\n");
  }
  else
  {
    ret="Обледенение не ожидается";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 0, ret, 500,100);
  }
  return ret;
}

function skorVertDvig850(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W850=0, Lapla_0sredni=0;

  Lapla_0sredni = lapSredni(Lapla0,LaplaAdv);
  if ( !isValidNum(Lapla_0sredni) ){
    return "Метод не работает с заданными значениями параметров";
  }
  W850= -3.5*(parseFloat(Lapla_0sredni)+parseFloat(Lapla0)); /**2.5e11*/
  return W850;
}

function skorVertDvig700(Lapla700,Lapla850,Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla700)||
      !isValidNum(Lapla850)||
      !isValidNum(Lapla0)  ||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W700=0, W850=0;
  W850 = skorVertDvig850(Lapla0,LaplaAdv);
  if ( !isValidNum(W850) ){
    return "Метод не работает с заданными значениями параметров";
  }

  W700= W850-2.1*(parseFloat(Lapla850)+parseFloat(Lapla700));
  return W700;
}

function SkorVertDvig500(Lapla700,Lapla500,Lapla850,Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla850)||
      !isValidNum(Lapla700)||
      !isValidNum(Lapla0)||
      !isValidNum(LaplaAdv)||
      !isValidNum(Lapla500))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W500=0, W700=0;

  W700 = skorVertDvig700(Lapla700,Lapla850,Lapla0,LaplaAdv);
  if ( !isValidNum(W700) ){
    return "Метод не работает с заданными значениями параметров";
  }
  W500= W700-2.8*(parseFloat(Lapla700)+parseFloat(Lapla500));
  return W500;
}


//среднее арифметическое между значением лапласиана давления в пункте расчета
//в конечный момент 12-часового интервала и значением лапласиана давления в
//начальной точке траектории в начальный момент интервала;
function lapSredni(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var sred=0;

  sred = (parseFloat(Lapla0)+parseFloat(LaplaAdv))/2.;
  return sred;
}

