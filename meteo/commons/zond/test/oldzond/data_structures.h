#ifndef structures_main
#define structures_main

#define MAX_STR1 100
#define LENSTR 2001 
#define KOLIND 10330 
#define KOLNAME 267 

//////////////////////////////////////////////////////////////////////////
struct param1
 {
  char dirout1[MAX_STR1];
  char dirout4[MAX_STR1];
	char namfil[MAX_STR1];
  int mes;
	int	dat;
	int	god;
 };
////////////////////////////////////////////////////////////////////////// 
struct Weather
 {
  int KolDan;
  float P, P0, T, Td, Tend, R;
  int V, TipTend, H, N, w, W1, W2, Nh, Cl, Cm, Ch, dd, ff, Time, Hgor;
  char pok_kach[22];
 };
////////////////////////////////////////////////////////////////////////// 
struct Water
 {
  int KolDan;
  float Tw;
  int Ds, Vs, Pwa, Hwa, Pw, Hw, Dw1, Dw2, Pw1, Hw1, Pw2, Hw2, Is, Es, Rs, Ci, Si, Bi, Di, Zi;
  char pok_kach[22]; 
 };
////////////////////////////////////////////////////////////////////////// 
struct Utochnen
 {
  int KolDan;
  float Tmax, Tmin, Tsr, R24;
  int Tpochv, Ns, C, Hs, Es, S, E, Sp, sp;
  char pok_kach[14]; 
 };
////////////////////////////////////////////////////////////////////////// 
struct TWeatherDataAll
 {
  int Tip; 
  long Index;
  int Height;
  int Pok_Privyaz;
  char Name[31];
  int KolDan;
  char pok_inf[4];
  int Koord[4];
  int Quadrat[2];
  Weather s1;
  Water s2;
  Utochnen s3;
  char Nanos[101];
 };
////////////////////////////////////////////////////////////////////////// 
struct TSea_gr_3
 {
  int ah;        // 0 Тенденция в изменении уровня на морях без приливов
  int hshshs;    // 1 Высота уровня на морях без приливов
  int GtGt;      // 2 Время наступления полной или малой воды
  int hshs;      // 3 Высота уроня при полной или малой воде
  char pok_kach[5];
 };
//////////////////////////////////////////////////////////////////////////
struct TSea_gr_6
 {
  int Mi;        // 0 Количество дрейфующего льда в баллах
  int C1;        // 1 Преобладающий вид (стадия развития) льда
  int C2;        // 2 Преобладающая форма льда
  int Pit;       // 3 Преобладающая сплоченность дрейфующих льдов в баллах
  char pok_kach[5];
 };
//////////////////////////////////////////////////////////////////////////
struct TSea_gr_7
 {
  int A1A1;      // 0 Характеристики состояния и распределения льда
  int A2A2;      // 1
  char pok_kach[3];
 };
//////////////////////////////////////////////////////////////////////////
struct TSeaDataAll
 {
  int Index;         // Индекс станции
  int Height;        // Высота станции над уровнем моря
  int Pok_Privyaz;   // 0- привязана, 1 - нет
  char Name[31];     // Название станции
  char pok_inf[5];   // Показатель наличия 3, 4 и 7 групп
  int Koord[4];      // Координаты станции
  int Quadrat[2];    // Синоптический район
  int kol_gr_3;      // Количество 3 групп
  int kol_gr_6;      // Количество 6 групп
  int kol_gr_7;      // Количество 7 групп
  int dd;            //  0 Направление ветра
  int ff;            //  1 Скорость ветра
  int V;             //  2 Видимость
  double TwTwTw;     //  3 Температура поверхности воды
  int TT;            //  4 Температура воздуха
  double SrTwTwTw;   //  5 Среднесуточная температура воды
  double SrTTT;      //  6 Среднесуточная температура воздуха
  double HwHw;       //  7 Высота волн
  int Pw;            //  8 Период волн
  int Dw;            //  9 Напрвление перемещения волн
  int Me;            // 10 Количесвто неподвижного льда в баллах
  int Le;         // 11 Ширина припая
  int ii;            // 12 Толщина неподвижного льда
  int B;             // 13 Ширина прибойной полосы
  int b;             // 14 Ширина полосы наката
  int HnHn;          // 15 Высота прибойных волн
  int hehe;          // 16 Высота снега на льду
  char pok_kach[18]; // Массив показателей качества данных [0-16]
  TSea_gr_3 *gr_3;   //
  TSea_gr_6 *gr_6;   //
  TSea_gr_7 *gr_7;   //
 };
//////////////////////////////////////////////////////////////////////////
struct Uroven
 {
  int KolDan;
  int P, H, T, D, dd, ff;
  char pok_kach[8]; 
 };
////////////////////////////////////////////////////////////////////////// 
struct Gruppa
 {
  int KolDan;
  Uroven data[100];
 };
////////////////////////////////////////////////////////////////////////// 
struct TAeroDataBasa
 {
  int Tip; 
  long Index; 
  int Height;
  int Pok_Privyaz;
  char Name[31];
  int KolDan;
  char pok_inf[5];
  int Term[5];
  int Koord[4];
  int Quadrat[2];
  Gruppa s[4];
 };
//////////////////////////////////////////////////////////////////////////
struct TAeroDataAll
 {
  int Tip;
  long Index;
  int Height;
  int Pok_Privyaz;
  char Name[31];
  int KolDan;
  char pok_inf[5];
  int Term[5];
  int Koord[4];
  int Quadrat[2];
  Uroven data[200];
 };
//////////////////////////////////////////////////////////////////////////
/*struct TMeteo11Data
 {
  int Tip;
  long Index;
  int Pok_Privyaz;
  int KolDan;
  int Koord[4];
  int Date;
  int Quadrat[2];
  int Height_end;

  Uroven data[20]; // H[0] - ЪОБЮЕОЙС ЧУЕИ ЧЕМЙЮЙО Х РПЧЕТИОПУФЙ ЪЕНМЙ
 };*/
//////////////////////////////////////////////////////////////////////////
struct TAeroDataALL
 {
  int Tip;
  long Index;
  int Pok_Privyaz;
  int KolDan;
  int coord[4];
  int P[100];    // pressure
  int H[100];    // height
  double T[100]; // temperature
  double D[100]; // deficit
  int dd[100];   // wind direction
  int ff[100];   // wind velocity
 };
//////////////////////////////////////////////////////////////////////////
struct Uroven_SHAR
 {
  int KolDan;
  int P, H, dd, ff;
  char pok_kach[6]; //Строка показателей качества компонентов записи
 };
//////////////////////////////////////////////////////////////////////////
struct Gruppa_SHAR
 {
  int KolDan;
  int Tip_Gr;
  Uroven_SHAR data[100];
 };
//////////////////////////////////////////////////////////////////////////
struct TSharDataBasa
 {
  int Tip; // 0 -стационарная, 1 - судовая
  long Index; // Для стационарных
  int Height;
  int Pok_Privyaz; // 0 - привязана, 1 - не привязана (по координатам)
  char Name[31];
  int KolDan;
  int Tip_Sond;
  char pok_inf[5];
  int Koord[4];
  int Quadrat[2];
  Gruppa_SHAR s[4];
 };
//////////////////////////////////////////////////////////////////////////
struct TSharDataAll
 {
  int Tip; // 0 -стационарная, 1 - судовая
  long Index; // Для стационарных
  int Height;
  int Pok_Privyaz; // 0 - привязана, 1 - не привязана (по координатам)
  char Name[31];
  int KolDan;
  int Tip_Sond;
  char pok_inf[5];
  int Koord[4];
  int Quadrat[2];
  Uroven_SHAR data[200];
 };
//////////////////////////////////////////////////////////////////////////
struct Sluzh_Tesac
 {
  int KolDan;
  int Tip_Temp, Tip_Sol, Vremya, Period, Met_Sud, Pribor, Registr, Z_Dno;
  char pok_kach[9]; //Строка показателей качества компонентов записи
 };
//////////////////////////////////////////////////////////////////////////
struct Sluzh_Bathy
 {
  int KolDan;
  int Tip_Temp, Met_Sud, D_Sud, V_Sud, Pribor, Registr, Z_Dno;
  char pok_kach[8]; //Строка показателей качества компонентов записи
 };
//////////////////////////////////////////////////////////////////////////
struct Uroven_TS_DC
 {
  int KolDan;
  int Z, T, S;
  char pok_kach[4]; //Строка показателей качества компонентов записи
 };
//////////////////////////////////////////////////////////////////////////
struct Uroven_T
 {
  int KolDan;
  int Z, T;
  char pok_kach[3]; //Строка показателей качества компонентов записи
 };
//////////////////////////////////////////////////////////////////////////
struct Gruppa_TS_DC
 {
  int KolDan;
  Uroven_TS_DC data[500];
 };
//////////////////////////////////////////////////////////////////////////
struct Gruppa_T
 {
  int KolDan;
  Uroven_T data[500];
 };
//////////////////////////////////////////////////////////////////////////
struct TTesacDataAll
 {
  long Index; // Номер станции (или 0)
  int Pok_Privyaz; // 0 - привязана, 1 - не привязана (по координатам)
  char Name[31];
  int KolDan;
  char pok_inf[4];
  int Koord[4];
  int Date[5];
  int Quadrat[2];
  Uroven_TS_DC TDF;
  Gruppa_TS_DC TS, DC;
  Sluzh_Tesac Sl;
 };
//////////////////////////////////////////////////////////////////////////
struct TBathyDataAll
 {
  long Index; // Номер станции (или 0)
  int Pok_Privyaz; // 0 - привязана, 1 - не привязана (по координатам)
  char Name[31];
  int KolDan;
  char pok_inf[3];
  int Koord[4];
  int Date[5];
  int Quadrat[2];
  Uroven_TS_DC TDF;
  Gruppa_T T;
  Sluzh_Bathy Sl;
 };
//////////////////////////////////////////////////////////////////////////
struct Climat
 {
  int KolDan;
  float P, P0, T, St, Tx, Tn, e, R;
  int Rd, nr, S1, ps, mp, mt, mtx, me, mr, ms, Hgor;
  char pok_kach[20];
 };
//////////////////////////////////////////////////////////////////////////
struct Climat2
 {
  int KolDan;
  float P, P0, T, St, Tx, Tn, e, R1;
  int Yb, Yc, nr, S1, Yp, Yt, Ytx, Ye, Yr, Ys, Hgor;
  char pok_kach[20];
 };
//////////////////////////////////////////////////////////////////////////
struct Climat3
 {
  int KolDan;
  int T25,T30,T35,T40,Tn0,Tx0,R01,R05,R10,R50,R100,R150,s00,s01,s10,s50,f10,f20,f30,V1,V2,V3;
  char pok_kach[23];
 };
//////////////////////////////////////////////////////////////////////////
struct Climat4
 {
  int KolDan;
  float Txd, Tnd, Tax, Tan, Rx, fx;
  int Yx, Yn, Yax, Yan, Yr, Yfx, Dts, Dgr, indy, Gx, Gn;
  char pok_kach[18];
 };
//////////////////////////////////////////////////////////////////////////
struct TClimatDataAll
 {
  int Tip;
  long Month;
  long Year;
  long Index;
  int Height;
  int Pok_Privyaz;
  char Name[31];
  int KolDan;
  char pok_inf[5];
  int Koord[4];
  int Quadrat[2];
  Climat s1;
  Climat2 s2;
  Climat3 s3;
  Climat4 s4;
 };

//////////////////////////////////////////////////////////////////////////
struct GRWUroven
 {
  int KolDan;
  int dd, ff;
  double P, T, Td;
  char pok_kach[6];
 };

struct Net
 {
  int tip;  //  Тип сетки (1 - картограф. сетки. 0 - географ. сетки
  int FGM;  //  Широта главного масштаба (для tip = 1)
  int GM;   //  Главный масштаб (для tip = 1)
  int FMM;  //  Широта минимального масштаба (для tip = 1)
  int GL;   //  Главная долгота (для tip = 1)
  int UGP;  //  Количество точек GRID
  int UGL;  //  Количество линий GRID
  double dF;   //  Шаг сетки по широте
  double dL;   //  Шаг сетки по долготе
  double F0;   //  Географическая широта точки отсчета (для tip = 0)
  double L0;   //  Географическая долгота точки отсчета (для tip = 0)
 };
struct TGRIDDataAll
 {
  int Year;          // Год
  int Month;         // Месяц
  int Day;           // День
  int Hour;          // Час
  int Time;          // Срок прогноза в часах
  int Model;         // Метод или модель
  int Type_Level;    // Тип уровня (1-давления,2-высоты,3-особый)
  int Value_Level;   // Значение уровня
  int Param;         // Номер параметра
  int Center;        // Номер прогностического центра
  int Type_Net;      // NNN
  struct Net setka;  // Спецификация сетки
  double *inf;       // Массив с данными (для направления и скорости
                     // ветра *inf в кодовой форме TEMP)
  char *pok_kach;    // Массив показателей качества данных
 };

struct Net_GRIB
 {
  int tip;        //  Тип сетки (1 - картограф. сетки. 0 - географ. сетки
  int flag_UGLP;  // 0- UGL=const, UGP=const;
                  // 1- UGL=var,   UGP=const;
                     // 2- UGL=const, UGP=var;
  int napr_lat;   // 0 - широта растет, 1 - наоборот
  int napr_lon;   // 0 - долгота растет 1 - наоборот
  int flag_scan;  // 0 - сканирование вдоль параллели 1 - вдоль мередиана
  int flag_vekt;  // 0 - u,v  на восток и север, 1 - в соответствии с сеткой
  int FGM;     //  Широта главного масштаба (для tip = 1)
  int GM;      //  Главный масштаб (для tip = 1)
  int FMM;     //  Широта минимального масштаба (для tip = 1)
  int GL;      //  Главная долгота (для tip = 1)
  int *UGL;    //  Количество точек GRIB в каждой колонке
  int *UGP;    //  Количество линий GRIB в каждом ряду
  double F0;   //  Географическая широта точки отсчета (для tip = 0)
  double L0;   //  Географическая долгота точки отсчета (для tip = 0)
  double FN;   //  Географическая широты последней точки (для tip = 0)
  double LN;   //  Географическая долгота последней точки (для tip = 0)
 };

struct TGRIBDataAll
 {
  int Year;          // Год
  int Month;         // Месяц
  int Day;           // День
  int Hour;          // Час
  int Time;          // Срок прогноза в часах
  int Model;         // Метод или модель
  int Type_Level;    // Тип уровня (1-давления,2-высоты,3-особый)
  int Value_Level;   // Значение уровня
  int Param;         // Номер параметра
  int Center;        // Номер прогностического центра
  int Type_Net;      // Номер сетки
  struct Net_GRIB setka;  // Спецификация сетки
  double *inf;       // Массив с данными (для направления и скорости
                     // ветра *inf в кодовой форме TEMP)
  char *pok_kach;    // Массив показателей качества данных
 };

struct Level
 {
  int PP;  // 0 пФЛМПОЕОЙЕ РМПФОПУФЙ ЧПЪДХИБ (рр)
  int TT;  // 1 пФЛМПОЕОЙЕ ФЕНРЕТБФХТЩ ЧПЪДХИБ(фф)
  int dd;  // 2 дЙТЕЛГЙПООЩК ХЗПМ ОБРТБЧМЕОЙС УТЕДОЕЗП ЧЕФТБ (оо)
  int ff;  // 3 уЛПТПУФШ УТЕДОЕЗП ЧЕФТБ (уу)
  char pok_kach[5]; // чЕЛФПТ РПЛБЪБФЕМЕК ЛБЮЕУФЧБ ДБООЩИ
 };

struct TMeteo11DataAll
 {
  int Year;          // зПД
  int Month;         // нЕУСГ
  int Day;           // дЕОШ
  int Hour;          // юБУ
  int Min;           // нЙОХФЩ
  long Index;        // оПНЕТ НЕФЕПУФБОГЙЙ
  char Name[31];     // оБЪЧБОЙЕ НЕФЕПУФБОГЙЙ
  int P;             // рПМХЫБТЙЕ ( 0 -УЕЧЕТОПЕ, 1 - АЦОПЕ)
  int X;             // лППТДЙОБФБ и
  int Y;             // лППТДЙОБФБ Y
  int H;             // чЩУПФБ УФБОГЙЙ ОБД ХТПЧОЕН НПТС, Н (чччч)
  int KolDan;        // лПМЙЮЕУФЧП ДБООЩИ
  int dP0;           // пФЛМПОЕОЙЕ ОБЪЕНОПЗП ДБЧМЕОЙС       (ввв)
  int dT0;           // пФЛМПОЕОЙЕ РТЙЪЕНОПК ФЕНРЕТБФХТЩ, ЗТБД. (ф0ф0)
  Level data[19];    // дБООЩЕ ОБ 18 ХТПЧОСИ
  int Ht;            // чЩУПФБ ФЕНРЕТБФХТОПЗП ЪПОДЙТПЧОБЙС, ЛН (чФчФ)
  int Hv;            // чЩУПФБ ЧЕФТПЧПЗП ЪПОДЙТПЧБОЙС, ЛН (чЧчЧ)
  int start_Level;   // оПНЕТ РЕТЧПЗП УМПС, ДМС ЛПФПТПЗП ЕУФШ ДБООЩЕ
  int end_Level;     // оПНЕТ РПУМЕДОЕЗП УМПС, ДМС ЛПФПТПЗП ЕУФШ ДБООЩЕ
  char tlg[512];     // вАММЕФЕОШ "нЕФЕП-11"
 };

struct NanoskaKN15 {
  int poima;    // 0 тБЪОПУФШ НЕЦДХ ХТПЧОЕН ЧПДЩ ОБД ОХМЕН РПУФБ Й
                //   ХТПЧОЕН ЧЩИПДБ ЧПДЩ ОБ РПКНХ
  int flooding; // 1 тБЪОПУФШ НЕЦДХ ХТПЧОЕН ЧПДЩ ОБД ОХМЕН РПУФБ Й
                //   ХТПЧОЕН ЪБФПРМЕОЙС
  char pok_kach[3];
                };

struct THydLand_gr_5
 {
  int EE;      // 0 иБТБЛФЕТЙУФЙЛБ МЕДПЧЩИ СЧМЕОЙК (ФБВМ. 1)
  int ieie;    // 1 йОФЕОУЙЧОПУФШ МЕДПЧПЗП СЧМЕОЙС Ч ЧЙДЕ ЮЙУМБ (ФБВМ. 1)
  int EEEE;    // 2 иБТБЛФЕТЙУФЙЛБ ДЧХИ ПУОПЧОЩИ МЕДПЧЩИ СЧМЕОЙК (ФБВМ. 1)
  char pok_kach[4];
 };

struct THydLand_gr_6
 {
  int CCC;      // 0 иБТБЛФЕТЙУФЛЙБ УПУФПСОЙС ЧПДОПЗП ПВЯЕЛФБ (ФБВМ. 2)
  int icic;    // 1 йОФЕОУЙЧОПУФШ СЧМЕОЙС, ИБТБЛФЕЙЪХАЭЕЗП УПУФПСОЙС ЧПДОПЗП ПВЯЕЛФБ
  int CCCC;    // 2 дЧЕ ИБТБЛФЕТЙУФЙЛЙ УПУФПСОЙС ТЕЛЙ, ПЪЕТБ, ЧПДПИТБОЙМЙЭБ
  char pok_kach[4];
 };

struct Section_1
 {
//  int KolDan;
  int HHHH;    // 0 хТПЧЕОШ ЧПДЩ ОБД ОХМЕН РПУФБ Ч УН Ч УТПЛ ОБВМАДЕОЙК
  int HiHi;    // 1 йЪНЕОЕОЙЕ ХТПЧОС ЧПДЩ Ч УН ЪБ 8-ЮБУПЧПК УТПЛ
  int H20H20;  // 2 хТПЧЕОШ ЧПДЩ ОБД ОХМЕН РПУФБ Ч УН ЪБ 20-ЮБУПЧПК УТПЛ
  float tt;    // 3 фЕНРЕТБФХТБ ЧПДЩ У ФПЮОПУФША ДП ДЕУСФЩИ ДПМЕК
  int TT;      // 4 фЕНРЕТБФХТБ ЧПЪДХИБ У ФПЮОПУФША ДП ГЕМПЗП ЗТБДХУБ
  int DDD;     // 5 фПМЫЙОБ МШДБ Ч УН
  int S;       // 6 чЩУПФБ УОЕЦОПЗП РПЛТПЧБ ОБ МШДХ
  float QQQ;   // 7 еЦЕДОЕЧОЩК ТБУИПД ЧПДЩ ПФОПУЙФЕМШОП ХТПЧОС, ХЛБЪБООПЗП Ч ЗТХРРЕ 1ооооо
  float R24R24;// 8 лПМЙЮЕУФЧП ПУБДЛПЧ Ч НН, ЧЩРБЧЫЙИ ЪБ УХФЛЙ (ПФ 08 Ю. НЕУФОПЗП ЧТЕНЕОЙ)
  int dc;      // 9 пВЭБС РТПДПМЦЙФЕМШОПУФШ ЧЩРБДЕОЙС ПУБДЛПЧ ЪБ УХФЛЙ
  char pok_kach[11];
  THydLand_gr_5 gr_5[10];
  THydLand_gr_6 gr_6[10];
 };

struct THydrLandDataAll
 {
  int Tip;
  long Index;
  int Height;
  int Pok_Privyaz;
  char Name[31];
  int KolDan;
  char pok_inf[2]; // рПЛБЪБФЕМШ ЛПМЙЮЕУФЧБ ТБЪДЕМПЧ
  int Koord[4];
  int Quadrat[2];
  int n;          // хЛБЪБФЕМШ ОБМЙЮЙС ТБЪДЕМПЧ 1 - 7
  int poima_Level;    // хТПЧЕОШ ЧЩИПДБ ЧПДЩ ОБ РПКНХ
  int flooding_Level; // хТПЧЕОШ ЧПДЩ ОБЮБМБ ЪБФПРМЕОЙС
  Section_1 s1;
  NanoskaKN15   s2;
 };

struct Field
 {
  int sss;    // 0 уТЕДОСС ЧЩУПФБ УОЕЦОПЗП РПЛТБЧБ ОБ НБТЫТХФЕ Ч РПМЕ
  int Kl;     // 1 уФЕРЕОШ РПЛТЩФЙС МЕДСОПК ЛПТЛПК РПЧЕТИОПУФЙ РПЮЧЩ ОБ НБТЫТХФЕ Ч РПМЕ
  double dd;  // 2 уТЕДОСС РМПФОПУФШ УОЕЗБ ОБ НБТЫТХФЕ Ч РПМЕ
  int LL;     // 3 уТЕДОСС ФПМЭЙОБ МЕДСОПК ЛПТЛЙ ОБ НБТЫТХФЕ Ч РПМЕ
  int RsRsRs; // 4 пВЭЙК ЪБРБУ ЧПДЩ Ч УОЕЦОПН РПЛТПЧЕ ОБ НБТЫТХФЕ Ч РПМЕ
  int E1;     // 5 иБТБЛФЕТЙУФЙЛБ УПУФПСОЙС РПЧЕТИОПУФЙ РПЮЧЩ ОБ НБТЫТХФЕ Ч РПМЕ
  char pok_kach[7];
 };

struct Forest
 {
  int sss;    // 0 уТЕДОСС ЧЩУПФБ УОЕЦОПЗП РПЛТБЧБ ОБ НБТЫТХФЕ Ч МЕУХ
  int Kl;     // 1 уФЕРЕОШ РПЛТЩФЙС МЕДСОПК ЛПТЛПК РПЧЕТИОПУФЙ РПЮЧЩ ОБ НБТЫТХФЕ Ч МЕУХ
  double dd;  // 2 уТЕДОСС РМПФОПУФШ УОЕЗБ ОБ НБТЫТХФЕ Ч МЕУХ
  int LL;     // 3 уТЕДОСС ФПМЭЙОБ МЕДСОПК ЛПТЛЙ ОБ НБТЫТХФЕ Ч МЕУХ
  int RsRsRs; // 4 пВЭЙК ЪБРБУ ЧПДЩ Ч УОЕЦОПН РПЛТПЧЕ ОБ НБТЫТХФЕ Ч МЕУХ
  int E1;     // 5 иБТБЛФЕТЙУФЙЛБ УПУФПСОЙС РПЧЕТИОПУФЙ РПЮЧЩ ОБ НБТЫТХФЕ Ч МЕУХ
  char pok_kach[7];
 };

struct Terms
 {
  int YY;   // 0 дЕОШ ПВТБЪПЧБОЙС (УИПДБ) УОЕЦОПЗП РПЛТПЧБ
  int MM;   // 1 нЕУСГ ПВТБЪПЧБОЙС  (УИПДБ) УОЕЦОПЗП РПЛТПЧБ
  char pok_kach[3];
 };

struct TSnowDataAll
 {
  int Tip;
  long Index;
  int Koord[4];
  int Height;
  int Quadrat[2];
  int Pok_Privyaz;
  char Name[31];
  int KolDan;
  char pok_inf[2]; // рПЛБЪБФЕМШ ЛПМЙЮЕУФЧБ ТБЪДЕМПЧ
  int year;
  int month;
  int day;
  Field  s1; // ЗТХРРЩ 1-3
  Forest s2; // ЗТХРРЩ 4-6
  Terms  s3; // 7 ЗТХРРБ
  Terms  s4; // 8 ЗТХРРБ
  Terms  s5; // 9 ЗТХРРБ
  Terms  s6; // 0 ЗТХРРБ
 };

struct TLayerDataAll
 {
  int Year;          // зПД
  int Month;         // нЕУСГ
  int Day;           // дЕОШ
  int Hour;          // юБУ
  int Min;           // нЙОХФЩ
  long Index;        // оПНЕТ НЕФЕПУФБОГЙЙ
  char Name[31];     // оБЪЧБОЙЕ НЕФЕПУФБОГЙЙ
  int P;             // рПМХЫБТЙЕ ( 0 -УЕЧЕТОПЕ, 1 - АЦОПЕ)
  int X;             // лППТДЙОБФБ и
  int Y;             // лППТДЙОБФБ Y
  int H;             // чЩУПФБ УФБОГЙЙ ОБД ХТПЧОЕН НПТС, Н (чччч)
  int KolDan;        // лПМЙЮЕУФЧП ДБООЩИ
  Level data[7];    // дБООЩЕ ОБ 18 ХТПЧОСИ
  int start_Level;   // оПНЕТ РЕТЧПЗП УМПС, ДМС ЛПФПТПЗП ЕУФШ ДБООЩЕ
  int end_Level;     // оПНЕТ РПУМЕДОЕЗП УМПС, ДМС ЛПФПТПЗП ЕУФШ ДБООЩЕ
  char tlg[512];     // вАММЕФЕОШ "нЕФЕП-11"
 };

 
 #endif

