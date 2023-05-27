
//прогноз гроз по Вайтингу
function VaytingK( T500, T700, Td700, D700, T850, Td850, D850 )
{
  
  if( !isValidNum( T500 )   ||
    !isValidNum( T700 )     ||
    (!isValidNum( Td700 )&& !isValidNum( D700 )) ||
    !isValidNum( T850 )   ||
    (!isValidNum( Td850 ) && !isValidNum( D850 )))
  {
    return badvalue();
  }
 // var D850, D700;
  var K;
  if(!isValidNum( D850 )) { D850 = parseFloat(T850) - parseFloat(Td850);}
  if(!isValidNum( D700 )) { D700 = parseFloat(T700) - parseFloat(Td700);}
 
  K = 2*(T850) - parseFloat(D850) - parseFloat(D700) - T500;
 
 
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13044, K.toFixed(2),K.toFixed(2), 0,1);
 
  return K;
}

function VaytingDescr ( K )
{
  if( !isValidNum( K ) )    
        return badvalue();
  
  var G1 = "Без грозы", G2 ="Грозы местами", G3 = "Грозы повсеместно";
  var result = G1;
  var res_val;

  if(K >= 30)
  {
    result =  G3;
    res_val = 9;
  }
  else if (K > 20 && K < 30)
  {
    result = G2;
    res_val = 9;
  }
  else if (K <= 20)
  {
    result = G1;
    res_val = 0;
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, res_val, result, 0,1);
 
  return result;
}
