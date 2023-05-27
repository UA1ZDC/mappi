
function badvalue()
{
  return -9999;
}


//максимальная температура по Титову
function TitovTMax(NObl, T700, Td700,D700, T850, Td850,D850, T900 )
{
  
    if(!isValidNum(T700) ||
       (!isValidNum(Td700 ) && !isValidNum(D700))  ||
       !isValidNum(T850) ||
       (!isValidNum(Td850 ) && !isValidNum(D850))  ||
       !isValidNum(T900) ){
   return "Метод не работает с заданными значениями параметров";
    }
// if(obl > 10 ) obl = obl/10;
 if(NObl > 7)  return "Для работы метода необходимо, чтобы количество облаков было не больше 7 баллов";

 var Ds, Tmax;
 var T1,T2;
 T1 = 8.;
 T2 = 12.; // Для неадаптированных методов
 if(!isValidNum( D850 )) { D850 = parseFloat(T850) - parseFloat(Td850);}
 if(!isValidNum( D700 )) { D700 = parseFloat(T700) - parseFloat(Td700);}

 Ds = parseFloat(D850) + parseFloat(D700);

 if(Ds < 3.5) {
   Tmax =  parseFloat(T900) + parseFloat(T1); 
 } else {
   Tmax = parseFloat(T900) + parseFloat(T2);
 }

 dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 12111, Tmax.toFixed(2),Tmax.toFixed(2), 0,1);
 return Tmax;
}
