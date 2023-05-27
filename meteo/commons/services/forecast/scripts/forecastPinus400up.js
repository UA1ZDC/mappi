////14.3 для метода болтанки выше 400 гПа (Пинус)

function getDataDD(H_high, gp)
{
  return getData(H_high, gp, 7);
}
function getDataFF(H_high, gp)
{
  return getData(H_high, gp,6);
}
function getDataT(H_high, gp)
{
  return getData(H_high, gp,2);
}

function getData(H_high, gp, type)
{
  var st =  obj.getStationData();
  var coord =  obj.toGeoPoint(gp);
  st.fi = coord.fi;
  st.la = coord.la;
  st.index = ""; //значение в точке адвекции, а не на станции!!!
  if( !isValidNum( H_high )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var ff;
  ff = zondObj.getDataFromHeight(st,H_high,0,type);
  if( !isValidNum( ff ) )
  {
      return "Не удалось определить значение автоматически";
  }
  return ff;
}


function M0To360(d)
{
  while(d<-0.) d+=360.;
  while(d>=360.) d-=360.;
  return d;
}

function convertDFtoUV(tip,dd, ff)
{
  var angl;
  angl = dd * 0.0174533;

  var M_PI = 3.14159265358979323846;



  dd = M0To360(dd);
  if ( 0. <= dd && dd < 90. )
    {
      u = ff*Math.sin(angl);
      v = ff*Math.cos(angl);
    }
  if ( 90. <= dd && dd < 180. )
    {
      u = ff*Math.sin( M_PI - angl );
      v = -ff*Math.cos( M_PI - angl );
    }
  if ( 180. <= dd && dd < 270. )
    {
      u = -ff*Math.sin( angl - M_PI );
      v = -ff*Math.cos( angl - M_PI );
    }
  if ( 270. <= dd && dd < 360. )
    {
      u = -ff*Math.sin( M_PI*2 - angl );
      v = ff*Math.cos( M_PI*2 - angl );
    }

  if( tip === 1)
  {
      return u;
  }
  if( tip === 2)
  {
      return v;
  }

}

function getBetta( H_high,H_low,ff_high,ff_low,dd_high,dd_low)
{
  if( !isValidNum( H_high )   ||
      !isValidNum( H_low )    ||
      !isValidNum( ff_high )   ||
      !isValidNum( ff_low )    ||
      !isValidNum( dd_high )   ||
      !isValidNum( dd_low )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  var uUp = convertDFtoUV(1,dd_high, ff_high);
  var vUp = convertDFtoUV(2,dd_high, ff_high);
 // print("uv up",uUp,vUp,dd_high, ff_high );
  var uLo = convertDFtoUV(1,dd_low, ff_low);
  var vLo = convertDFtoUV(2,dd_low, ff_low);
 // print("uv lo",uLo,vLo );

  var Betta,dH;
   if(H_high>H_low)
  {
    dH = parseFloat(H_high) - parseFloat(H_low);
    dH = dH/100.;
    Betta = Math.sqrt((parseFloat(uUp)-parseFloat(uLo))*(parseFloat(uUp)-parseFloat(uLo))+
            (parseFloat(vUp)-parseFloat(vLo))*(parseFloat(vUp)-parseFloat(vLo)))/dH;
    return Betta;
  }

  return "Метод не работает с заданными значениями параметров";

}

function getGamma(T_high,T_low,H_high,H_low)
{
  var Gamma,dH;

  if(H_high>H_low)
  {
    dH=parseFloat(H_high)-parseFloat(H_low);
    Gamma=((parseFloat(T_high)-parseFloat(T_low))/dH)*(-100.);
    return Gamma;

  }
  else
  {
    return "Метод не работает с заданными значениями параметров";
  }
}

function Pinus400(T_high,T_low,ff_high,ff_low,dd_high,dd_low,H_high,H_low)
{
    if( !isValidNum( H_high )   ||
        !isValidNum( H_low )    ||
        !isValidNum( ff_high )   ||
        !isValidNum( ff_low )    ||
        !isValidNum( dd_high )   ||
        !isValidNum( dd_low )    ||
        !isValidNum( T_high )    ||
        !isValidNum( T_low )   )
    {
      return "Метод не работает с заданными значениями параметров";
    }

    if( H_high === 0.    &&
        H_low === 0.    &&
        ff_high === 0.    &&
        ff_low === 0.    &&
        dd_high === 0.    &&
        dd_low === 0.    &&
        T_high === 0.    &&
        T_low === 0. )
    {
      return "Метод не работает с заданными значениями параметров";
    }
  var Betta,Gamma;
  var pok_bolt;

  Gamma = getGamma(T_high,T_low,H_high,H_low);
  Betta = getBetta( H_high,H_low,ff_high,ff_low,dd_high,dd_low);

  var ret;
  if(H_high>H_low)
  {
     if(pic136(Betta,Gamma)==="err1")
     {
        return "Градиент ветра gV<0 или gV>3";

     }
     if(pic136(Betta,Gamma)==="err2")
     {
        return "Градиент температуры gT<0 или gT>1";

     }
     if(pic136(Betta,Gamma)===1)
     {
       ret ="Болтанка";
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 10, ret, 400,100);
       return ret;

     }
     if(pic136(Betta,Gamma)===2)
     {
       ret = "Без болтанки";
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 8, ret, 400,100);
       return ret;

     }
     else
     {
       return "Не удалось произвести расчет (исходные данные вне области допустимых значений метода)";

     }
  }
  else
  {
    return "ОШИБКА: давление на верхнем уровне должно быть меньше чем на нижнем";

  }

}

//Функция для определения возможности появления болтанки по значениям вертикального градиента скорости ветра gV и вертикального градиента температуры gT. рис.136
function pic136( gV,  gT)
{
  var a=-2.83;
  var b=2.0;
  var c=0.5923;
  var y;

  if(gV<0. || gV>3.)
  {
    return "err1";
  }
  if(gT<0. || gT>1.)
  {
    return "err2";
  }

  y=parseFloat(a*gV*gV)+parseFloat(b*gV)+parseFloat(c);
  if(gT>=y)
  {
    return 1;//"Болтанка"
  }

  return 2;//"Без болтанки"
//  return badvalue();
}


