
function badvalue()
{
  return -9999;
}


//прогноз гроз по Сосину
function SosinDescr( T500, Td500,D500, T700, Td700,D700, T850, Td850, D850 )
{
    if(!isValidNum(T500) ||
    (!isValidNum(Td500 ) && !isValidNum(D500)) ||
    !isValidNum(T700)    ||
    (!isValidNum(Td700 ) && !isValidNum(D700)) ||
    !isValidNum(T850)    ||
    (!isValidNum(Td850 ) && !isValidNum(D850)) )
    return badvalue();

    var Pv,Pn,Tpv,Tpn;
    if(!zondObj.resetData()) return badvalue();
    Pv = zondObj.calcPNS(obj.getStationData(),0);
    Pn = zondObj.calcPNS(obj.getStationData(),1);
    Tpv = zondObj.calcPNS(obj.getStationData(),2);
    Tpn = zondObj.calcPNS(obj.getStationData(),3);
    var tpot, Ds, dTp500, Tp500, dTph, dD850_500;
    var retBool = false, rte_val, ret = badvalue();
    if(!isValidNum( D850 )) {D850 = parseFloat(T850) - parseFloat(Td850);}
    if(!isValidNum( D700 )) {D700 = parseFloat(T700) - parseFloat(Td700);}
    if(!isValidNum( D500 )) {D500 = parseFloat(T500) - parseFloat(Td500);}

    Tp500 = oprTPot(500.,T500, Td500); //псевдопотенциальная температура
    //if (badvalue() === tpot) { return badvalue(); }
    //Tp500 = oprTet(500.,tpot); //потенциальная температура
    if (badvalue() == Tp500) { return badvalue(); }
//    return Tp500;

    Ds = parseFloat(D850)+parseFloat(D700)+parseFloat(D500);
    dTp500=Tp500-Tpn;
    //return dTp500;

    dTph=(Tpn-Tpv)/(0.01*(Pn-Pv));
//    return dTph;
    dD850_500=D850-D500;
//    return dD850_500;

    var dataB;
    if(Ds<30&&dTp500<=3&&dTph>0)
    {
        if(dD850_500>0)
        {
            dataB = 29;
            ret = "Гроза";
            retBool=true;
            dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, dataB.toFixed(2),ret, 0,1);

        }
        else
        {
                rte_val=pic106(dTph,dD850_500);
                if (rte_val === 0)
                {
                    dataB = 29;
                    ret = "Гроза";
                    retBool=true;
                    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, dataB.toFixed(2),ret, 0,1);

                }
                else if (rte_val === 1)
                {
                    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0,"Без грозы", 0,1);
                    retBool=true;ret = "Без грозы";
                }
         }
    }
    if(false === retBool)
    {
        if(dTp500<=3&&dTph>0)
        {
            retBool=true;
            ret = "Гроза";
            dataB = 29;
            dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, dataB.toFixed(2),ret, 0,1);

        }
        else {retBool=true;ret = "Без грозы"; dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0,ret, 0,1);}
    }
    if(false === retBool){retBool=true;ret = "Без грозы";dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, 0,ret, 0,1);}

    return ret;

}

function pic106(QVal, DVal)
{
var Q = parseFloat(QVal);
var D = parseFloat(DVal);
var nal=0;
var a=-0.0816,b=0.133,c=-1.8486;
var y;
if (Q<0.||Q>12.) return -2;
if (D<-12.||D>4.) return -2;
 y=parseFloat(a*Q*Q)+parseFloat(b*Q)+parseFloat(c);
 if(D<=y)nal=0;
  else nal=1;
return nal;
}

function oprTPot(pVal,tVal, TdVal)
{
  var s,cp,l,val;
  var p = parseFloat(pVal);
  var t = parseFloat(tVal);
  var Td = parseFloat(TdVal);
  if(t<-60.||t>500.) return badvalue();
  if(p<100.||p>1100.) return badvalue();

  s=0.622*EP(Td)/p; //массовая доля водяного пара
//  return s;
  cp = 1004.7*(1.0-s)+1846.0*s; // удельная теплота парообразования
//  return cp;
  l=(3.14821-0.00237*(parseFloat(t)+273.15))*1.e6;
  //return l;
  val=parseFloat(t)+273.15+s*l/cp;
//  return val;
  val*=Math.pow((1000./p),0.286);
  val-=273.15;
  return val;
}

function oprTet(pVal, tVal)
{
  var p = parseFloat(pVal);
  var t = parseFloat(tVal);
  var val;
  if(t<-60.||t>500.) return badvalue();
  if(p<100.||p>1100.) return badvalue();

  val=parseFloat(t)+273.15;
  val*=Math.pow((1000./p),0.286);
  val-=273.15;
  return val;
}



//мп прогнозирования грозы Сосина
//8.5-25
//bool Sosin(QString & inStr,QString & ret)
//{
//int rte_val;
//bool retBool=FALSE;
//float Ds,dD850_500,dTph,dTp500,Pn,Pv,Tpn,Tpv,D850,D700,T500,D500;
//double tpot,Tp500;
//if(kolChiselVString(inStr)!=8) return false;
//sscanf(inStr.ascii(),"%f %f %f %f %f %f %f %f",&Pn,&Pv,&Tpn,&Tpv,&D850,&D700,&T500,&D500);

//if(!oprTPot(500.,T500,tpot)) return false;
//if(!oprTet(500.,tpot,Tp500)) return false;
//Ds=D850+D700+D500;
//dTp500=Tp500-Tpn;
//dTph=(Tpn-Tpv)/(0.01*(Pn-Pv));
//dD850_500=D850-D500;

// if(Ds<30&&dTp500<3&&dTph>0)
//   {
//   if(dD850_500>0)
//       {ret.sprintf("гроза");retBool=true;}
//     else
//      {
//      rte_val=pic106(dTph,dD850_500);
//      switch (rte_val)
//          {
//          case 0:
//          retBool=true;ret.sprintf("гроза");
//          break;
//	  case 1:
//          retBool=true;ret.sprintf("без грозы");
//	  break;
//          }
//      }
//  }
// if(!retBool)
//   {
//   if(dTp500<3&&dTph>0)
//     {retBool=true;ret.sprintf("гроза");}
//   else
//     {retBool=true;ret.sprintf("без грозы");}
//   }
// if(!retBool){retBool=true;ret.sprintf("без грозы");}
//return retBool;
//}
