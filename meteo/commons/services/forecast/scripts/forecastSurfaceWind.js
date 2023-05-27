
function badvalue()
{
  return -9999;
}

function veter_skor_Z_P(lap_P,Pdelta_0X,Pdelta_0Y,type_pp)
{
  if( !isValidNum(lap_P)||
      !isValidNum(Pdelta_0X) ||
      !isValidNum(Pdelta_0Y))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var result;

  result = getVeterZDopData(1,type_pp,lap_P,Pdelta_0X,Pdelta_0Y,0,0,0);
  if (!isValidNum(result))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  if (result==badvalue())
  {
    return "Ветер слабый переменных направлений";
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11002, result.toFixed(2), result.toFixed(2), 0,1);

  return result;
}

function veter_nap_Z_P(lap_P,Pdelta_0X,Pdelta_0Y,type_pp)
{
  if( !isValidNum(lap_P)||
      !isValidNum(Pdelta_0X) ||
      !isValidNum(Pdelta_0Y))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var result;

  lap_P=parseFloat(lap_P);
  Pdelta_0X=parseFloat(Pdelta_0X);
  Pdelta_0Y=parseFloat(Pdelta_0Y);
  type_pp=parseFloat(type_pp);

  result = getVeterZDopData(2,type_pp,lap_P,Pdelta_0X,Pdelta_0Y,0,0,0);
  if (!isValidNum(result))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  if (result==badvalue())
  {
    return "Ветер слабый переменных направлений";
  }
  return result;
  result=result+180.;
  if(result>360.)result-=360.;
  if(result<0.)result+=360.;
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11001, result.toFixed(2), result.toFixed(2), 0,1);

  return result;

}


function getVeterZDopData( tip,type_pp,lap_P,Pdelta_0X,Pdelta_0Y,fl_adapt,koef_ff,koef_OX)
{
  var i;
  var delta,vdelta_OX,delta_n,Pdelta,fi,k;//поправочный к-т
  var alfa,fdelta_tg;
  var skor, napr;

  fi = 60.;
  fi=(fi)*Math.PI/180.;//широта пункта

  if(Pdelta_0X==0)Pdelta_0X=0.0001;
  if(Pdelta_0Y==0)Pdelta_0Y=0.0001;

  Pdelta=Math.sqrt((Pdelta_0X)*(Pdelta_0X)+(Pdelta_0Y)*(Pdelta_0Y));
//  return Pdelta_0X;

  if (Pdelta==0) Pdelta=0.0001;

//  if(fi==0) fi=0.0001;
  delta_n=5.4/Math.sin(fi);
//  return Pdelta;
  if(delta_n==0) delta_n=0.0001;
  vdelta_OX=delta_n*Pdelta;


  if(Math.abs(lap_P)<2) delta=0;
  if(lap_P>=2) delta=-1.5/delta_n;
  if((lap_P<=-2)&& fi>=55.) delta=4.5/delta_n;
//  if((lap_P<=-2)&& fi<55.) delta=6./delta_n;
  vdelta_OX+=delta;

  switch ((type_pp))
  {
  case "Cуша":
      k=0.55;
      i=30.;
      break;
  case "Море":
      k=0.75;
      i=15.;
      break;
  case "Холмистая":
      k=0.55;
      i=45.;
      break;
  default:
      k=0.55;
      i=30.;
      break;
  }

  if(vdelta_OX>15) k=1;
  vdelta_OX*=k;



  alfa=Math.atan(Math.abs(Pdelta_0Y)/Math.abs(Pdelta_0X));

  if((Pdelta_0X>=0)&&(Pdelta_0Y>0)) fdelta_tg=360.-alfa*180./Math.PI;
  if((Pdelta_0X>0)&&(Pdelta_0Y<=0)) fdelta_tg=alfa*180./Math.PI;
  if((Pdelta_0X<=0)&&(Pdelta_0Y<0)) fdelta_tg=180.-alfa*180./Math.PI;
  if((Pdelta_0X<0)&&(Pdelta_0Y>=0)) fdelta_tg=180.+alfa*180./Math.PI;
  fdelta_tg-=i;
  //адаптация
  if(fl_adapt)
     if(vdelta_OX>0)
     {
       fdelta_tg=fdelta_tg+koef_ff;
       vdelta_OX=vdelta_OX+koef_OX;
     }

  //приведение
  if(fdelta_tg>360.)fdelta_tg-=360.;
  if(fdelta_tg<0.)fdelta_tg+=360.;

  if((fdelta_tg<0.)&&(fdelta_tg>360.)) return badvalue();
//  return vdelta_OX;

  if(vdelta_OX<0.) vdelta_OX=0.;
  if(vdelta_OX>90.) vdelta_OX=90.;


  if((vdelta_OX>=0)&&(vdelta_OX<=3.)) {return badvalue();}

  if (tip==1)
  {
    return skor=ftoi_norm(vdelta_OX);
  }
  if (tip==2)
  {
    return napr=ftoi_norm(fdelta_tg);
  }

}
