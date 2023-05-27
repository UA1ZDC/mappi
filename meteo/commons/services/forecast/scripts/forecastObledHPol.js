
//Наличие облачности на высоте полета
function NearLevel( Hp ){
  if( !isValidNum( Hp )  )
  {
    return "850";
  }
  if(Hp>=2000 && Hp<4000){
      return "700";
  }
  if(Hp>=4000 ){
      return "500";
  }

  return "850";
}

function NalOblak( Hp ){
  if( !isValidNum( Hp )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  if(0 < zondObj.getNalOblak(obj.getStationData(),0, Hp)){
    return 1;
  }
  return 0;
}


//Температура воздуха на нижней границе обледенения
function tObled(skorost){
    var tObled;
    tObled= pic122( skorost);
    if ( !isValidNum(tObled) )
    {
        return "Скорость вне области допустимых значений метода! Повторите ввод!";
     }

  return tObled;//zondObj.getT_obled(obj.getStationData(),0);
}

//Высота нижней границы обледенения,м
function hObled(skorost){
  var tObled;
  tObled= pic122( skorost);
  if ( !isValidNum(tObled) )
  {
      return "Скорость вне области допустимых значений метода! Повторите ввод!";
   }
   var htObed = zondObj.opredHPoT(obj.getStationData(),tObled,0); //в декаметрах???

   return htObed;
}


//Температура воздуха на высоте полета,°С
function Th(Hp){
  if( !isValidNum( Hp )  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  return zondObj.getDataFromHeight(obj.getStationData(),Hp,0,2);
}

//V_s - скорость самолета в км/ч
function obledTemp(V_s)
{
  var tObled;

  tObled= pic122( V_s);

  if ( !isValidNum(tObled) )
  {
    return "Скорость вне области допустимых значений метода! Повторите ввод!";
  }
  return tObled;
}

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
  if (Vs>=0. && Vs<=2000.)
  {
    y=(a*Vs*Vs)+(b*Vs)+(c);
    return y;
  }
  return "Метод не работает с заданными значениями параметров";
}



//T_s - температура на высоте полета
function obledNal(T_s, V_s, H_p)
{
  if( !isValidNum( T_s ) ||
    !isValidNum( V_s ))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  var TObled;
  var ret;
  var ret_val;

  ret_val = zondObj.getNalOblak(obj.getStationData(),0, H_p);
  if( 1 != ret_val ){
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042,ret_val, "Отсутствует облачность", 0,200);
    return "Отсутствует облачность";
  }

  TObled = obledTemp(V_s);
  if( !isValidNum(TObled) )
  {
    return "Некорректное значение скорости";
  }
  else
  if (TObled >=T_s)
  {
    ret="Ожидается обледенение на высоте "+H_p+" м";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 1, ret, 0,200);
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11032, H_p, ret, 0,200);
    return "Ожидается обледенение";
  }
  else
  {
    ret="Обледенение на высоте полета не ожидается";
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20042, 0, ret, 0,200);
    return "Обледенение не ожидается";
  }
}

function ObledIntens(T_s, V_s,TObled,HObled, H_p,T,Ta,D,Da)
{
  if( !isValidNum( T_s ) ||
      !isValidNum( V_s ) ||
      !isValidNum(T)  ||
      !isValidNum(Ta)  ||
      !isValidNum(D) ||
      !isValidNum(Da))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var obled;
  obled = obledNal(T_s, V_s, H_p );

  if( obled == "Обледенение не ожидается" ||
      obled == "Отсутствует облачность")
  {
    return obled;
  }

  var ret_80 = pic80(Ta-T,Da-D);
  //pic80 == 3 - "Облачность  образуется (не размывается)";
  //pic80 == 2 - "Ситуация не определена"
  //pic80 == 1 - "Облачность не образуется (размывается)"


  var colVo=0,colVo1=0,ret_val=0,type_intens=0;
  var ret=0,vs_str=0;

  type_intens = pic123(TObled,HObled);

    if(type_intens == "err1")
    {
      return "Температура обледенения вне области допустимых значений метода";
    }
    if(type_intens == "err2")
    {
      return "Высота полета вне области допустимых значений метода";
    }
    if(type_intens == 0)
    {
      return "Исходные данные вне области допустимых значений метода";
    }


    if(1 == type_intens && 3 == ret_80 )
    {
      return "Обледенение умеренное или сильное";// printf ("Obledenenie umerennoe ili silnoe!\n");
    }
    if(1 == type_intens && 1 == ret_80 )
    {
      return "Обледенение слабое или умеренное";// printf ("Obledenenie umerennoe ili silnoe!\n");
    }

    if((2 == type_intens && 3 == ret_80) )
    {
      return "Обледенение слабое или умеренное";// printf ("Obledenenie umerennoe ili silnoe!\n");
    }
    if((2 == type_intens && 1 == ret_80) )
    {
      return "Обледенение слабое или отсутствует";// printf ("Obledenenie umerennoe ili silnoe!\n");
    }

    if(1 == type_intens )
    {
      return "Обледенение умеренное или сильное (без учета эволюции облачности)";// printf ("Obledenenie umerennoe ili silnoe!\n");
    }

    if(type_intens==2)
    {
      return "Обледенение слабое или отсутствует (без учета эволюции облачности)";// printf ("Obledenenie umerennoe ili silnoe!\n");
    }

    return "Метод не работает с заданными значениями параметров";
}


//Функция для прогноза обледенения по значениям температуры обледенения Tob и высоты H рис.123
function pic123 ( Tob,  H) //изменено ЛКА 15.03.2007
{
   var k=-275;
   var b=500;
   var x1=0.0, x2=-11.0;
   var y;
//Проверка на попадание в область допустимых значений
   var x_str=0.;
   var type_intens;
   if (Tob>x_str)
   {
     return "err1";// Tob-Vne oblasti ODZ!
   }
//Проверка на попадание в области
   if (Tob<=x1)
   {
      y=(k*Tob)+(b);
      if (H<=y)
         type_intens=1;// Obledenenie umerennoe ili silnoe!
      if (H>y)
         type_intens=2;// Obledenenie slaboe ili otsutstvuet!
   }
   else
   {
      type_intens=0
   }
   return type_intens;
}

