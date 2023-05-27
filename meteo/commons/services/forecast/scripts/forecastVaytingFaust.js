
function VaytingK( T500, T700, Td700,D700, T850, Td850, D850 )
{
  
  if( !isValidNum( T500 )   ||
    !isValidNum( T700 )     ||
    (!isValidNum(Td700 ) && !isValidNum(D700))  ||
    !isValidNum( T850 )     ||
    (!isValidNum(Td850 ) && !isValidNum(D850)) )
  {
    return badvalue();
  }
  var K;
  if(!isValidNum( D850 )) {D850 = parseFloat(T850) - parseFloat(Td850);}
  if(!isValidNum( D700 )) {D700 = parseFloat(T700) - parseFloat(Td700);}

  K = 2*(T850) - parseFloat(D850) - parseFloat(D700) - T500;
  
  
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13044, K.toFixed(2),K.toFixed(2), 0,1);
  
  return K;
}




//прогноз гроз по Фаусту
function FaustK( T500, Td500, D500, T700, Td700, D700, T850, Td850, D850, dT1)
{
  if( !isValidNum( T500 )   ||
    (!isValidNum(Td500 ) && !isValidNum(D500))  ||
    !isValidNum( T700 )     ||
    (!isValidNum(Td700 ) && !isValidNum(D700))  ||
    !isValidNum( dT1 )     ||
    !isValidNum( T850 )   ||
    (!isValidNum(Td850 ) && !isValidNum(D850))  )
  {
    return badvalue();
  }
  
  if(dT1 < 0.){
    dT1 = -dT1;
  }
  
 
 var D, Tv, dT, z, dT2;

  if(!isValidNum( D850 )) {D850 = parseFloat(T850) - parseFloat(Td850);}
  if(!isValidNum( D700 )) {D700 = parseFloat(T700) - parseFloat(Td700);}
  if(!isValidNum( D500 )) {D500 = parseFloat(T500) - parseFloat(Td500);}
 D = (parseFloat(D850) + parseFloat(D700) + parseFloat(D500))/3;
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
   dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13044, dT.toFixed(2),dT.toFixed(2), 0,1);
 }
 
 
 return dT;
}

////прогноз гроз по Ватингу
//function VaytingK( T500, T700, Td700, T850, Td850 )
//{
// if(T500  == badvalue() ||
//    T700 == badvalue() ||
//    Td700  == badvalue() ||
//    T850 == badvalue() ||
//    Td850  == badvalue() )
//   return badvalue();
 
// var D850, D700;
// var K;
// var G1 = "Без грозы", G2 ="Грозы местами", G3 = "Грозы повсеместно";
// var result = G1;
// D850 = parseFloat(T850) - parseFloat(Td850);
// D700 = parseFloat(T700) - parseFloat(Td700);
 
// K = 2*(T850) - parseFloat(D850) - parseFloat(D700) - T500;

// if(K >= 30)
// {
//   result =  G3;
// }
// else if (K > 20 && K < 30)
// {
//   result = G2;
// }
// else if (K <= 20)
// {
//   result = G1;
// }
// return Math.round(K*100)/100;
//}

////прогноз гроз по Фаусту
//function FaustK( T500, Td500, T700, Td700, T850, Td850, dT1)
//{
// if(T500  == badvalue() ||
//    Td500 == badvalue() ||
//    T700 == badvalue() ||
//    Td700  == badvalue() ||
//    T850 == badvalue() ||
//    Td850  == badvalue() )
//   return badvalue();

// var D850, D700, D500, D, Tv, dT, z, dT2;

// D850 = parseFloat(T850) - parseFloat(Td850);
// D700 = parseFloat(T700) - parseFloat(Td700);
// D500 = parseFloat(T500) - parseFloat(Td500);
// D = (parseFloat(D850) + parseFloat(D700) + parseFloat(D500))/3;
// Tv = -29.05+1.414*T850 - 0.5126*D-0.01715*T850*D+0.004362*Math.pow(T850,2);
// /*z = 1;
// if (R > 0)
// {
//   z = 1;
// }
// else
// {
//   z = -1;
// }*/
// dT2 = 0;
// //dT1 = z*(2.867-0.004*Math.abs(R)+0.000002133*Math.abs(R)*Math.abs(R));
// dT = Tv - T500 + (parseFloat(dT1) + parseFloat(dT2));

// return Math.round(dT*100)/100;
//}

function VaytingFaustDescr( KV, KF )
{
    var res_val;
    if (KV == badvalue() || KF == badvalue() || isNaN(KV) || isNaN(KF))
           return badvalue();
 var G1 = "Без грозы", G2 ="Грозы местами", G3 = "Грозы повсеместно";
 var result = G1;

 if(KV <20)
 {
   result =  G1;
   res_val = 0;
   
 }
 else if (KF >= -3)
 {
   result = G3;
   res_val = 9;
   
 }
 else
 {
   result = G2;
   res_val = 9;
   
 }
 
 dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, res_val,result, 0,1);
 
 return result;
}

