var pic10_7_x = [8.1,6.0,4.0,3.0,2.0,1.0,1.2,2.15,10.0,4.0,0.0,-1.1,-2.5,-2.7,-2.2,5.0,2.1,0.0,-2.5,-4.0,-4.9,-5.1,-6.0,-7.8];
var pic10_7_y = [1.9,1.1,2.1,2.5,4.0,6.0,7.0,8.0,-7.7,-4.8,-1.85,0.0,4.0,6.0,8.0,-10.0,-8.0,-6.2,-4.0,-1.8,0.0,2.0,6.15,8.0];
var pic10_7_z = [100,100,100,100,100,100,100,100,200,200,200,200,200,200,200,300,300,300,300,300,300,300,300,300];


//Оценка возможности возникновения низкой облачности (менее 300м)
//T0 - температура в точке прогноза
//T0a - адвективная температура
function nalObl( T0, T0a )
{
  if( !isValidNum(T0) ||
      !isValidNum(T0a) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  T0  = parseFloat(T0);
  T0a = parseFloat(T0a);
  var ret, dT;

  if( T0a-T0 < 0. ){
    return "Без облаков: адвекция холода";
  }
  dT = (T0a) - (T0);

  ret = pic10_6(T0,dT);

  if( !isValidNum(ret) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  if( ret == -1 )
  {
    return "Исходные данные вне области допустимых значений метода: Т в пункте прогноза лежит вне диапазона [-20;4], либо адвективное изменение Т вне [0;14]";
  }
  if( ret == 1 )
  {
    return "Есть облачность";
  }
  if( ret == 2 )
  {
    return "Нет облачности";
  }

}


function pic10_6( T, dTa )
{
  var a=[0.0327, -0.0138];
  var b=[-0.3815, -0.9948];
  var c=[0.7491, -1.148];
  var y1, y2;

  T = parseFloat(T);
  dTa = parseFloat(dTa);

  if (T<-20. || T>4.)
  {
    return -1;
  }
  if (dTa<0. || dTa>14.)
  {
    return -1;
  }

  y1=a[0]*T*T+b[0]*T+parseFloat(c[0]);
  y2=a[1]*T*T+b[1]*T+parseFloat(c[1]);

  if (dTa>=0 && dTa<5.9)
  {
    if (dTa<y1)
    {
      return 2;
    }
    else
    {
      return 1;
    }
  }

  else //if (dT>=5.9 && dT<=14)
  {
    if (dTa<y2)
    {
      return 2;
    }
    else //printf ("\nOblaka!\n\n");
    {
      return 1;
    }

  }

  return -1;
}


//Функция прогноза ВНГО на 6 ч
//Td0 - температура точки росы в пункте прогноза
//Td0a - адвективная температура точки росы
//Tpr - прогностическое значение темпераутуры в пункте прогноза
function VGNOAdv6(Td0, Td0a, Tpr,T0, T0a)
{
  var nalobl = nalObl( T0, T0a );
  if("Есть облачность" != nalobl){
      return nalobl;
  }

  if( !isValidNum( Tpr )   ||
    !isValidNum( Td0 )     ||
    !isValidNum( Td0a ) )
  {
    return badvalue();
  }
  
  var vngo = 0.;
  var dTd;
  dTd = Td0a-Td0;
//  print(dTd);
  vngo = zondObj.pic(pic10_7_x, pic10_7_y, pic10_7_z, Tpr, dTd);
  if (!isValidNum(vngo)) { return badvalue(); }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),"0;12", 20013, vngo,vngo, 0,1);
//return wrap.getFormul_VNGO(Td0, Td0a, Tpr);
  return vngo;
}


//Функция прогноза ВНГО на 12 ч
//Td0 - температура точки росы в пункте прогноза
//Td0a - адвективная температура точки росы
//Tpr - прогностическое значение темпераутуры в пункте прогноза
function VGNOAdv12(Td0, Td0a, Tpr,T0, T0a)
{
    var nalobl = nalObl( T0, T0a );
    if("Есть облачность" != nalobl){
        return nalobl;
    }
    if( !isValidNum( Tpr )   ||
    !isValidNum( Td0 )     ||
    !isValidNum( Td0a ) )
  {
    return badvalue();
  }
  
  var vngo = 0.;
  var dTd;
  dTd = Td0a-Td0;
  vngo = zondObj.pic(pic10_7_x, pic10_7_y, pic10_7_z, Tpr, dTd);
  if (!isValidNum(vngo)) { return badvalue(); }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),"0;18", 20013, vngo,vngo, 0,1);
  //return wrap.getFormul_VNGO(Td0, Td0a, Tpr);
  return vngo;
}







