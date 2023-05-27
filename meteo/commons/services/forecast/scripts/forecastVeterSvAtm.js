
function napr_veter_Sv_atm(uroven, lap_P, Pdelta_0X, Pdelta_0Y)//направление
{
 
    if (!isValidNum(lap_P)||
        !isValidNum(Pdelta_0X)||
        !isValidNum(Pdelta_0Y) )
    {
      return "Метод не работает с заданными значениями параметров";
    }

    var result;
    uroven=parseFloat(uroven);
    lap_P=parseFloat(lap_P);
    Pdelta_0X=parseFloat(Pdelta_0X);
    Pdelta_0Y=parseFloat(Pdelta_0Y);

       //  lap_P=lap_P*9.e10;
  //  Pdelta_0Y=Pdelta_0Y*1.e5; // на 100 км
  //  Pdelta_0X=Pdelta_0X*1.e5; // на 100 км
  result = getVeterHDopData(1,uroven,lap_P,Pdelta_0X,Pdelta_0Y,0,0,0);
  if (!isValidNum(result))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  if (result==badvalue())
  {
    return "Ветер слабый, переменных направлений";
  }
  result=result+180.;
  if(result>360.)result-=360.;
  if(result<0.)result+=360.;
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11001, result.toFixed(2), result.toFixed(2), uroven,100);

  return result;

}

function skor_veter_Sv_atm(uroven,lap_P, Pdelta_0X, Pdelta_0Y)//скорость
{
    if(  !isValidNum(lap_P)||
        !isValidNum(Pdelta_0X)||
        !isValidNum(Pdelta_0Y) )
    {
      return "Метод не работает с заданными значениями параметров";
    }

    uroven=parseFloat(uroven);
    lap_P=parseFloat(lap_P);
    Pdelta_0X=parseFloat(Pdelta_0X);
    Pdelta_0Y=parseFloat(Pdelta_0Y);
    var result;
//    lap_P=lap_P*9.e10;
//    Pdelta_0Y=Pdelta_0Y*1.e5; // на 100 км
//    Pdelta_0X=Pdelta_0X*1.e5; // на 100 км

    result = getVeterHDopData(2,uroven,lap_P,Pdelta_0X,Pdelta_0Y,0,0,0);
    if (!isValidNum(result))
    {
      return "Метод не работает с заданными значениями параметров";
    }
    if (result==badvalue())
    {
      return "Ветер слабый, переменных направлений";
    }
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, result.toFixed(2), result.toFixed(2), uroven,100);

    return result;


}

function getVeterHDopData(tip,  type_uroven, lap_P, Pdelta_0X, Pdelta_0Y, fl_adapt, koef_ff, koef_OX)
{
  
  fi_ = obj.getStationData().fi;
  
  var fi, Pdelta, delta_vs, delta_n, vdelta_OX, delta, alfa, fdelta_tg, skor, napr;
  fi=(fi_)*Math.PI/180.;//широта пункта в радианах, fi_h - градусы, fi_mim - минуты
 // print(fi_,fi);
  

  if(Pdelta_0X==0)Pdelta_0X=0.0001;
  if(Pdelta_0Y==0)Pdelta_0Y=0.0001;

  Pdelta=Math.sqrt((Pdelta_0X*Pdelta_0X)+(Pdelta_0Y*Pdelta_0Y)); // длина вектора, т.е. градиент давления - изменение давления в ГПа через 1 градус широты.
  if(Pdelta==0) Pdelta=0.0001;
//  return Pdelta;

  delta_vs=4./Math.abs(Pdelta);
  delta_n=27./(Math.sin(fi));
//  return delta_n;


  delta_n=delta_n/delta_vs;
  vdelta_OX=delta_n;

  if(lap_P>=4.) delta=2./3.;
  if(lap_P<=-4.) delta=3./2.;
  if(lap_P<4.) delta=1.;

  vdelta_OX=vdelta_OX*delta;
//  return vdelta_OX;


  alfa=Math.atan(Math.abs(Pdelta_0Y)/Math.abs(Pdelta_0X));


  if((Pdelta_0X>=0)&&(Pdelta_0Y>0)) fdelta_tg=360.-alfa*180./Math.PI;
  if((Pdelta_0X>0)&&(Pdelta_0Y<=0)) fdelta_tg=alfa*180./Math.PI;
  if((Pdelta_0X<=0)&&(Pdelta_0Y<0)) fdelta_tg=180.-alfa*180./Math.PI;
  if((Pdelta_0X<0)&&(Pdelta_0Y>=0)) fdelta_tg=180.+alfa*180./Math.PI;

      if(fdelta_tg>360.)fdelta_tg-=360.;
      if(fdelta_tg<0.)fdelta_tg+=360.;

  if(fl_adapt==1)
  {
    if(type_uroven==925)
    if(vdelta_OX>0)
    {
      fdelta_tg=(fdelta_tg)+(koef_ff);
      vdelta_OX=(vdelta_OX)+(koef_OX);
     }
  }

  //приведение
  if(fdelta_tg>360.)fdelta_tg-=360.;
  if(fdelta_tg<0.)fdelta_tg+=360.;

  if((fdelta_tg<0.)&&(fdelta_tg>360.)) return badvalue();

//  return vdelta_OX;


  if(vdelta_OX<0.) vdelta_OX=0.;
  if(vdelta_OX>90.) vdelta_OX=90.;


  if((vdelta_OX>=0)&&(vdelta_OX<=3)) {return badvalue();}

  if(tip==1)
  {
    return napr=ftoi_norm(fdelta_tg);
  }
  if(tip==2)
  {
    return skor=ftoi_norm(vdelta_OX);
  }

}



//////////
