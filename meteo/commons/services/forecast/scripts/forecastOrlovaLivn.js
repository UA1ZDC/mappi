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
  return " - ";
}
var intens      = 0.0;
var hour_global = 0.0;
//Исходными данными для прогнозирования являются:

//– давление на верхнем(Pv) и на нижнем(Pn) уровнях конвективно-неустойчивого слоя (КНС) (Р1,Р2, гПа);

//– прогностическое значение температуры воздуха на уровне 850 гПа в точке прогноза через 12 ч после исходного срока;

//– температура воздуха на верхнем(Tpv) и на нижнем(Tpn) уровне КНС (Т1, Т2);

//– температура на кривой состояния на верхнем  и на нижнем  уровне КНС;

//– средняя длина траектории в слое 1000-850 гПа, , км, за интервал времени 12 час (период продолжительности конвекции при внутримассовых ливнях);

//– прогностическое значение максимальной температуры воздуха у земли в точке прогноза, Tmax, oC

//– средняя скорость ветра вдоль 12-часовой траектории на уровнях 700 и 500 гПа.

function Pv()
{
   var res = zondObj.calcPNS(obj.getStationData(),0);
    // var res = zondObj.calcPNS(obj.getStationData(),0);
    if (!isValidNum(res)) { return badvalue(); }
    return res;
}
function Pn()
{
  var res = zondObj.calcPNS(obj.getStationData(),1);
  // res = zondObj.calcPNS(obj.getStationData(),1);
    if (!isValidNum(res)) { return badvalue(); }
    return res;
}
function Tpv()
{
  var res =  zondObj.calcPNS(obj.getStationData(),2);
  // zondObj.calcPNS(obj.getStationData(),4);
    if (!isValidNum(res)) { return badvalue(); }
    return res;
}
function Tpn()
{
  var res =  zondObj.calcPNS(obj.getStationData(),3);
  // zondObj.calcPNS(obj.getStationData(),5);
    if (!isValidNum(res)) { return badvalue(); }
    return res;
}

function T850Adv()
{
  zondObj.advData(obj.getStationData());
  var res = zondObj.advParam(obj.getStationData(),850, 2);
    if (!isValidNum(res)) { return badvalue(); }
    return res;
}

function Tsostn(P)
{
    return tsost(P);
}

function Tsostv(P)
{
    return tsost(P);
}

function tsost(P)
{
  var res = zondObj.oprTsost(obj.getStationData(),P);
    if (!isValidNum(res)) { return badvalue(); }
    return res;
}

function TmaxAdv()
{
  var T800 = zondObj.advParam(obj.getStationData(),800, 2);
  var P0 = zondObj.getUrzDataAdv(obj.getStationData(),0);
    if (!isValidNum(T800)||!isValidNum(P0)) { return badvalue(); }
//    var T850 = zondObj.advParam(850, 2);
//    var T700 = zondObj.advParam(850, 2);
//    var T800 = parseFloat(T850) - 50*((parseFloat(T850)-parseFloat(T700))/150);// интерполируем температуру на кривой стратификации между поверхностями 850 и 700 гПа
    var Tmax = (parseFloat(T800)+273.15)*Math.pow((parseFloat(P0)/800),0.286)-273.15; // расчет максимальной температуры у земли по сухоадиабатическому соотношению
    return Tmax;
}

function ff500()
{
  var ff = zondObj.getUr(obj.getStationData(),500,6);
    if (!isValidNum(ff)) { return badvalue(); }
    return ff;
}

function ff700()
{
    zondObj.resetData();
    var ff = zondObj.getUr(obj.getStationData(),700,6);
    if (!isValidNum(ff)) { return badvalue(); }
    return ff;
}

function traekt1000850()
{
  var dist;
  dist = dataObj.getAdvectTrackLength();
//  dist = zondObj.calcDist(obj.getCurrentCoord(),obj.getStationCoord());

  if(dist == -9999.)
  {
   return badvalue();
  }
  return dist;
}


//1. Определяем величину m-массу воздуха в КНС, г, численно равную толщине КНС, выраженной в гПа:

function calcM(Pv,Pn)
{
  if (!isValidNum(Pv) ||
      !isValidNum(Pn))
  {
      return badvalue();
  }
  return parseFloat(Pn)-parseFloat(Pv);
}

//2. Определяем среднюю температуру  на кривой стратификации в КНС: T = (T1+T2)/2
//где  T1,T2– температура на кривой стратификации на нижней и верхней границе КНС, оС;
function calcT(T1,T2)
{
    if (!isValidNum(T1)||
        !isValidNum(T2))
    {
      return badvalue();
    }
    return (parseFloat(T1)+parseFloat(T2))/2.;
}

//3. Определяем среднюю температуру на кривой состояния в КНС
function calcTsost(Tsost1,Tsost2)
{
    if (!isValidNum(Tsost1) ||
        !isValidNum(Tsost2))
    {
        return badvalue();
    }
    var tsost = (parseFloat(Tsost1)+parseFloat(Tsost2))/2.;
    return tsost;
}

//4. Определяем среднюю разность между температурой на кривой состояния и на кривой стратификации в КНС
function calcDT(Tsost,T)
{

    if (!isValidNum(Tsost) ||
        !isValidNum(T))
    {
        return badvalue();
    }
    return parseFloat(Tsost)-parseFloat(T);
}

//5. Рассчитываем среднюю скорость вертикальных токов конвекции в КНС
function calcWk(Tsost,P1,P2)
{
    if( !isValidNum( Tsost ) ||
        !isValidNum( P1 ) ||
        !isValidNum( P2 ) )
    {
      return badvalue();
    }
//    if( !isValidNum( Tsost ) )
//    {
//      return "odin";
//    }
//    if( !isValidNum( P1 )  )
//    {
//      return "dwa";
//    }
//    if( !isValidNum( P2 ) )
//    {
//      return "troi";
//    }
    var Wk;
    // Wk = 0;
//    Wk = Math.sqrt(287.*(-2.)*Math.log(parseFloat(500.)/parseFloat(925.)));
//    print(P1);
//    print(P2);
//    print(Tsost);
    Wk = Math.sqrt(287.*parseFloat(Tsost)*Math.log(parseFloat(P1)/parseFloat(P2))); // 287 - удельная газовая постоянная сухого воздуха

//    print(Wk);
    return Wk;
}

//6. Рассчитываем среднюю скорость вертикальных движений в конвективном облаке:

function calcW(m,Wk,Pv,Pn)
{
    if (!isValidNum(m) ||
        !isValidNum(Wk)||
        !isValidNum(Pv)||
        !isValidNum(Pn))
    {
        return badvalue();
    }
    var dP = parseFloat(Pn)-parseFloat(Pv);
    var W = 0.33*Math.pow(10,-6)*(m*Wk*dP);
    return W;
//    return  Math.round(W*100)/100;

//    return zondObj.oprSrKonvSkor(1000,850);
}

//7. Рассчитываем прогностическое (на 12 ч) значение максимальной упругости водяного пара на уровне 850 гПа в точке прогноза, г/кг:
function calcQmax850(T850Adv)
{
    if (!isValidNum(T850Adv))
    {
     return badvalue();
    }
    var qmax850 = (3799.05/850.)*Math.exp((parseFloat(17.13)*parseFloat(T850Adv))/(235.+parseFloat(T850Adv)));
    return qmax850;
}

//8.Рассчитываем интенсивность ливневых осадков , мм/ч
function calcI(qmax850,W)
{
    if (!isValidNum(qmax850) ||
        !isValidNum(W))
    {
     return badvalue();
    }
    var I = 2.88*parseFloat(qmax850)*parseFloat(W);
//    13055
    intens = I;
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13055, I.toFixed(2), I.toFixed(2), 0,1); //??????? верный ли взял дескриптор

//    return qmax850;
    return I;
}

//9. Рассчитываем среднюю скорость перемещения конвективных облаков
function calcV(V700,V500)
{
//    return 'dg';
    if (!isValidNum(V700) ||
        !isValidNum(V500))
    {
        return badvalue();
    }
    var V = 0.7*(parseFloat(V700) + parseFloat(V500))/2.;
    return V;
}

//10. Рассчитываем продолжительность ливневых осадков tл, час

function calcTl(sigma1000850,V)
{
//    return sigma1000850;

//    if( check === 0.)
//    {
//      return 0.;
//    }
    // if( intens == 0.)
    // {
    //   return 0;
    // }
    if (!isValidNum(sigma1000850) ||
        !isValidNum(V))
    {
        return badvalue();
    }
    if (V == 0.)
    {
        return badvalue();
    }
    var tl = 0.1*sigma1000850/V;
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 15007, tl.toFixed(2), tl.toFixed(2), 0,1); //??????? верный ли взял дескриптор
//    15007
    return tl;
}

function floatToHourHuman(sigma1000850,V){
    var tl = calcTl( sigma1000850,V );

    var hour = parseFloat(Math.floor(tl));
    var min = parseFloat(Math.floor(60*(tl-hour)));
    // return tl;
    //
    hour_global = tl;

    return hour+" ч."+min+" мин.";//.arg(min) //+"ч. "+min+"мин."
}

function floatToHour(sigma1000850,V){
    var tl = calcTl( sigma1000850,V );
    hour_global = tl;
    return tl;
}



//11. Рассчитываем количество ливневых осадков Qл, мм:
function calcQl(Il)
{
    if (!isValidNum(Il))
    {
      return badvalue();
    }
    var Ql = parseFloat(Il)*parseFloat(hour_global);

   // dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13011, dataB, dataB, 0,1);

    return Ql;
}

