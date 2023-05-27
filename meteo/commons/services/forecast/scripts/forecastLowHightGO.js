

function vnvgoV(p,t,td,d,h1,h2)
{
    if( !isValidNum( t )) return badvalue();
    if( !isValidNum( td )&&!isValidNum( d )) return badvalue();
    var sl = "В слое "+h1+"-"+h2;
    if( !isValidNum( d )){
        d = t-td;
    }
    var nalObl = getNalOblvnvgo(p, d)
    if(0 == nalObl ){ return sl+" наблюдается облачность";}
    if(1 == nalObl ){ return sl+" облачность отсутствует";}
    return sl+" надежных выводов об облачности нет";
}

function getNalOblvnvgo(P, D)
{

  var nalObl;
  if ((P>1000)||(P<200)) return badvalue();
  if ((P<=1000)&&(P>930))
  {
    if (D<=1.6) return nalObl=0;
    if (D>=4.5) return nalObl=1;
    if ((D>1.6)&&(D<4.5)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=930)&&(P>870))
  {
    if (D<=1.6) return nalObl=0;
    if (D>=4.5) return nalObl=1;
    if ((D>1.6)&&(D<4.5))return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=870)&&(P>830))
  {
    if (D<=1.5) return nalObl=0;
    if (D>=5.2) return nalObl=1;
    if ((D>1.5)&&(D<5.2)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=830)&&(P>780))
  {
    if (D<=1.7) return nalObl=0;
    if (D>=6.0) return nalObl=1;
    if ((D>1.7)&&(D<6.0)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=780)&&(P>730))
  {
    if (D<=1.8) return nalObl=0;
    if (D>=7.0) return nalObl=1;
    if ((D>1.8)&&(D<7.0)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=730)&&(P>680))
  {
    if (D<=2.0) return nalObl=0;
    if (D>=7.2) return nalObl=1;
    if ((D>2.0)&&(D<7.2)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=680)&&(P>630))
  {
    if (D<=1.9) return nalObl=0;
    if (D>=8.0) return nalObl=1;
    if ((D>1.9)&&(D<8.0)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=630)&&(P>580))
  {
    if (D<=2.2) return nalObl=0;
    if (D>=8.0) return nalObl=1;
    if ((D>2.2)&&(D<8.0)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=580)&&(P>530))
  {
    if (D<=2.4) return nalObl=0;
    if (D>=8.2) return nalObl=1;
    if ((D>2.4)&&(D<8.2)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=530)&&(P>400))
  {
    if (D<=2.5) return nalObl=0;
    if (D>=8.2) return nalObl=1;
    if ((D>2.5)&&(D<8.2)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=400)&&(P>250))
  {
    if (D<=3.2) return nalObl=0;
    if (D>=8.5) return nalObl=1;
    if ((D>3.2)&&(D<8.5)) return nalObl=2;
    ok = Boolean(true);
  }
  if ((P<=250)&&(P>=200))
  {
    if (D<=4.0) return nalObl=0;
    if (D>=9.0) return nalObl=1;
    if ((D>4.0)&&(D<9.0)) return nalObl=2;
    ok = Boolean(true);
  }
  if (!ok) return 2;
  //  return badvalue();
}






