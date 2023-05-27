
function badvalue()
{
  return -9999;
}


//прогноз количества облаков нижнего, среднего и верхнего ярусов
function getVlaznostMagnusCloud( T, Td )
 {
  if(!isValidNum(T) ||
     !isValidNum(Td)  )
     return badvalue();
   
  
    T = parseFloat(T);
    Td = parseFloat(Td);


    var tmp1,tmp2, tmp11, tmp22;
    var  a,b,fin;

    if (T<-10.) {
      a=9.5;
      b=265.5;
      
    }
    else {
      a=7.63;
      b=241.9;
    }
    
    tmp1=(a*Td)/(parseFloat(b)+parseFloat(Td));
    tmp2=(a*T)/(parseFloat(b)+parseFloat(T));

    tmp11=tmp1-tmp2;
    tmp22= Math.pow(10.,tmp11);


    if (tmp22<0)
        return 0;
    if (tmp22>1)
        return 100;
    else
      return tmp22*100;
 }

 
function getHumidityCloud(T, D)
{
  if(!isValidNum(T) ||
     !isValidNum(D)  )
     return badvalue();

  T = parseFloat(T);
  D = parseFloat(D);
  var res, fin;

  res = getVlaznostMagnusCloud(T,(T-D));
  return res;
}


function getFormul_low(T1000,DD1000,T850,DD850)
{
  if( !isValidNum(T1000)  ||
      !isValidNum(DD1000)  ||
      !isValidNum(T850)  ||
      !isValidNum(DD850) )
      return badvalue();
  var f0, f1000, f925, f850, result;
  f1000 = getHumidityCloud(T1000,DD1000);
  f850 = getHumidityCloud(T850,DD850);

  if( !isValidNum(f1000) ||
      !isValidNum(f850) )
      return badvalue();

  var f_sr, N_low, vs, fin;

  f_sr=(parseFloat(f1000)+parseFloat(f850))/2.;

  N_low=-2.+3.3*f_sr*.01;
  N_low*=10.;

  if (N_low<0.) N_low=0.;
  if (N_low>10.) N_low=10.;
  result = ftoi_norm(N_low);


  return result*10;
}


function getFormul_middle(T850,DD850,T500,DD500,T1000,DD1000)
{
  if( !isValidNum(T850) ||
      !isValidNum(DD850) ||
      !isValidNum(T500) ||
      !isValidNum(T1000) ||
      !isValidNum(DD1000) ||
      !isValidNum(DD500) )
        return badvalue();
  var f850, f700,f500,result;
  f850 = getHumidityCloud(T850,DD850);
//  f700 = getHumidityCloud(T700,DD700);
  f500 = getHumidityCloud(T500,DD500);

  if( !isValidNum(f850)  ||
//      !isValidNum(f700) ||
      !isValidNum(f500)  )
      return badvalue();

  var f_sr, N_mid, vs, fin;

  f_sr=(parseFloat(f850)+/*f700*/+parseFloat(f500))/2.;

  N_mid=-0.7+2.0*f_sr*.01;
  N_mid*=10.;

  if (N_mid<0.) N_mid=0.;
  if (N_mid>10.) N_mid=10.;

//  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20012, ret, ret, 0,1);
  result = ftoi_norm(N_mid);
  //записываем в базу наибольшее значение количества облачности нижнего и среднего слоев

  var compar = getFormul_low(T1000,DD1000,T850,DD850);
  if( result > compar)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20011, result, "На нижнем-среднем уровнях", 0,200);
  }
  if( result < compar)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20012, compar, "На нижнем уровне", 0,200);
  }

//  if( result < compar)
//  {
//      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20011, compar, "На нижнем-среднем уровнях", 0,200);
//  }

  return result*10;
}




function getFormul_high(T500,DD500,T300,DD300)
{

  if( !isValidNum(T500) ||
      !isValidNum(DD500)  ||
//      !isValidNum(T400) ||
//      !isValidNum(DD400) ||
      !isValidNum(T300)  ||
      !isValidNum(DD300) )
        return badvalue();
  var f500,f400,f300,result;
  f500 = getHumidityCloud(T500,DD500);
//  f400 = getHumidityCloud(T400,DD400);
  f300 = getHumidityCloud(T300,DD300);

  var f_sr, N_high, vs, fin;

  f_sr=(parseFloat(f500)+/*f400*/+parseFloat(f300))/2.;


  N_high=-0.4+1.7*f_sr*.01;
  N_high*=10.;

  if (N_high<0.) N_high=0.;
  if (N_high>10.) N_high=10.;

  result = ftoi_norm(N_high);
//  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20012, result*10, "На верхнем уровне", 0, 1);

  return result*10;
}
