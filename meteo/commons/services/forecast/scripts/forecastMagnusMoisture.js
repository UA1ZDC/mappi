
//прогноз влажности (относительной влажности) по методу Магнуса
function getVlaznostMagnus( T, Td )
 {
  if(!isValidNum(T) ||
     !isValidNum(Td) ){
     return "Метод не работает с заданными значениями параметров";
  }
    T = parseFloat(T);
    Td = parseFloat(Td);
    var a,b;
    var RH;

    if (T<-10.) {
      a=9.5;
      b=265.5;      
    }
    else {
      a=7.63;
      b=241.9;
    }
    if ( (b+Td) != 0 && (b+T)!=0 ){
      RH = 100*(Math.exp((a*Td)/(b+Td))/Math.exp((a*T)/(b+T)));
    }else{
      return "Невозможно рассчитать формулу, так как присутствует деление на 0"
    }
    if (RH<0.){
      RH = 0;
    }
    if (RH>100.){
      RH = 100;
    }
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 13003, RH.toFixed(2), RH.toFixed(2), 0, 1);
    return ftoi_norm(RH);

 }
