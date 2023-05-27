
function badvalue()
{
  return -9999;
}


//прогноз влажности (дефицита точки росы) по методу Магнуса
function getVlaznostMagnus( T, Td )
 {
  if(T  == badvalue() ||
     Td == badvalue() )
     return badvalue();
   
   

    T = parseFloat(T);
    Td = parseFloat(Td);

    var tmp1,tmp2;
    var  a,b;

    if (T<-10.) {
      a=9.5;
      b=265.5;
      
    }
    else {
      a=7.63;
      b=241.9;
    }
    
    tmp1=(a*Td)/(b+Td);
    tmp2=(a*T)/(b+T);
    tmp1=tmp1-tmp2;
    tmp2= Math.pow(10.,tmp1);
    if (tmp2<0.) 
      return tmp2=0.;
    if (tmp2>1.) 
      return tmp2=1.;
    else 
      return tmp2;
 }


 
 
 

// Функция расчета упругости насыщения
function getUprugAll(T)
{
  
  T = parseFloat(T);
 //double *E;  
 var tmp, E0;
 E0 = 6.1078;
 tmp = (241.9 + T);
 if(tmp == 0.) return badvalue();
 var res = (E0*Math.pow(10,(7.63*T/(241.9 +T))));
 return res;
}

// Функция расчета удельной влажности
function getVlagAll(P, E)
{
//double *S;
  P = parseFloat(P); 
  E = parseFloat(E);
 var tmp;
 tmp=(P - 0.378*E);
 
 if(tmp == 0.) return badvalue();
 var res = (0.622*E/tmp);
 return res;
}

// Функция расчета  псевдопотенциальной температуры Tau воздуха
// Tau, K
// T, C
function getTauAll( P, T )
{
 //double *Tau;
   T = parseFloat(T);
   P = parseFloat(P);
 if(P == 0.) return badvalue();

  var res = ((T + 273.16)*Math.pow((1000./P),0.286));
 return res;
}

// Функция расчета температуры воздуха Tk, соответствующей псевдопотенциальной
// температуре воздуха в точке пересечения изограммы с кривой стратификации
// Tau, K
// Tk, C
function getTkAll( P, Tau )
{
 //double *Tk;  
 P = parseFloat(P);
 Tau = parseFloat(Tau); 
 if(P == 0.) return badvalue();
 var res = ((Tau/(Math.pow((1000./P),0.286))) - 273.16);
 return res;
}

//максимальная температура по Титову
function TitovTMax( T700, Td700, T850, Td850, T900 )
{
 if(T700  == badvalue() ||
    Td700 == badvalue() ||
    T850  == badvalue() ||
    Td850 == badvalue() ||
    T900  == badvalue() )
   return badvalue();
 
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

//для метода прогнозирования ВНГО конвективной ТИТОВА
function getLowBorderTitov(P0, T0_in, Td0_in, Tmax_in)
{
  if(P0      == badvalue() ||
     T0_in   == badvalue() ||
     Td0_in  == badvalue() ||
     Tmax_in == badvalue() )
    return badvalue();
 
  
  
//int titov_vngoAll(P0, T0_in, Td0_in,float Tmax_in,int)
P0 = parseFloat(P0);
T0_in = parseFloat(T0_in);
Td0_in = parseFloat(Td0_in);
Tmax_in = parseFloat(Tmax_in);
  
var success; 
var h_progn;
var N_progn;
//TDataAeroStorage *&aero,///????  


var P0_in = P0;
if(T0_in<-80.||T0_in>80.)        return badvalue(); //1;
if ((P0_in<600.)||(P0_in>1500.)) return badvalue(); //1;
if(Td0_in<-80.||Td0_in>80.)      return badvalue(); //1;
if(Tmax_in<-80.||Tmax_in>80.)    return badvalue(); //1;

// 1. Запись макисмальной температуры
var Tmax = Tmax_in;
// 3. Получение параметров начального уровня
var Pn,Tn,Tdn;
var Em, Sm, En, Sn,h_inv;
var fl_inver;


//****ParamTypes
//  Pressure    = 0,
//  Height      = 1,
//  Temperature = 2,
//  DewPoint    = 3,
//  Inversion   = 4


//if(zondObj.getPprizInv(Pn,Tn,Tdn,h_inv)){ 

//определение приземной инверсии


if(!zondObj.resetData()) return badvalue();
success = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,4);

if(badvalue() == success)     return badvalue();

if(1. == success) { 
  fl_inver = true;
  Pn  = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,0);  
  Tn  = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,2);  
  Tdn = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,3);  
}
else {
  fl_inver = false;
  Pn=P0_in;       // 1 Давление на начальном уровне
  Tn=T0_in;       // 2 Температура на начальном уровне
  Tdn=Td0_in;
}

// if(!getUprugAll(Tdn, &Em))   return 2;
  Em = getUprugAll(Tdn);
  if(Em == badvalue()) return badvalue(); //2;
 
// if(!getVlagAll(Pn, Em, &Sm)) return 2;
  Sm = getVlagAll(Pn, Em);
  if(Sm == badvalue()) return badvalue();

// if(!getUprugAll(Tn, &En))    return 2;
  En = getUprugAll(Tn);
  if(En == badvalue()) return badvalue();
  
// if(!getVlagAll(Pn, En, &Sn)) return 2;
  Sn = getVlagAll(Pn, En);
  if(Sn == badvalue()) return badvalue();
  
// 4. Определение возможности развития конвективной облачности
// 4.1 Определение точки пересечения изограммы с кривой стратификации
var i,KolDan2;
var P,T,E,S,dS;
var Tau,Tk,dT;
 P = Pn;
 i = 0;
do{
  P = P - 10.;
  if(P <= 300)       return badvalue(); //3; // Нет точки пересечения
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())     return badvalue();  //5; //Нет достоверных данных по температуре
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;  
//  if(ur.pok_kach[2]=='1') return 5;
//   T=double(ur.T)/10.;
  E = getUprugAll(T);
  if(E == badvalue())     return badvalue();
  S = getVlagAll(P, E);
  if(S == badvalue())     return badvalue();
   dS = S - Sm;
   KolDan2 = parseFloat(i) + parseFloat(1);
   i++;
}while(dS>=0.00005);

// 4.2 Определение P, T, Tk в точке пересечения изограммы с кривой стратификации
  P = parseFloat(P) + parseFloat(5);
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())     return badvalue();  //5; //Нет достоверных данных по температуре
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//  T=double(ur.T)/10.;
//   if(!getTauAll(P,T,&Tau))                   return 2;

 
  Tau = getTauAll( P, T );
  if(Tau == badvalue())    return badvalue();

//   if(!getTkAll(P0_in,Tau,&Tk))               return 2;
  Tk  = getTkAll(P0_in,Tau);
  if(Tk == badvalue())    return badvalue();



// 4.3 Сравнение Tk и Tmax
  if(Tk >= Tmax)     return -1;        //4; // Конвекции нет
  
// 5. Расчет ожидаемого количества облаков
// 5.1 Определение точки (А) пересечения кривой стратификации с сухой адиабатой
var Ssr,Sf;
var E1, S1, E2, S2;
var  N, h;
var Td;
  P = Pn;
  i = 0;
do{
  P = P - 10;
  if(P <= 300)              return badvalue(); //3;
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())            return badvalue();  //5; //Нет достоверных данных по температуре
  
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//   T=double(ur.T)/10.;
//  if(!getTauAll(P,T,&Tau))                   return 2;

  Tau = getTauAll(P,T);
  if(Tau == badvalue())  return badvalue();
   dT = (parseFloat(Tmax) + parseFloat(273.16)) - Tau;
   KolDan2 = parseFloat(i) + parseFloat(1);
   i++;
}while(dT >= 0);

  P = parseFloat(P) + parseFloat(5);
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())            return badvalue();  //5; //Нет достоверных данных по температуре
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//   T=double(ur.T)/10.;

//  if(!getUprugAll(T, &E1))                   return 2;
  E1 = getUprugAll(T);
  if(E1 == badvalue())           return badvalue();

//  if(!getVlagAll(P, E1, &S1))                return 2;
  S1 = getVlagAll(P, E1);
  if(S1 == badvalue())           return badvalue();

  Td = zondObj.getUr(obj.getStationData(),P, 3);
  if(Td == badvalue())           return badvalue();  //5; //Нет достоверных данных по температуре
//  if(ur.pok_kach[3]=='1')                 return 2;
//   Td=T-(double(ur.D)/10.);

//  if(!getUprugAll(Td, &E2))                  return 2;
  E2 = getUprugAll(Td);
  if(E2 == badvalue())           return badvalue();

//  if(!getVlagAll(P, E2, &S2))                return 2;
  S2 = getVlagAll(P, E2);  
  if(S2 == badvalue())           return badvalue();

  
  Ssr = (S1 + S2)/2;
  if(S1 > Sm || S2 >= Sm)   return badvalue();       //6;
   N = 10.*(Sm-S1)/(Sm-S2);
   N_progn = N;
   if(N < 0)  N = 0;
   if(N > 10) N = 10;
   Sf = ((parseFloat(Sm) + (1. - (0.1 * N)) * Ssr)/(2. - (0.1 * N)));
 // 6. Расчет высоты нижней границы облаков
// 6.1 Нахождение точки пересечения кривой стратификации с изограммой Sf
P = Pn;
i = 0;
do{
   P = P - 10;
  if(P <= 300)                return badvalue();  //3;
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())            return badvalue();  //5; //Нет достоверных данных по температуре

//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//   T=ur.T/10;

//  if(!getUprugAll(T, &E))                    return 2;
  E = getUprugAll(T);
  if(E == badvalue())            return badvalue();

//  if(!getVlagAll(P, E, &S))                  return 2;
  S = getVlagAll(P, E);
  if(S == badvalue())            return badvalue();

   dS = S - Sf;
   KolDan2 = parseFloat(i) + parseFloat(1);
   i++;
}while(dS >= 0);

  P = parseFloat(P) + parseFloat(5);
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())            return badvalue();  //5; //Нет достоверных данных по температуре

//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//   T=double(ur.T)/10;
  
  
// 6.2 Расчет ВНГО
  h = Math.round(287.039 * (((parseFloat(T0_in) + parseFloat(273.16)) + (parseFloat(T) + parseFloat(273.16)))/2.) * (Math.log(P0_in) - Math.log(P))/9.81);
  h_progn = h;
  if(h_progn < 0)    h_progn = -1; //нет облачности
  if(h_progn > 9000) h_progn = -1; //нет облачности

  return h_progn;
}

//для метода прогнозирования количества конвективной облачности ТИТОВА
function getNTitov(P0, T0_in, Td0_in, Tmax_in)
{
  if(P0      == badvalue() ||
     T0_in   == badvalue() ||
     Td0_in  == badvalue() ||
     Tmax_in == badvalue() )
    return badvalue();
 
  
  
//int titov_vngoAll(P0, T0_in, Td0_in,float Tmax_in,int)
P0 = parseFloat(P0);
T0_in = parseFloat(T0_in);
Td0_in = parseFloat(Td0_in);
Tmax_in = parseFloat(Tmax_in);
  
var success; 
var h_progn;
var N_progn;
//TDataAeroStorage *&aero,///????  


var P0_in = P0;
if(T0_in<-80.||T0_in>80.)        return badvalue(); //1;
if ((P0_in<600.)||(P0_in>1500.)) return badvalue(); //1;
if(Td0_in<-80.||Td0_in>80.)      return badvalue(); //1;
if(Tmax_in<-80.||Tmax_in>80.)    return badvalue(); //1;

// 1. Запись макисмальной температуры
var Tmax = Tmax_in;
// 3. Получение параметров начального уровня
var Pn,Tn,Tdn;
var Em, Sm, En, Sn,h_inv;
var fl_inver;


//****ParamTypes
//  Pressure    = 0,
//  Height      = 1,
//  Temperature = 2,
//  DewPoint    = 3,
//  Inversion   = 4


//if(zondObj.getPprizInv(Pn,Tn,Tdn,h_inv)){ 

//определение приземной инверсии


if(!zondObj.resetData()) return badvalue();
success = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,4);

if(badvalue() == success)     return badvalue();

if(1. == success) { 
  fl_inver = true;
  Pn  = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,0);  
  Tn  = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,2);  
  Tdn = zondObj.getZondValuePoDescr(obj.getStationData(),17,0,3);  
}
else {
  fl_inver = false;
  Pn=P0_in;       // 1 Давление на начальном уровне
  Tn=T0_in;       // 2 Температура на начальном уровне
  Tdn=Td0_in;
}

// if(!getUprugAll(Tdn, &Em))   return 2;
  Em = getUprugAll(Tdn);
  if(Em == badvalue()) return badvalue(); //2;
 
// if(!getVlagAll(Pn, Em, &Sm)) return 2;
  Sm = getVlagAll(Pn, Em);
  if(Sm == badvalue()) return badvalue();

// if(!getUprugAll(Tn, &En))    return 2;
  En = getUprugAll(Tn);
  if(En == badvalue()) return badvalue();
  
// if(!getVlagAll(Pn, En, &Sn)) return 2;
  Sn = getVlagAll(Pn, En);
  if(Sn == badvalue()) return badvalue();
  
// 4. Определение возможности развития конвективной облачности
// 4.1 Определение точки пересечения изограммы с кривой стратификации
var i,KolDan2;
var P,T,E,S,dS;
var Tau,Tk,dT;
 P = Pn;
 i = 0;
do{
  P = P - 10.;
  if(P <= 300)       return badvalue(); //3; // Нет точки пересечения
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())     return badvalue();  //5; //Нет достоверных данных по температуре
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;  
//  if(ur.pok_kach[2]=='1') return 5;
//   T=double(ur.T)/10.;
  E = getUprugAll(T);
  if(E == badvalue())     return badvalue();
  S = getVlagAll(P, E);
  if(S == badvalue())     return badvalue();
   dS = S - Sm;
   KolDan2 = parseFloat(i) + parseFloat(1);
   i++;
}while(dS>=0.00005);

// 4.2 Определение P, T, Tk в точке пересечения изограммы с кривой стратификации
  P = parseFloat(P) + parseFloat(5);
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())     return badvalue();  //5; //Нет достоверных данных по температуре
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//  T=double(ur.T)/10.;
//   if(!getTauAll(P,T,&Tau))                   return 2;

 
  Tau = getTauAll( P, T );
  if(Tau == badvalue())    return badvalue();

//   if(!getTkAll(P0_in,Tau,&Tk))               return 2;
  Tk  = getTkAll(P0_in,Tau);
  if(Tk == badvalue())    return badvalue();



// 4.3 Сравнение Tk и Tmax
  if(Tk >= Tmax)     return -1;        //4; // Конвекции нет
  
// 5. Расчет ожидаемого количества облаков
// 5.1 Определение точки (А) пересечения кривой стратификации с сухой адиабатой
var Ssr,Sf;
var E1, S1, E2, S2;
var  N, h;
var Td;
  P = Pn;
  i = 0;
do{
  P = P - 10;
  if(P <= 300)              return badvalue(); //3;
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())            return badvalue();  //5; //Нет достоверных данных по температуре
  
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//   T=double(ur.T)/10.;
//  if(!getTauAll(P,T,&Tau))                   return 2;

  Tau = getTauAll(P,T);
  if(Tau == badvalue())  return badvalue();
   dT = (parseFloat(Tmax) + parseFloat(273.16)) - Tau;
   KolDan2 = parseFloat(i) + parseFloat(1);
   i++;
}while(dT >= 0);

  P = parseFloat(P) + parseFloat(5);
  T = zondObj.getUr(obj.getStationData(),P, 2);
  if(T == badvalue())            return badvalue();  //5; //Нет достоверных данных по температуре
//  if(!aero->aero->getUr(ftoi_norm(P*10.),ur)) return 5;
//  if(ur.pok_kach[2]=='1') return 5;
//   T=double(ur.T)/10.;

//  if(!getUprugAll(T, &E1))                   return 2;
  E1 = getUprugAll(T);
  if(E1 == badvalue())           return badvalue();

//  if(!getVlagAll(P, E1, &S1))                return 2;
  S1 = getVlagAll(P, E1);
  if(S1 == badvalue())           return badvalue();

  Td = zondObj.getUr(obj.getStationData(),P, 3);
  if(Td == badvalue())           return badvalue();  //5; //Нет достоверных данных по температуре
//  if(ur.pok_kach[3]=='1')                 return 2;
//   Td=T-(double(ur.D)/10.);

//  if(!getUprugAll(Td, &E2))                  return 2;
  E2 = getUprugAll(Td);
  if(E2 == badvalue())           return badvalue();

//  if(!getVlagAll(P, E2, &S2))                return 2;
  S2 = getVlagAll(P, E2);  
  if(S2 == badvalue())           return badvalue();

  
  Ssr = (S1 + S2)/2;
  if(S1 > Sm || S2 >= Sm)   return badvalue();       //6;
   N = 10.*(Sm-S1)/(Sm-S2);
   N_progn = N;
   if(N < 0)  N = 0;
   if(N > 10) N = 10;
   return N_progn;
}



