function badvalue()
{
  return -9999;
}
function nodata()
{
  return "Нет данных";
}
function emptyvalue()
{
  return "значение не рассчитано";
}

//максимальная температура по Титову - используется в методе
function titovTMax( T700, Td700, T850, Td850, T900 )
{
 if(T700  == badvalue() ||
    Td700 == badvalue() ||
    T850  == badvalue() ||
    Td850 == badvalue() ||
    T900  == badvalue() )
   return "Метод не работает с заданными значениями параметров";

 var D850, D700;
 var Ds, Tmax;
 var T1,T2;
 T1 = 8.;
 T2 = 12.; // Для неадаптированных методов
 D850 = parseFloat(T850) - parseFloat(Td850);
 D700 = parseFloat(T700) - parseFloat(Td700);

 Ds = parseFloat(D850) + parseFloat(D700);

 if(Ds < 3.5) {
   Tmax =  parseFloat(T900) + parseFloat(T1);
 } else {
   Tmax = parseFloat(T900) + parseFloat(T2);
 }
 return Tmax;
}

//прогноз шквала по Решетову 1
//function Reshetov1K( Z1t, Z2t, Tz1t, Tz2t, Z1td, Z2td, Tz1td, Tz2td, Pz1t, Tzemmax, T500)
function Reshetov1K(Pzem,T0,Td0,T1000,Td1000,H1000,T900,T850,Td850,H850,T700,Td700,H700,T500,Td500,H500,T400,Td400,H400)
{

    //TODO Tzemmax получать из Титова - макс температура воздуха у поверхности земли перед грозовым дождем
    if(Pzem  == badvalue() ||
        T0 == badvalue() ||
        Td0 == badvalue() ||
        T1000 == badvalue() ||
        Td1000 == badvalue() ||
        H1000  == badvalue() ||
        T900 == badvalue() ||
        T850 == badvalue() ||
        Td850 == badvalue() ||
        H850  == badvalue() ||
        T700 == badvalue() ||
        Td700 == badvalue() ||
        H700  == badvalue() ||
        T500 == badvalue() ||
        Td500 == badvalue() ||
        H500  == badvalue() ||
        T400 == badvalue() ||
        Td400 == badvalue() ||
        H400  == badvalue()
        ){
        return "Метод не работает с заданными значениями параметров";
    }
 
    var Zt0, Ztd0, Zser, dZ, T, Pser, P, Tsm, dTsm, dT500, dT500p, A, B, d, Umax, Tzemmax;
    Umax = emptyvalue();


    Tzemmax = parseFloat(titovTMax( T700, Td700, T850, Td850, T900 ));

    // определяем пограничные уровни Z1 Z2
    var arrT = [], arrTd = [], arrLevelsP = [], arrLevelsH = [];

    if (Pzem > 1000)
    {
        arrT = [parseFloat(T0),parseFloat(T1000),parseFloat(T850),parseFloat(T700),parseFloat(T500),parseFloat(T400)];
        arrTd = [parseFloat(Td0),parseFloat(Td1000),parseFloat(Td850),parseFloat(Td700),parseFloat(Td500),parseFloat(Td400)];
        arrLevelsP = [parseFloat(Pzem),1000,850,700,500,400];
        arrLevelsH = [0,parseFloat(H1000)*10,parseFloat(H850)*10,parseFloat(H700)*10,parseFloat(H500)*10,parseFloat(H400)*10];
    }
    else
    {
        arrT = [parseFloat(T1000),parseFloat(T0),parseFloat(T850),parseFloat(T700),parseFloat(T500),parseFloat(T400)];
        arrTd = [parseFloat(Td1000),parseFloat(Td0),parseFloat(Td850),parseFloat(Td700),parseFloat(Td500),parseFloat(Td400)];
        arrLevelsP = [1000,parseFloat(Pzem),850,700,500,400];
        arrLevelsH = [parseFloat(H1000)*10,0,parseFloat(H850)*10,parseFloat(H700)*10,parseFloat(H500)*10,parseFloat(H400)*10];
    }

    //вычисляем Z1t, Z2t
    var Z1tNum = -1, Z2tNum= -1;
    for (var i = 0; i < arrT.length-1; i++) {
        //находим соседние уровни, на которых происходит смена знака
        if ((arrT[i]>=0) && (arrT[i+1]<=0))
        {
            Z1tNum = i;
            Z2tNum = i+1;
            break;
        }
        else if ((arrT[i]<=0) && (arrT[i+1]>=0))
        {
            Z1tNum = i+1;
            Z2tNum = i;
            break;
        }
    }

    if ((Z1tNum == -1) || (Z2tNum == -1)) { return Umax; } //уровни не найдены

    var Z1t, Z2t, Tz1t, Tz2t, Z1td, Z2td, Tz1td, Tz2td, Pz1t;

    Z1t = arrLevelsH[Z1tNum]; // высота в метрах
    Z2t = arrLevelsH[Z2tNum]; // высота в метрах
    Tz1t = arrT[Z1tNum];
    Tz2t = arrT[Z2tNum];

    //вычисляем Z1td, Z2td
    var Z1tdNum = -1, Z2tdNum= -1;
    for (var k = 0; k < arrTd.length-1; k++) {
        //находим соседние уровни, на которых происходит смена знака
        if ((arrTd[k]>=0) && (arrTd[k+1]<=0))
        {
            Z1tdNum = k;
            Z2tdNum = k+1;
            break;
        }
        else if ((arrTd[k]<=0) && (arrTd[k+1]>=0))
        {
            Z1tdNum = i+1;
            Z2tdNum = i;
            break;
        }
    }

    if ((Z1tdNum == -1) || (Z2tdNum == -1)) { return "Невозможно рассчитать максимальную скорость ветра"; } //уровни не найдены
        return Tzemmax;

    Z1td = arrLevelsH[Z1tdNum]; // высота в метрах
    Z2td = arrLevelsH[Z2tdNum]; // высота в метрах
    Tz1td = arrT[Z1tdNum];
    Tz2td = arrT[Z2tdNum];

    Pz1t = arrLevelsP[Z1t]; // 1 мбар = 1 ГПа

    /*
     Z1t - высота, соотв станд уровню, на которой темпер воздуха T >=0
     Z2t - высота, соотв станд уровню, на которой темпер воздуха T <=0
     Z1td - высота, соотв станд уровню, на которой темпер точки росы Td >=0
     Z2td - высота, соотв станд уровню, на которой темпер точки росы Td <=0
     Tz1t - температура воздуха на высоте Z1t
     Tz2t - температура воздуха на высоте Z2t
     Tz1td - температура точки росы на высоте Z1td
     Tz2td - температура точки росы на высоте Z2td
     Pz1t - атм давл на высоте Z1t, соотвующей станд уровню, на кот T>=0
     Pzem - атм давл у поверх земли
    */

    //1. Вычисл темпер смоч термометра
    //Вычисляем высоты и находим середину слоя
    Zt0 = Z1t + (Z2t - Z1t)*(Tz1t/Tz2t); //
    Ztd0 = Z1td + (Z2td - Z1td)*(Tz1td/(Tz1td-Tz2td));
    Zser = 2*Zt0/Ztd0;
    //Давление атм на уровне Zser
    dZ = Zser - Z1t;
    T = Tz1t/2;
    Pser = Pz1t*(Math.exp((-0.0001252*dZ)/(1+0.00366*T)));
    // нулевая температура смоченного термометра
    P = Math.pow((Pzem/Pser),0.3)
    Tsm = -318.4 + 600.37*P - 219*Math.pow(P,2); // нулевая температура смоченного термометра


    //2. Вычисл знач параметров
    dTsm = parseFloat(Tzemmax) - Tsm;
    dT500 = parseFloat(Tzemmax)- parseFloat(T500);

    //3. Пороговое значение параметра dT500

    dT500p = 57.4-4.106*dTsm+0.1889*Math.pow(dTsm,2)-0.003054*Math.pow(dTsm,3);

    //4. Разность между пороговым и вычисленным значениями dT500 с учетом возможной поправки A и определяем ее знак для определения возможности возникновения шквала
    A = 0;
    d = dT500 - dT500p + A;
    //5. Прогноз по шквалу
    if (d<=0)
    {
        return Umax;
    }
    //иначе вычисляем макс скорость ветра при шквале
    B = 0; // местная корректировка
    Umax = 12.33+0.9381*dTsm+0.01744*dTsm+0.001487*Math.pow(dTsm,3)+B;
    return Umax;
}


function Reshetov1Descr( U )
{
 var R1 = "Шквал не ожидается", R2 ="Ожидается шквал";
 if (U == badvalue())
    return badvalue();
 if (U == emptyvalue())
 {
    return R1;
 }
 return R2;
}
