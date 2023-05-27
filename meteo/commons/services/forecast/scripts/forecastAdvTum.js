

function traek()
{
  var dist;
  dist = dataObj.getAdvectTrackLength();
  //zondObj.calcDist(obj.getCurrentCoord(),obj.getStationCoord());
  if(dist == -9999.)
  {
   return 0;
  }
  return dist;
}


function sGradientT(T0,T0a,dS) //dS - траектория от точки адвекции до пункта прогноза
{

  if( !isValidNum( T0 )   ||
      !isValidNum( T0a )     ||
      !isValidNum( dS ) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var ret,vs_str;
  var SgradT;

  dS = parseFloat(dS);
  SgradT=(parseFloat(T0a)-parseFloat(T0))/dS*100.;
  return SgradT;
}

//ff_pr - ожидаемая скорость приземного ветра в пункте прогноза
function getTumNal(T0a,Td0a,ff_pr,dS,T0)
{

  if( !isValidNum( T0a )   ||
      !isValidNum( Td0a )     ||
      !isValidNum( dS )   ||
      !isValidNum( T0 )     ||
      !isValidNum( ff_pr )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var Da;
  var ret,vs_str;
  var ret_val;
  var SgradT;
  SgradT = sGradientT(T0,T0a,dS);
  if(!isValidNum(SgradT) || SgradT < 0. )    {
      return "Нет адвекции тепла, туман не ожидается";
    }


  Da=parseFloat(T0a)-parseFloat(Td0a);
  ret_val=pic98(SgradT, ff_pr, Da);

  var result;


  if (ret_val=="err1" || ret_val=="err2" || ret_val=="err3" ) {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0, "Адвективный туман не ожидается", 0,1);
    return "Адвективный туман не ожидается";
  }


  if (ret_val==0) {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0, "Адвективный туман не ожидается", 0,1);
    return "Область слоистой облачности";
  }
  if (ret_val==1) {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 12, "Адвективный туман", 0,1);
    return "Область тумана";
  }


}

//Функция для прогноза адвективного тумана по методу Н.В. Петренко по значению отношения
////(разности средней температуры притекающего воздуха и температуры в точке прогноза к
////длине траектории воздушной частицы) TS и значению дефицита точки росы в начале
/////траектории def рис.98

function pic98 ( TS,  V,  def)
{

//  return TS;
  var c=[0.6,1.2];
  var k=[1.,0.7];
  var v=[1.,5.6];
  var a=[-1.737,-5.098];
  var b=[4.674,13.196];
  var y1,y2,p1,p2;
  if (TS<0.|| def>3.){
    return 0;
  }
  if (TS>2.&& (def<3. || V < 7.)){
    return 1;
  }

  if (V<0.)
  {return "err2";}
  if (def<0. )
  {return "err3";}
  y1=parseFloat(a[0]*TS*TS)+parseFloat(b[0]*TS)+parseFloat(c[0]);
  y2=parseFloat(a[1]*TS*TS)+parseFloat(b[1]*TS)+parseFloat(c[1]);
  p1=parseFloat(k[0]*TS)+parseFloat(v[0]);
  p2=parseFloat(k[1]*TS)+parseFloat(v[1]);
  if (def<=y1 && def<=p1 && V<=y2 && V<=p2){
    return 1;
  }
  else {
    return 0;
  }
}

//9.2 для метода Петренко прогноз видимости в тумане
function getTumW(T0,ff_pr,T0a,Td0,D0,Td0a,dS)
{

 T0=parseFloat(T0);
 ff_pr=parseFloat(ff_pr);
 T0a=parseFloat(T0a);
 Td0=parseFloat(Td0);
 Td0a=parseFloat(Td0a);
 dS=parseFloat(dS);

  var SgradTd,Da,SgradT;
  var ret,vs_str;
  var ret_val;
  if(!isValidNum( Td0 )) { Td0 = parseFloat(T0) - parseFloat(D0);}
  SgradTd = sGradientT(Td0,Td0a,dS);
  SgradT = sGradientT(T0,T0a,dS);

  if(!isValidNum(SgradT)||!isValidNum(SgradTd))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  if( SgradT < 0.){
    return "Нет адвекции тепла"
  }

  Da=T0a-Td0a;
  ret_val=pic98(SgradT, ff_pr, Da);


  if (ret_val=="err2") {return "скорость ветра <0 ";}
  if (ret_val=="err3") {return "Дефицит точки росы <0";}

  if (ret_val==0) {return "Область слоистой облачности";}
  if (ret_val==1)
  {
    ret_val=pic99(SgradTd, Da, parseInt(ff_pr));
    switch (ret_val)
    {
      case 0:  return "Область слоистой облачности";
      case 1:
          dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, 500, 500,  0,1);
          return "Видимость более 500 м";
      case 2:
          dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, 400, 400,  0,1);
          return "Видимость 300-500 м";
      case 3:
          dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, 200, 200,  0,1);
          return "Видимость 100-300 м";
      case 4:   return "Скорость ветра и дефицит вне области допустимых значений метода: V<0 или D>6";
      default: return "Метод не работает с заданными значениями параметров";
    }
  }
  else  {return "Исходные данные вне области допустимых значений метода";}
}

//Функция для прогноза видимости в адвективном тумане по градиенту точки росы вдоль
//9-часовой траектории воздушной частицы TS и дефициту точки росы в начале этой траектории
//def, и ожидаемой скорости ветра в пункте прогноза V рис.99.

function pic99 ( TS,  def,  V)
{
  TS = parseFloat(TS);
  def = parseFloat(def);
  V = parseFloat(V);

  var a=[-2.3,-4.8,-2.08];
  var b=[4.12,5.5,8.749];
  var c=[0.34,2.45,-5.6];
  var k=[0.75,0.25];
  var v=[1.65,4.05];
  var y1=0,y2=0,y3=0,p1=0,p2=0;
   //int V;
  if (TS<0. || def<0.|| V <0.)
  {return 4;}

  if (V >5. )
  {return 0;}

  if (TS >2. && def<3.){
    if (V < 4.){
      return 3;
    }
    return 1
  }

  if (TS<0.8)
  {
    y1=(a[0])*TS*TS+(b[0])*TS+(c[0]);
    y2=(a[1])*TS*TS+(b[1])*TS+(c[1]);
    if (def>y1)
    {return 0;}
    else
    {
      if (V>y2) return 1;
      else  return 2;
    }
  }
  else
  {
    p1=(k[0])*TS+(v[0]);
    y3=(a[2])*TS*TS+(b[2])*TS+(c[2]);
    p2=(k[1])*TS+(v[1]);
    if (def>p1) return 0;
    if (V>p2)  return 1;
    else
    {
      if (V<=y3) return 3;
      else return 2;
    }
  }
  return 0;
}





