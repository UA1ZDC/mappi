//18.2 Влажность в свободной атмосфере

function DeficSvobAtm(p,D,T,Tadv,Dadv,W) /*p-давление рассматриваемого слоя*/
{
  var Dpr;
  var res_s,dP;
  var ret;
  var Tdadv;

  if( !isValidNum( p )   ||
      !isValidNum( D )   ||
      !isValidNum( T )   ||
      !isValidNum( Tadv ) ||
      !isValidNum( Dadv )   ||
      !isValidNum( W ))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  var Td;
  Td = parseFloat(T) - parseFloat(D);
  Tdadv = parseFloat(Tadv) - parseFloat(Dadv);
  dP = parseFloat(p) + parseFloat(W);
  res_s = oprSPoTP( dP,Tdadv);

  if (res_s == badvalue())
  {
     return "Метод не работает с заданными значениями параметров";
  }

  Td = oprTPoSP(dP,res_s);
  if ( Td == badvalue() )
  {
     return "Метод не работает с заданными значениями температуры точки росы";
  }

  if (Td<Tadv)
  {
    Dpr=parseFloat(Tadv)-parseFloat(Td);
    ret=Dpr;

  }
  else {
      return "Невозможно расcчитать дефицит";

}
  if (ret == badvalue())
  {
    return "Метод не работает с заданными значениями параметров";
  }

  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 12108, ret.toFixed(2), ret.toFixed(2), 925,100);

  return ret;
}





function oprTPoSP( p, s )
{
  var t,t1,t2,s1,s2,ss;
  var res;
  if(s<0.) {
    return "Метод не работает с заданными значениями параметров";
  }
  if(p<100.||p>1100.) {
    return "Метод не работает с заданными значениями давления";
  }
  t1=-60.;
  t2=50.;

  s1 = oprSPoTP(p,t1);
  s2 = oprSPoTP(p,t2);

  if (s1 == badvalue()|| s2 == badvalue() )
  {
     return "Метод не работает с заданными значениями параметров";
  }

  ss=s2;
  if((parseFloat(s1)-parseFloat(s))*(parseFloat(s2)-parseFloat(s))>0.) return badvalue();
  t=t1;

  while(Math.abs(parseFloat(s)-parseFloat(ss))>1.e-6)
  {
    t=( parseFloat(t1) + parseFloat(t2) )*0.5;
    ss = oprSPoTP(p,t);
    if (ss == badvalue())
    {
       return "Метод не работает с заданными значениями параметров";
    }
    if((parseFloat(s)-parseFloat(ss))*(parseFloat(s1)-parseFloat(ss))>0.) t2=t; else t1=t;
  }
  res=t;
  return res;
}

function SkorVertDvig925(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W925, Lapla_0sredni;
  Lapla_0sredni = lapSredni(Lapla0,LaplaAdv);

  W925=(-3.5*(parseFloat(Lapla_0sredni/**2.5e11*/)+parseFloat(Lapla0)-parseFloat(LaplaAdv)))*0.85;

  return W925;
}

function SkorVertDvig850(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W850, Lapla_0sredni;

  Lapla_0sredni = lapSredni(Lapla0,LaplaAdv);
  W850=-3.5*(parseFloat(Lapla_0sredni)/**2.5e11*/+parseFloat(Lapla0)-parseFloat(LaplaAdv));

  return W850;
}

function SkorVertDvig700(Lapla700,Lapla700adv,Lapla850,Lapla850adv,W850)
{
  if (!isValidNum(Lapla700)||
      !isValidNum(Lapla850)||
      !isValidNum(Lapla700adv)||
      !isValidNum(Lapla850adv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W700;
  W700= parseFloat(W850)-2.1*(parseFloat(Lapla850)-parseFloat(Lapla850adv)+parseFloat(Lapla700)-parseFloat(Lapla700adv));
  return W700;
}

function SkorVertDvig500(Lapla500,Lapla500adv, Lapla700, Lapla700adv, W700)
{
  if (!isValidNum(Lapla700) ||
      !isValidNum(Lapla500) ||
      !isValidNum(Lapla700adv)||
      !isValidNum(Lapla500adv)  )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W500;
  W500= W700-2.8*(parseFloat(Lapla700)-parseFloat(Lapla700adv)+parseFloat(Lapla500)-parseFloat(Lapla500adv));
  return W500;
}

function SkorVertDvig400(Lapla400,Lapla400adv,Lapla500, Lapla500adv,W500)
{
  if (!isValidNum(Lapla400)||
      !isValidNum(Lapla500) ||
      !isValidNum(Lapla500adv) ||
      !isValidNum(Lapla400adv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W400;
  W400= W500-2.8*(parseFloat(Lapla500)-parseFloat(Lapla500adv)+parseFloat(Lapla400)-parseFloat(Lapla400adv));
  return W400;
}
function SkorVertDvig300(Lapla300,Lapla300adv,Lapla400, Lapla400adv,W400)
{
  if (!isValidNum(Lapla300)||
      !isValidNum(Lapla400) ||
      !isValidNum(Lapla300adv) ||
      !isValidNum(Lapla400adv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W300;
  W300= W400-2.8*(parseFloat(Lapla400)-parseFloat(Lapla400adv)+parseFloat(Lapla300)-parseFloat(Lapla300adv));
  return W300;
}

//среднее арифметическое между значением лапласиана давления в пункте расчета
//в конечный момент 12-часового интервала и значением лапласиана давления в
//начальной точке траектории в начальный момент интервала;
function lapSredni(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var sred;
  sred = (parseFloat(Lapla0)+parseFloat(LaplaAdv))/2.;
  return sred;
}


