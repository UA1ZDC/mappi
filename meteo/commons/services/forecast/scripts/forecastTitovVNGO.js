

//максимальная температура по Титову
function TitovTMax( T700, Td700,D700, T850, Td850,D850, T900 )
{

 if(!isValidNum(T700) ||
    (!isValidNum(Td700 ) && !isValidNum(D700))  ||
    !isValidNum(T850) ||
    (!isValidNum(Td850 ) && !isValidNum(D850))  ||
    !isValidNum(T900) ){
   return "Метод не работает с заданными входными значениями";
  }

 var Ds=0, Tmax=0;
 var T1=0,T2=0;
 T1 = 8.;
 T2 = 12.; // Для неадаптированных методов
 if(!isValidNum( D850 )) { D850 = parseFloat(T850) - parseFloat(Td850);}
 if(!isValidNum( D700 )) { D700 = parseFloat(T700) - parseFloat(Td700);}

 Ds = (D850) + (D700);

 if(Ds < 3.5) {
   Tmax =  parseFloat(T900) + (T1);
 } else {
   Tmax = parseFloat(T900) + (T2);
 }
 return Tmax;
}

// Функция расчета упругости насыщения
function getUprugAll(T)
{
  if (isNaN(T)) {return "Неверно задан параметр температуры";}
 T = parseFloat(T);

 var tmp = 0, E0=0;
 E0      = 6.1078;
 tmp     = (241.9 + T);
 if(tmp  == 0.) return "Метод не работает с заданными входными значениями";
 var res = (E0*Math.pow(10,( (7.63*T )/(241.9 + T) )));
 return res;
}

// Функция расчета удельной влажности
function getVlagAll(P, E)
{
  if (isNaN(P) || isNaN(E)) { return "Неверно заданы параметр давления"; }

 var tmp;
 P   = parseFloat(P);
 P   = parseFloat(E);
 tmp = (P - 0.378*E);

 if(tmp == 0.) return "Метод не работает с заданными входными значениями";
 var res = (0.622*E/tmp);
 return res;
}

// Функция расчета  псевдопотенциальной температуры Tau воздуха
// Tau, K
// T, C
function getTauAll( P, T )
{
    if (isNaN(P) || isNaN(T)) { return "Неверно заданы параметр давления и температуры"; }
   T = parseFloat(T);
   P = parseFloat(P);
 if(P == 0.) return "Метод не работает с заданными входными значениями";

  var res = (((T) + 273.16)*Math.pow((1000./P),0.286));
 return res;
}

// Функция расчета температуры воздуха Tk, соответствующей псевдопотенциальной
// температуре воздуха в точке пересечения изограммы с кривой стратификации
// Tau, K
// Tk, C
function getTkAll( P, Tau )
{
 if (isNaN(P) || isNaN(Tau)) { return "Неверно заданы параметр давления "; }
 //double *Tk;
 P = parseFloat(P);
 Tau = parseFloat(Tau);
 if(P == 0.) return "Метод не работает с заданными входными значениями   ";
 var res = ((Tau/(Math.pow((1000./P),0.286))) - 273.16);
 return res;
}

//для метода прогнозирования ВНГО конвективной ТИТОВА
function getLowBorderTitov(Tmax_in)
{
  if(!isValidNum(Tmax_in)){
    return "Метод не работает с заданными входными значениями";
  }


  konv = getNTitov(Tmax_in);
  if ( !isValidNum(konv) || konv == badvalue() ||  konv==0 ){
    return "Конвективной облачности нет";
  }


  Tmax_in = parseFloat(Tmax_in);

  var h_progn;
  var N_progn;

  if(Tmax_in<-80.||Tmax_in>80.)    return "Метод не работает с заданными входными значениями температуры"; //1;

  // 1. Запись максимальной температуры
  var Tmax = Tmax_in;
  // 3. Получение параметров начального уровня
  var Pn,Tn,Tdn;
  var Em, Sm, En, Sn,h_inv;
  var fl_inver;

  Pn   = zondObj.getPrizInvData(obj.getStationData(),0);
  Tn   = zondObj.getPrizInvData(obj.getStationData(),2);
  Tdn  = zondObj.getPrizInvData(obj.getStationData(),3);


  if (Pn == badvalue())
  {
    Pn  = zondObj.getDataFromHeight(obj.getStationData(),0,0,0);       // 1 Давление на начальном уровне
    Tn  = zondObj.getDataFromHeight(obj.getStationData(),0,0,2);       // 2 Температура на начальном уровне
    Tdn = zondObj.getDataFromHeight(obj.getStationData(),0,0,3);
  }

  // if(!getUprugAll(Tdn, &Em))   return 2;
    Em = getUprugAll(Tdn);
    if(!isValidNum(Em)) return "Метод не работает с заданными входными значениями."; //2;

  // if(!getVlagAll(Pn, Em, &Sm)) return 2;
    Sm = getVlagAll(Pn, Em);
    if(!isValidNum(Sm)) return "Метод не работает с заданными входными значениями..";

  // if(!getUprugAll(Tn, &En))    return 2;
    En = getUprugAll(Tn);
    if(!isValidNum(En)) return "Метод не работает с заданными входными значениями...";

  // if(!getVlagAll(Pn, En, &Sn)) return 2;
    Sn = getVlagAll(Pn, En);
    if(!isValidNum(Sn)) return "Метод не работает с заданными входными значениями....";


  // 4. Определение возможности развития конвективной облачности
  // 4.1 Определение точки пересечения изограммы с кривой стратификации
  var i,KolDan2;
  var P,T,E,S,dS;
  var Tau,Tk,dT;
  P = Pn;

  // максимальное давление на уровне станции
    // больше него не может быть
    var maxP = P;

    for (var i = 0; i < 100; i++) {
      P = P - 10.;
      if(P <= 300){
        // return "Метод не работает с заданными входными значениями"; //3; // Нет точки пересечения
      }
      T = zondObj.getUr(obj.getStationData(),P, 2);
      if(!isValidNum(T))  {
        maxP = zondObj.getDataFromHeight(obj.getStationData(),0,0,0);
        if ( maxP<800 ){
          maxP=P;
        }
        P = maxP -10;
        T = zondObj.getUr(obj.getStationData(),P, 2);
        // return "Метод не работает с заданными входными значениями";  //5; //Нет достоверных данных по температуре
      }

      E = getUprugAll(T);
      if(!isValidNum(E)) {
        return "Метод не работает с заданными входными значениями ";
      }
      S = getVlagAll(P, E);
      if(!isValidNum(S)){
        return "Метод не работает с заданными входными значениями  ";
      }
      dS = S - Sm;
      KolDan2 = (i) + (1);

      //
      // если достигли нужного значения - выходим
      //
      if ( dS>=0.00005 ){
        break;
      }
    };

  // 4.2 Определение P, T, Tk в точке пересечения изограммы с кривой стратификации
    P = (P) + (5);
    T = zondObj.getUr(obj.getStationData(),P, 2);

    if(!isValidNum(T)) {
      P = maxP-5;
      T = zondObj.getUr(obj.getStationData(),P, 2);
      // return "Метод не работает с заданными входными значениями";  //5; //Нет достоверных данных по температуре
    }

    Tau = getTauAll( P, T );
    if(!isValidNum(Tau))    return "Метод не работает с заданными входными значениями .";


    // return Pn;
    Tk  = getTkAll(Pn,Tau);
    if(!isValidNum(Tk))    return "Метод не работает с заданными входными значениями  .";



  // 4.3 Сравнение Tk и Tmax
    if(Tk >= Tmax)     return "Конвективной облачности нет";        //4; // Конвекции нет



  // 5. Расчет ожидаемого количества облаков
  // 5.1 Определение точки (А) пересечения кривой стратификации с сухой адиабатой
    var Ssr,Sf;
    var E1, S1, E2, S2;
    var N, h;
    var Td;
    P = Pn;

    for (var i = 0; i < 100; i++) {
      P = P - 10;
      if(P <= 300){
        // return "Метод не работает с заданными входными значениями"; //3;
      }
      T = zondObj.getUr(obj.getStationData(),P, 2);
      if(!isValidNum(T)){
        P = maxP-10;
        T = zondObj.getUr(obj.getStationData(),P, 2);
        // return "Метод не работает с заданными входными значениями";  //5; //Нет достоверных данных по температуре
      }

      Tau = getTauAll(P,T);
      if(!isValidNum(Tau)){
        return "Метод не работает с заданными входными значениями Tau";
      }
      dT = ((Tmax) + (273.16)) - Tau;
      KolDan2 = (i) + (1);

      if ( dT >= 0 ){
        break
      }
    };



    P = (P) + (5);


    T = zondObj.getUr(obj.getStationData(),P, 2);
    if(!isValidNum(T))            {
      P = maxP-5;
      T = zondObj.getUr(obj.getStationData(),P, 2);
      return "Метод не работает с заданными входными значениями";  //5; //Нет достоверных данных по температуре
    }


  //  if(!getUprugAll(T, &E1))                   return 2;
    E1 = getUprugAll(T);
    if(!isValidNum(E1))           return "Метод не работает с заданными входными значениями E1";

  //  if(!getVlagAll(P, E1, &S1))                return 2;
    S1 = getVlagAll(P, E1);
    if(!isValidNum(S1))           return "Метод не работает с заданными входными значениями S1";

    Td = zondObj.getUr(obj.getStationData(),P, 3);
    if(!isValidNum(Td))           {
      P = maxP;
      Td = zondObj.getUr(obj.getStationData(),P, 3);
      // return "Метод не работает с заданными входными значениями";  //5; //Нет достоверных данных по температуре
    }
  //  if(ur.pok_kach[3]=='1')                 return 2;
  //   Td=T-(double(ur.D)/10.);

  //  if(!getUprugAll(Td, &E2))                  return 2;
    E2 = getUprugAll(Td);
    if(!isValidNum(E2)){
        return "Метод не работает с заданными входными значениями E2";
    }

  //  if(!getVlagAll(P, E2, &S2))                return 2;
    S2 = getVlagAll(P, E2);
    if(!isValidNum(S2)){
        return "Метод не работает с заданными входными значениями S2";
    }


    Ssr = (S1 + S2)/2;

  // для определения количества облачности
    if(S1 > Sm || S2 >= Sm){
      return  "Конвективной облачности нет";       //6; облаков нет
    }

    N = 10.*(Sm-S1)/(Sm-S2);
    N_progn = N;
    if(N < 0)  N = 0;
    if(N > 10) N = 10;
    Sf = (((Sm) + (1. - (0.1 * N)) * Ssr)/(2. - (0.1 * N)));

    // 6. Расчет высоты нижней границы облаков
    // 6.1 Нахождение точки пересечения кривой стратификации с изограммой Sf
    P = Pn;

    for (var i = 0; i < 100; i++) {
      P = P - 10;
      if(P <= 300){
        return "Метод не работает с заданными входными значениями";  //3;
      }
      T = zondObj.getUr(obj.getStationData(),P, 2);

      if(!isValidNum(T)) {
        P = maxP-10;
        T = zondObj.getUr(obj.getStationData(),P, 2);
        return "Метод не работает с заданными входными значениями";  //5; //Нет достоверных данных по температуре
      }

      //  if(!getUprugAll(T, &E))                    return 2;
      E = getUprugAll(T);
      if(!isValidNum(E)){
        return "Метод не работает с заданными входными значениями E";
      }

      //  if(!getVlagAll(P, E, &S))                  return 2;
      S = getVlagAll(P, E);
      if(!isValidNum(S)){
        return "Метод не работает с заданными входными значениями S";
      }

      dS = S - Sf;
      KolDan2 = (i) + (1);

      if ( dS >= 0 ){
        break;
      }
    };


    P = (P) + (5);
    T = zondObj.getUr(obj.getStationData(),P, 2);
    if(!isValidNum(T)) {
      P = maxP-5;
      T = zondObj.getUr(obj.getStationData(),P, 2);
      return "Метод не работает с заданными входными значениями";  //5; //Нет достоверных данных по температуре
    }


  // 6.2 Расчет ВНГО
  h = Math.round(287.039 * ((( Tn + 273.16 ) + ( T + 273.16 ))/2.) * (Math.log(Pn) - Math.log(P))/9.81);
  if (isNaN(h)) { return badvalue(); }
  h_progn = h;

  if(h_progn < 0)
  {
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, 0,"Конвективной облачности нет", 0,1);
      return "Конвективной облачности нет";
  }   //-1; //нет низкой облачности
  if(h_progn > 9000)
  {
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, 0,"Конвективной облачности нет", 0,1);
      return "Конвективной облачности нет";
  } //-1; //нет низкой облачности

  dataObj.savePrognoz( obj.getStationData(),name(),title(),hours(), 20013, h_progn,h_progn, 0,1 );

  return h_progn;
}




//
//
//для метода прогнозирования количества конвективной облачности ТИТОВА
//
//
function getNTitov(Tmax_in)
{

  if(!isValidNum(Tmax_in))
    return "Метод не работает с заданными входными значениями";

  Tmax_in = parseFloat(Tmax_in);

  var N_progn;

  if(Tmax_in<-80.||Tmax_in>80.)    {
    return "Максимальная температура должна быть от -80 до 80 градусов";
  }

// 1. Запись максимальной температуры
  var Tmax = Tmax_in;
// 3. Получение параметров начального уровня
  var Pn,Tn,Tdn;
  var Em, Sm, En, Sn,h_inv;
  var fl_inver;

  Pn  = zondObj.getPrizInvData(obj.getStationData(),0);
  Tn  = zondObj.getPrizInvData(obj.getStationData(),2);
  Tdn = zondObj.getPrizInvData(obj.getStationData(),3);


  if (Pn == badvalue())
  {
    Pn  = zondObj.getDataFromHeight(obj.getStationData(),0,0,0);       // 1 Давление на начальном уровне
    Tn  = zondObj.getDataFromHeight(obj.getStationData(),0,0,2);       // 2 Температура на начальном уровне
    Tdn = zondObj.getDataFromHeight(obj.getStationData(),0,0,3);
  }


// if(!getUprugAll(Tdn, &Em))   return 2;
  Em = getUprugAll(Tdn);

  if(!isValidNum(Em)) return "Метод не работает с заданными входными значениями"; //2;

// if(!getVlagAll(Pn, Em, &Sm)) return 2;
  Sm = getVlagAll(Pn, Em);
  if(!isValidNum(Sm)) return "Метод не работает с заданными входными значениями";

// if(!getUprugAll(Tn, &En))    return 2;
  En = getUprugAll(Tn);
  if(!isValidNum(En)) return "Метод не работает с заданными входными значениями";

// if(!getVlagAll(Pn, En, &Sn)) return 2;
  Sn = getVlagAll(Pn, En);
  if(!isValidNum(Sn)) return "Метод не работает с заданными входными значениями";

// 4. Определение возможности развития конвективной облачности
// 4.1 Определение точки пересечения изограммы с кривой стратификации
  var i,KolDan2;
  var P,T,E,S,dS;
  var Tau,Tk,dT;
  P = Pn;
  // максимальное давление на уровне станции
  // больше него не может быть
  var maxP = P;
  for (var i = 0; i < 100; i++) {

    P = P - 10.;
    if(P <= 300)       {
      // return "Точка пересечения изограммы с кривой стратификации не найдена"; //3; // Нет точки пересечения
    }
    // забираем данные на уровне
    // необходимо проверить - если введенный уровень ниже чем то, что есть в зонде -
    // берем ближайший валиндый уровень
    T = zondObj.getUr(obj.getStationData(),P, 2);
    // return P;

    if(!isValidNum(T))   {
      maxP = zondObj.getDataFromHeight(obj.getStationData(),0,0,0);
      if ( maxP<800 ){
        maxP=P;
      }
      P = maxP -10;
      T = zondObj.getUr(obj.getStationData(),P, 2);
      // return "Нет достоверных данных зондирования по температуре";  //5; //Нет достоверных данных по температуре
    }

    E = getUprugAll(T);
    if(!isValidNum(E))     return "Метод не работает с заданными входными значениями";
    S = getVlagAll(P, E);
    if(!isValidNum(S))     return "Метод не работает с заданными входными значениями";
     dS = S - Sm;
     KolDan2 = (i) + (1);
     if ( dS>=0.00005 ){
      break;
    }
  };

// 4.2 Определение P, T, Tk в точке пересечения изограммы с кривой стратификации
  P = (P) + (5);
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(!isValidNum(T))     {
    P = maxP;
    T = zondObj.getUr(obj.getStationData(),P, 2);
    // return "Нет достоверных данных зондирования по температуре";  //5; //Нет достоверных данных по температуре
  }

  Tau = getTauAll( P, T );
  if(!isValidNum(Tau))    return "Метод не работает с заданными входными значениями ";

  Tk  = getTkAll(Pn,Tau);
  if(!isValidNum(Tk))    return "Метод не работает с заданными входными значениями  ";


// 4.3 Сравнение Tk и Tmax
  if(Tk >= Tmax)     return "Конвективной облачности нет";        //4; // Конвекции нет

  // return Tk+" >= "+Tmax;

// 5. Расчет ожидаемого количества облаков
// 5.1 Определение точки (А) пересечения кривой стратификации с сухой адиабатой
  var Ssr,Sf;
  var E1, S1, E2, S2;
  var  N, h;
  var Td;
  P = Pn;
  for (var i = 0; i < 100; i++) {

    P = P - 10;
    if(P <= 300){
      // return "Метод не работает с заданными входными значениями"; //3;
    }
    T = zondObj.getUr(obj.getStationData(),P, 2);
    if(!isValidNum(T))  {
      P=maxP-10;
      T = zondObj.getUr(obj.getStationData(),P, 2);
      // return "Нет достоверных данных зондирования по температуре";  //5; //Нет достоверных данных по температуре
    }

    Tau = getTauAll(P,T);
    if(!isValidNum(Tau)){
      return "Метод не работает с заданными входными значениями";
    }
    dT = ((Tmax) + (273.16)) - Tau;
    KolDan2 = (i) + (1);
    i++;

    if ( (dT >= 0) ){
      break;
    }
  };

  P = (P) + (5);
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(!isValidNum(T))   {
    P=maxP-5;
    T = zondObj.getUr(obj.getStationData(),P, 2);
    // return "Нет достоверных данных зондирования по температуре";  //5; //Нет достоверных данных по температуре
  }


  E1 = getUprugAll(T);
  if(!isValidNum(E1))           return "Метод не работает с заданными входными значениями";

//  if(!getVlagAll(P, E1, &S1))                return 2;
  S1 = getVlagAll(P, E1);
  if(!isValidNum(S1))           return "Метод не работает с заданными входными значениями";

  Td = zondObj.getUr(obj.getStationData(),P, 3);
  if(!isValidNum(Td))           {
    P=maxP-5;
    Td = zondObj.getUr(obj.getStationData(),P, 3);
    // return "Нет достоверных данных зондирования по температуре";  //5; //Нет достоверных данных по температуре
  }


  E2 = getUprugAll(Td);
  if(!isValidNum(E2))           return "Метод не работает с заданными входными значениями";

//  if(!getVlagAll(P, E2, &S2))                return 2;
  S2 = getVlagAll(P, E2);
  if(!isValidNum(S2))           return "Метод не работает с заданными входными значениями";


  Ssr = (S1 + S2)/2;


  if(S1 > Sm || S2 >= Sm)   {
    return "Конвективной облачности нет";       //6; облаков нет
  }
   N = 100.*(Sm-S1)/(Sm-S2);


   if (isNaN(N)) { return "Метод не работает с заданными входными значениями"; }
   N_progn = Math.round(N);
   if(N < 0)  N = 0;
   if(N > 100) N = 10;
   if (isNaN(N_progn)) { return "Метод не работает с заданными входными значениями"; }

   if ( N_progn==0 ){
    return "Конвективной облачности нет";
   }
   dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20010, N_progn,N_progn, 0,1);

   return N_progn;
}



