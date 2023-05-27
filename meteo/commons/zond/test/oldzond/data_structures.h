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
  int ah;        // 0 ��������� � ��������� ������ �� ����� ��� ��������
  int hshshs;    // 1 ������ ������ �� ����� ��� ��������
  int GtGt;      // 2 ����� ����������� ������ ��� ����� ����
  int hshs;      // 3 ������ ����� ��� ������ ��� ����� ����
  char pok_kach[5];
 };
//////////////////////////////////////////////////////////////////////////
struct TSea_gr_6
 {
  int Mi;        // 0 ���������� ����������� ���� � ������
  int C1;        // 1 ������������� ��� (������ ��������) ����
  int C2;        // 2 ������������� ����� ����
  int Pit;       // 3 ������������� ������������ ���������� ����� � ������
  char pok_kach[5];
 };
//////////////////////////////////////////////////////////////////////////
struct TSea_gr_7
 {
  int A1A1;      // 0 �������������� ��������� � ������������� ����
  int A2A2;      // 1
  char pok_kach[3];
 };
//////////////////////////////////////////////////////////////////////////
struct TSeaDataAll
 {
  int Index;         // ������ �������
  int Height;        // ������ ������� ��� ������� ����
  int Pok_Privyaz;   // 0- ���������, 1 - ���
  char Name[31];     // �������� �������
  char pok_inf[5];   // ���������� ������� 3, 4 � 7 �����
  int Koord[4];      // ���������� �������
  int Quadrat[2];    // ������������� �����
  int kol_gr_3;      // ���������� 3 �����
  int kol_gr_6;      // ���������� 6 �����
  int kol_gr_7;      // ���������� 7 �����
  int dd;            //  0 ����������� �����
  int ff;            //  1 �������� �����
  int V;             //  2 ���������
  double TwTwTw;     //  3 ����������� ����������� ����
  int TT;            //  4 ����������� �������
  double SrTwTwTw;   //  5 �������������� ����������� ����
  double SrTTT;      //  6 �������������� ����������� �������
  double HwHw;       //  7 ������ ����
  int Pw;            //  8 ������ ����
  int Dw;            //  9 ���������� ����������� ����
  int Me;            // 10 ���������� ������������ ���� � ������
  int Le;         // 11 ������ ������
  int ii;            // 12 ������� ������������ ����
  int B;             // 13 ������ ��������� ������
  int b;             // 14 ������ ������ ������
  int HnHn;          // 15 ������ ��������� ����
  int hehe;          // 16 ������ ����� �� ����
  char pok_kach[18]; // ������ ����������� �������� ������ [0-16]
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

  Uroven data[20]; // H[0] - �������� ���� ������� � ����������� �����
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
  char pok_kach[6]; //������ ����������� �������� ����������� ������
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
  int Tip; // 0 -������������, 1 - �������
  long Index; // ��� ������������
  int Height;
  int Pok_Privyaz; // 0 - ���������, 1 - �� ��������� (�� �����������)
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
  int Tip; // 0 -������������, 1 - �������
  long Index; // ��� ������������
  int Height;
  int Pok_Privyaz; // 0 - ���������, 1 - �� ��������� (�� �����������)
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
  char pok_kach[9]; //������ ����������� �������� ����������� ������
 };
//////////////////////////////////////////////////////////////////////////
struct Sluzh_Bathy
 {
  int KolDan;
  int Tip_Temp, Met_Sud, D_Sud, V_Sud, Pribor, Registr, Z_Dno;
  char pok_kach[8]; //������ ����������� �������� ����������� ������
 };
//////////////////////////////////////////////////////////////////////////
struct Uroven_TS_DC
 {
  int KolDan;
  int Z, T, S;
  char pok_kach[4]; //������ ����������� �������� ����������� ������
 };
//////////////////////////////////////////////////////////////////////////
struct Uroven_T
 {
  int KolDan;
  int Z, T;
  char pok_kach[3]; //������ ����������� �������� ����������� ������
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
  long Index; // ����� ������� (��� 0)
  int Pok_Privyaz; // 0 - ���������, 1 - �� ��������� (�� �����������)
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
  long Index; // ����� ������� (��� 0)
  int Pok_Privyaz; // 0 - ���������, 1 - �� ��������� (�� �����������)
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
  int tip;  //  ��� ����� (1 - ���������. �����. 0 - �������. �����
  int FGM;  //  ������ �������� �������� (��� tip = 1)
  int GM;   //  ������� ������� (��� tip = 1)
  int FMM;  //  ������ ������������ �������� (��� tip = 1)
  int GL;   //  ������� ������� (��� tip = 1)
  int UGP;  //  ���������� ����� GRID
  int UGL;  //  ���������� ����� GRID
  double dF;   //  ��� ����� �� ������
  double dL;   //  ��� ����� �� �������
  double F0;   //  �������������� ������ ����� ������� (��� tip = 0)
  double L0;   //  �������������� ������� ����� ������� (��� tip = 0)
 };
struct TGRIDDataAll
 {
  int Year;          // ���
  int Month;         // �����
  int Day;           // ����
  int Hour;          // ���
  int Time;          // ���� �������� � �����
  int Model;         // ����� ��� ������
  int Type_Level;    // ��� ������ (1-��������,2-������,3-������)
  int Value_Level;   // �������� ������
  int Param;         // ����� ���������
  int Center;        // ����� ���������������� ������
  int Type_Net;      // NNN
  struct Net setka;  // ������������ �����
  double *inf;       // ������ � ������� (��� ����������� � ��������
                     // ����� *inf � ������� ����� TEMP)
  char *pok_kach;    // ������ ����������� �������� ������
 };

struct Net_GRIB
 {
  int tip;        //  ��� ����� (1 - ���������. �����. 0 - �������. �����
  int flag_UGLP;  // 0- UGL=const, UGP=const;
                  // 1- UGL=var,   UGP=const;
                     // 2- UGL=const, UGP=var;
  int napr_lat;   // 0 - ������ ������, 1 - ��������
  int napr_lon;   // 0 - ������� ������ 1 - ��������
  int flag_scan;  // 0 - ������������ ����� ��������� 1 - ����� ���������
  int flag_vekt;  // 0 - u,v  �� ������ � �����, 1 - � ������������ � ������
  int FGM;     //  ������ �������� �������� (��� tip = 1)
  int GM;      //  ������� ������� (��� tip = 1)
  int FMM;     //  ������ ������������ �������� (��� tip = 1)
  int GL;      //  ������� ������� (��� tip = 1)
  int *UGL;    //  ���������� ����� GRIB � ������ �������
  int *UGP;    //  ���������� ����� GRIB � ������ ����
  double F0;   //  �������������� ������ ����� ������� (��� tip = 0)
  double L0;   //  �������������� ������� ����� ������� (��� tip = 0)
  double FN;   //  �������������� ������ ��������� ����� (��� tip = 0)
  double LN;   //  �������������� ������� ��������� ����� (��� tip = 0)
 };

struct TGRIBDataAll
 {
  int Year;          // ���
  int Month;         // �����
  int Day;           // ����
  int Hour;          // ���
  int Time;          // ���� �������� � �����
  int Model;         // ����� ��� ������
  int Type_Level;    // ��� ������ (1-��������,2-������,3-������)
  int Value_Level;   // �������� ������
  int Param;         // ����� ���������
  int Center;        // ����� ���������������� ������
  int Type_Net;      // ����� �����
  struct Net_GRIB setka;  // ������������ �����
  double *inf;       // ������ � ������� (��� ����������� � ��������
                     // ����� *inf � ������� ����� TEMP)
  char *pok_kach;    // ������ ����������� �������� ������
 };

struct Level
 {
  int PP;  // 0 ���������� ��������� ������� (��)
  int TT;  // 1 ���������� ����������� �������(��)
  int dd;  // 2 ������������ ���� ����������� �������� ����� (��)
  int ff;  // 3 �������� �������� ����� (��)
  char pok_kach[5]; // ������ ����������� �������� ������
 };

struct TMeteo11DataAll
 {
  int Year;          // ���
  int Month;         // �����
  int Day;           // ����
  int Hour;          // ���
  int Min;           // ������
  long Index;        // ����� ������������
  char Name[31];     // �������� ������������
  int P;             // ��������� ( 0 -��������, 1 - �����)
  int X;             // ���������� �
  int Y;             // ���������� Y
  int H;             // ������ ������� ��� ������� ����, � (����)
  int KolDan;        // ���������� ������
  int dP0;           // ���������� ��������� ��������       (���)
  int dT0;           // ���������� ��������� �����������, ����. (�0�0)
  Level data[19];    // ������ �� 18 �������
  int Ht;            // ������ �������������� ������������, �� (����)
  int Hv;            // ������ ��������� ������������, �� (����)
  int start_Level;   // ����� ������� ����, ��� �������� ���� ������
  int end_Level;     // ����� ���������� ����, ��� �������� ���� ������
  char tlg[512];     // ��������� "�����-11"
 };

struct NanoskaKN15 {
  int poima;    // 0 �������� ����� ������� ���� ��� ����� ����� �
                //   ������� ������ ���� �� �����
  int flooding; // 1 �������� ����� ������� ���� ��� ����� ����� �
                //   ������� ����������
  char pok_kach[3];
                };

struct THydLand_gr_5
 {
  int EE;      // 0 �������������� ������� ������� (����. 1)
  int ieie;    // 1 ������������� �������� ������� � ���� ����� (����. 1)
  int EEEE;    // 2 �������������� ���� �������� ������� ������� (����. 1)
  char pok_kach[4];
 };

struct THydLand_gr_6
 {
  int CCC;      // 0 �������������� ��������� ������� ������� (����. 2)
  int icic;    // 1 ������������� �������, ��������������� ��������� ������� �������
  int CCCC;    // 2 ��� �������������� ��������� ����, �����, �������������
  char pok_kach[4];
 };

struct Section_1
 {
//  int KolDan;
  int HHHH;    // 0 ������� ���� ��� ����� ����� � �� � ���� ����������
  int HiHi;    // 1 ��������� ������ ���� � �� �� 8-������� ����
  int H20H20;  // 2 ������� ���� ��� ����� ����� � �� �� 20-������� ����
  float tt;    // 3 ����������� ���� � ��������� �� ������� �����
  int TT;      // 4 ����������� ������� � ��������� �� ������ �������
  int DDD;     // 5 ������� ���� � ��
  int S;       // 6 ������ �������� ������� �� ����
  float QQQ;   // 7 ���������� ������ ���� ������������ ������, ���������� � ������ 1�����
  float R24R24;// 8 ���������� ������� � ��, �������� �� ����� (�� 08 �. �������� �������)
  int dc;      // 9 ����� ����������������� ��������� ������� �� �����
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
  char pok_inf[2]; // ���������� ���������� ��������
  int Koord[4];
  int Quadrat[2];
  int n;          // ��������� ������� �������� 1 - 7
  int poima_Level;    // ������� ������ ���� �� �����
  int flooding_Level; // ������� ���� ������ ����������
  Section_1 s1;
  NanoskaKN15   s2;
 };

struct Field
 {
  int sss;    // 0 ������� ������ �������� ������� �� �������� � ����
  int Kl;     // 1 ������� �������� ������� ������ ����������� ����� �� �������� � ����
  double dd;  // 2 ������� ��������� ����� �� �������� � ����
  int LL;     // 3 ������� ������� ������� ����� �� �������� � ����
  int RsRsRs; // 4 ����� ����� ���� � ������� ������� �� �������� � ����
  int E1;     // 5 �������������� ��������� ����������� ����� �� �������� � ����
  char pok_kach[7];
 };

struct Forest
 {
  int sss;    // 0 ������� ������ �������� ������� �� �������� � ����
  int Kl;     // 1 ������� �������� ������� ������ ����������� ����� �� �������� � ����
  double dd;  // 2 ������� ��������� ����� �� �������� � ����
  int LL;     // 3 ������� ������� ������� ����� �� �������� � ����
  int RsRsRs; // 4 ����� ����� ���� � ������� ������� �� �������� � ����
  int E1;     // 5 �������������� ��������� ����������� ����� �� �������� � ����
  char pok_kach[7];
 };

struct Terms
 {
  int YY;   // 0 ���� ����������� (�����) �������� �������
  int MM;   // 1 ����� �����������  (�����) �������� �������
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
  char pok_inf[2]; // ���������� ���������� ��������
  int year;
  int month;
  int day;
  Field  s1; // ������ 1-3
  Forest s2; // ������ 4-6
  Terms  s3; // 7 ������
  Terms  s4; // 8 ������
  Terms  s5; // 9 ������
  Terms  s6; // 0 ������
 };

struct TLayerDataAll
 {
  int Year;          // ���
  int Month;         // �����
  int Day;           // ����
  int Hour;          // ���
  int Min;           // ������
  long Index;        // ����� ������������
  char Name[31];     // �������� ������������
  int P;             // ��������� ( 0 -��������, 1 - �����)
  int X;             // ���������� �
  int Y;             // ���������� Y
  int H;             // ������ ������� ��� ������� ����, � (����)
  int KolDan;        // ���������� ������
  Level data[7];    // ������ �� 18 �������
  int start_Level;   // ����� ������� ����, ��� �������� ���� ������
  int end_Level;     // ����� ���������� ����, ��� �������� ���� ������
  char tlg[512];     // ��������� "�����-11"
 };

 
 #endif

