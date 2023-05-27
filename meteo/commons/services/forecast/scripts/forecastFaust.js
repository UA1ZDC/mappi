
var kDt1 = 0;

//прогноз гроз по Фаусту
function FaustK( T500, Td500,D500, T700, Td700, D700, T850, Td850, D850, dT1)
{

  if( !isValidNum( T500 ) ||
    (!isValidNum( Td500 ) && !isValidNum( D500 ))  ||
    !isValidNum( T700 )   ||
    (!isValidNum( Td700 ) && !isValidNum( D700 ))  ||
    !isValidNum( dT1 )    ||
    !isValidNum( T850 )   ||
    (!isValidNum( Td850 ) && !isValidNum( D850 ))  )
  {
    return badvalue();
  }

 if(dT1 < 0.){
    dT1 = -dT1;
 }
 
 
 var  D, Tv, dT, z, dT2;

 if(!isValidNum( D850 )) {D850 = parseFloat(T850) - parseFloat(Td850);}
 if(!isValidNum( D700 )) {D700 = parseFloat(T700) - parseFloat(Td700);}
 if(!isValidNum( D500 )) {D500 = parseFloat(T500) - parseFloat(Td500);}
 D = (parseFloat(D850) + parseFloat(D700) + parseFloat(D500))/3.;
 Tv = -29.05+1.414*T850 - 0.5126*D-0.01715*T850*D+0.004362*Math.pow(T850,2);
 
 if (dT1 > 1000.) 
 {
   z = 0.;
 }
 
 if (dT1 > 500. && dT1<=1000.) 
 {
   z = 1.;
 }
 
 if (dT1 > 250. && dT1<=500.) 
 {
   z = 1.4;
 }
 
 if (dT1 > 0. && dT1<=250.) 
 {
   z = 2.;
 }
 
 dT2 = 0;
 
 
 //dT1 = z*(2.867-0.004*Math.abs(R)+0.000002133*Math.abs(R)*Math.abs(R));
 dT = Tv - T500 + (parseFloat(z) + parseFloat(dT2));
 if(isValidNum(dT)){
   //dT1 = z*(2.867-0.004*Math.abs(R)+0.000002133*Math.abs(R)*Math.abs(R));
   dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13044, dT.toFixed(2),dT.toFixed(2), 0,1);
 }
 return dT;
}


function FaustDescr( K )
{
  if (!isValidNum(K))
         return badvalue();
  var G1 = "Без грозы", G2 ="Гроза";
  var result = G1;
  var res_val;

  if (K >=-3)
  {
    result = G2;
    res_val = 9;
  }
  else
  {
    result = G1;
    res_val = 0;
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, res_val, result, 0,1);

  return result;
}

function FaustDt1()
{
    if (!isValidNum(kDt1)) { return badvalue(); }
    return kDt1;
}
