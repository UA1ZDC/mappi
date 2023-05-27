// 10.1 обложные осадки Орловой
function orlovaOblOsNal( D_850a, tau850_pr )//адвекция +12 ч
{
  if( !isValidNum( D_850a ) ||
      !isValidNum( tau850_pr ) )
  {
    return badvalue() ;
  }
  var ret_val=-1;
  print (D_850a, tau850_pr)    ;

  ret_val=pic75(D_850a,tau850_pr);
  if (ret_val =="err1")
  {
    return "Дефицит отрицательный";
  }

//  return ret_val;
  switch(ret_val)
  {
    case 1:
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13022, 0,"Без осадков", 200,0);
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20010, 0,"Малооблачно", 200,0);
      return "Без осадков. Малооблачно";

    case 2:
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13022, 0,"Без осадков", 200,0);
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20010, 60,"Облачность 4-6 баллов", 200,0);

      return "Без осадков. Облачность 4-6 баллов";
    case 3:
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13022, 0,"Без осадков", 200,0);
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20010, 100,"Облачность 7-10 баллов", 200,0);
      return "Без осадков. Облачность 7-10 баллов";
    case 4:
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13022, 0,0, 0,1);
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20010, 100,"Облачность 7-10 баллов", 200,0);
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20011, 100,"Облачность 7-10 баллов", 200,0);//FIXME тип облачности
      return "Обложные осадки ожидаются";
    default:
      return"Ошибка";
  }
}

function SkorVertDvig850(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
//    return "yey";
    return badvalue();
  }
  var W850, Lapla_0sredni;
  Lapla0  = parseFloat(Lapla0);
  LaplaAdv  = parseFloat(LaplaAdv);

  Lapla_0sredni = lapSredni(Lapla0,LaplaAdv);
  W850=-3.5*(parseFloat(Lapla_0sredni)/**2.5e11*/+parseFloat(Lapla0));
  return W850;
}

function SkorVertDvig700(Lapla700,Lapla850,Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla700)||
      !isValidNum(Lapla850)||
      !isValidNum(Lapla0)  ||
      !isValidNum(LaplaAdv))
  {
    return badvalue();
  }
  var W700, W850;
  Lapla700 = parseFloat(Lapla700);
  Lapla850 = parseFloat(Lapla850);
  Lapla0   = parseFloat(Lapla0);
  LaplaAdv = parseFloat(LaplaAdv);
  W850     = SkorVertDvig850(Lapla0,LaplaAdv);
  W700     = W850-2.1*((Lapla850)+(Lapla700));
  return W700;
}

function SkorVertDvig500(Lapla700,Lapla500,Lapla850,Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla850)||
      !isValidNum(Lapla700)||
      !isValidNum(Lapla0)||
      !isValidNum(LaplaAdv)||
      !isValidNum(Lapla500))
  {
    return badvalue();
  }
  var W500, W700;
  Lapla850 = parseFloat(Lapla850);
  Lapla700 = parseFloat(Lapla700);
  Lapla0   = parseFloat(Lapla0);
  LaplaAdv = parseFloat(LaplaAdv);
  Lapla500 = parseFloat(Lapla500);
  W700     = SkorVertDvig700(Lapla700,Lapla850,Lapla0,LaplaAdv)

  W500= W700-2.8*(parseFloat(Lapla700)+parseFloat(Lapla500));
  return W500;
}


function lapSredni(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return badvalue();
  }
  var sred;
  sred = (parseFloat(Lapla0)+parseFloat(LaplaAdv))/2.;
  return sred;
}
 /*Функция расчёта возможности образования обложных осадков (и градаций облачности)
по данным о прогностической скорости вертикальных движений воздуха на уровне 850 гПа
в пункте прогноза на 12 часов (d, гПа/12 час) и дефиците температуры точки росы в точке
адвекции на 12 часов назад (град).*/
function pic75( d, r)// добавлено ЛКА в данный проект 01.03.2007
{
  var  a=[-0.6385, -1.065,-1.981];
  var  b=[2.858,4.789,4.578];
  var  c=[5.838,0.682,-2.641];
  var y2,y1,y3;
  var n;

  y1=parseFloat(a[0]*d*d)+parseFloat(b[0]*d)+parseFloat(c[0]);
  y2=parseFloat(a[1]*d*d)+parseFloat(b[1]*d)+parseFloat(c[1]);
  y3=parseFloat(a[2]*d*d)+parseFloat(b[2]*d)+parseFloat(c[2]);

  if(d<0. ) //vne ODZ
    return "err1";

  if(r>y1 || d>10.|| r>21.)   //malooblachno
    n=1;
  if (r<=y1 && r>y2) //4-6 ballov
    n=2;
  if (r<=y2 && r>y3)
    n=3;
  if (r<=y3 )
    n=4;

  return n;
}

//H_850_1000_pr - прогноз относительного геопотенциала
//T_pr - прогноз приземной температуры
function orlovaOblFaza(H_850pr,H_1000pr,T_pr,D_850a,tau850_pr)
{
  if( !isValidNum( H_850pr )   ||
      !isValidNum( H_1000pr )   ||
      !isValidNum( T_pr )       ||
      !isValidNum( D_850a )   ||
      !isValidNum( tau850_pr )   )
  {
    return badvalue() ;
  }
  var prog = orlovaOblOsNal( D_850a, tau850_pr);
  if( prog == "Ошибка")
  {
    return "Ошибка прогноза наличия осадков. Повторите ввод";
  }
  if( prog != "Обложные осадки ожидаются")
  {
    return "Обложные осадки не ожидаются";
  }

  var colVo,ret_val,faza;
  var vs_str, H_850_1000_pr;
  H_850pr   = parseFloat(H_850pr);
  H_1000pr  = parseFloat(H_1000pr);
  T_pr      = parseFloat(T_pr);
  D_850a    = parseFloat(D_850a);
  tau850_pr = parseFloat(tau850_pr);

  H_850_1000_pr = getOrlovaOblGeopot(H_850pr,H_1000pr);
  //print("H_850_1000_pr",H_850_1000_pr,H_850pr,H_1000pr);
  if( !isValidNum( H_850_1000_pr )  )
  {
    return badvalue() ;
  }

  if((H_850_1000_pr<127.)&&(T_pr<=1.)){return "Снег"; }
  if((H_850_1000_pr>132.)&&(T_pr>-1.)){return "Дождь";}

  ret_val = pic84(H_850_1000_pr, T_pr);

  // return ret_val+" "+(H_850_1000_pr.toString())+" "+(T_pr.toString());

  if (ret_val =="err1")
  {
    return "Исходные данные вне области допустимых значений метода: относительный геопотенциал от 127 до 132 гп. дам,температура воздуха от - 4 до 3";
  }
  if (ret_val ==badvalue())
  {
    return badvalue();
  }
  switch(ret_val)
  {
    case 1:
      return "Снег";
    case 2:
      return "Снег, дождь";
    case 3:
      return "Дождь";
    default:
      return "Ошибка";
  }
}

// Функция для расчёта фазового состояния обложных осадков по данным о прогностической температуре воздуха у поверхности земли в пункте прогноза на 12 часов (Т, часов) и прогностическом относительном геопотенциале между уровнями 850 и 1000 гПа (Н, гп.дам) в пункте прогноза на 12 часов

function pic84( H, T)// добавлено ЛКА в данный проект 01.03.2007
{
  var a=[1.25,-1.0,-0.65,-1.46,0.726];
  var b=[-326.375,261.252,167.25,380.098,-188.087];
  var c=[21300.375,-17064.26,-10758.272,-24737.155,12183.41];
  var y1,y2,y3,y4,y5,y6,k,d;
  var faza;

  //1 krivaj (vnizu)
  y1 = parseFloat(a[0]*H*H)+parseFloat(b[0]*H)+parseFloat(c[0]);
  y2 = parseFloat(a[1]*H*H)+parseFloat(b[1]*H)+parseFloat(c[1]);
  y3 = parseFloat(a[2]*H*H)+parseFloat(b[2]*H)+parseFloat(c[2]);
  k  = -0.111;
  d  = 14.68;
  y4 = parseFloat(k*H)+parseFloat(d);// prjmaj (chastj nignej krivoj)
  //2 krivaj (vverhu)
  y5=parseFloat(a[3]*H*H)+parseFloat(b[3]*H)+parseFloat(c[3]);
  y6=parseFloat(a[4]*H*H)+parseFloat(b[4]*H)+parseFloat(c[4]);

  if( H<127. || H>132. || T<-4.|| T>3. ){
    return "err1"; // входные данные вне области допустимых значений
  }

  if( H>=127. && H<=128.875) //Sneg
  {
    if (T<y4){
//      faza=1;//printf("\nSneg\n\n");
      return 1;
    }
  }

  if (H>128.875 && H<=130.25)// Sneg
  {
    if (T<y3 && T>y2 && T>-2.38){
//      faza=1;// printf("\nSneg\n\n");
      return 1;
    }
  }
  if (H>128.875 && H<=131.) // Sneg
  {
    if (T<=-2.38 && T<y1){
//      faza=1;//printf("\nSneg\n\n");
      return 1;
    }
  }

  if (H>=127. && H<129.) //Sneg,dogdj
  {
    if (T>=y4 && T<=y6){
//      faza=2;//printf("\nSneg,dogdj\n\n");
      return 2;
    }
  }

  if (H>=129. && H<=129.875) //Sneg,dogdj
  {
    if (T<=y6 && T>=y3){
//      faza=2;//printf("\nSneg,dogdj\n\n");
      return 2;
    }
  }

  if (H>129.875 && H<=131.7) //Sneg,dogdj
  {
    if (T<=y5 && T>=y3 && T>-1.0){
//      faza=2;//printf("\nSneg,dogdj\n\n");
      return 2;
    }
  }

  if (H>=129.5 && H<=131.) //Sneg,dogdj
  {
    if (T<=-1. && T<=y2 && T>=y1){
//      faza=2;//printf("\nSneg,dogdj\n\n");
      return 2;
    }
  }

  if (H>131. && H<=132.) // Sneg,dogdj
  {
    if (T<-1. && T>=-4.){
//      faza=2;//printf("\nSneg,dogdj\n\n");
      return 2;
    }
  }

  if (H>128. && H<129.875) //Dogdj
  {
    if (T>y6){
//      faza=3;//printf("\nDogdj\n\n");
      return 3;
    }
  }

  if (H>=129.875 && H<=132.) // Dogdj
  {
    if(T>=-1. && T>=y5){
//       faza=3;//printf("\nDogdj\n\n");
      return 3;
    }
  }
  else
  {
     return badvalue();
  }

//  return 0;
}



//
function getOrlovaOblGeopot(H_850,H_1000)
{
  if( !isValidNum( H_850 )   ||
      !isValidNum( H_1000 )  )
  {
    return badvalue() ;
  }
  var Hotn;
  var H850_1000;

  Hotn      = (parseFloat(H_850)-parseFloat(H_1000));
  // H850_1000 = ftoi_norm(Hotn);

  return Hotn;
}


//
function getOrlovaOblIntens(T_850pr,T_700pr,tau850_pr,tau700_pr,D_850a)
{
  if( !isValidNum( T_850pr ) ||
      !isValidNum( T_700pr ) ||
      !isValidNum( tau850_pr ) ||
      !isValidNum( tau700_pr ) ||
      !isValidNum( D_850a ) )
  {
    return badvalue() ;
  }
  T_850pr   = parseFloat(T_850pr);
  T_700pr   = parseFloat(T_700pr);
  var deltaP850_pr = parseFloat(tau850_pr/12);
  var deltaP700_pr = parseFloat(tau700_pr/12);
  D_850a    = parseFloat(D_850a);

  var prog = orlovaOblOsNal( D_850a, tau850_pr);

  if( prog != "Обложные осадки ожидаются")
  {
    return "Обложные осадки не ожидаются";
  }

  var q_max850 = oprSPoTP(850,T_850pr)*1000.;
  if(q_max850 == badvalue())
  {return "Ошибка в q_max850";}

  var q_max700 = oprSPoTP(700,T_700pr)*1000.;
  if(q_max700 == badvalue())
  {return "Ошибка в q_max700";}
 // print("q_max",q_max850,q_max700,T_700pr,T_850pr,deltaP850_pr,deltaP700_pr);
  var p1= 850;
  var p2 = 850 - deltaP850_pr;
  if(0 > deltaP850_pr ){
    p2 = 850;
    p1 = 850 - deltaP850_pr;
  }

  var tplusdelta850 = oprdelta_t(T_850pr,q_max850,p1,p2);
   p1 = 700;
   p2 = 700 - deltaP700_pr;
  if(0 > deltaP700_pr ){
    p2 = 700;
    p1 = 700 - deltaP700_pr;
  }
  var tplusdelta700 = oprdelta_t(T_700pr,q_max700,p1,p2);
  //print("tplusdelta850",tplusdelta850,tplusdelta700);
  var q_max850delta = oprSPoTP(850,tplusdelta850)*1000.;
  var q_max700delta = oprSPoTP(700,tplusdelta700)*1000.;
 //print("q_max850delta",q_max850delta,q_max700delta);

  var deltaq_max850 = q_max850delta - q_max850;
  var deltaq_max700 = q_max700delta - q_max700;
   //print("deltaq_max700",deltaq_max850,deltaq_max700);
 var intens = Math.abs(1.5*deltaq_max850+ 3.0*deltaq_max700);

 dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13022, intens,intens, 0,1);

  return intens;
}


function oprdelta_t(t,q,p1,p2)
{
 var tk =(t+273.15);
 var a = 2.49*q+0.286*(tk);
 var b = 1 + 13513.9*q/(tk*tk);
 var ln = Math.log((p1)/p2)
 return t+a*ln/b;

}

