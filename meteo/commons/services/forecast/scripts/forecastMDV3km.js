
function getFormul_MDV15( synop_sit, precip, St, Stm15, Stm30, Stm45 )
{
  if(!isValidNum(St) ||
    !isValidNum(Stm15) ||
    !isValidNum(Stm30) ||
    !isValidNum(Stm45) )
   return "Метод не работает с заданными значениями параметров";

  if(  Stm15 == 0.   ||
       Stm30 == 0.   ||
       Stm45 == 0.   )
  {
    return "Метод не работает с заданными значениями параметров";
  }
 
 St = parseFloat(St);
 if ( St>20 ){
  St = (St)/1000.;
 }
 Stm15 = parseFloat(Stm15);
 if (Stm15>20) {
  Stm15 = (Stm15)/1000.;
 }
 Stm30 = parseFloat(Stm30);
 if (Stm30>20) {
  Stm30 = (Stm30)/1000.;
 }
 Stm45 = parseFloat(Stm45);
 if (Stm45>20) {
  Stm45 = (Stm45)/1000.;
 }

 
 var S1, S2;
 var Stp15;
 // var dtime = "0.25;0.25";
 
 S1= ((St)+(Stm15)+(Stm30))/3.;
 S2= ((Stm15)+(Stm30)+(Stm45))/3.;
 var dataB;
 if( precip == "Без осадков" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.8*S1 - 0.8*S2;
     dataB = Stp15*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB.toFixed(2), dataB.toFixed(2),  0,1);
     return Stp15;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     dataB = Stp15*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB.toFixed(2), dataB.toFixed(2),  0,1);
     return Stp15;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     dataB = Stp15*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB.toFixed(2), dataB.toFixed(2),  0,1);
     return Stp15;
   }
 
 }else
 if( precip == "Осадки" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     dataB = Stp15*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB.toFixed(2), dataB.toFixed(2),  0,1);
     return Stp15;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     dataB = Stp15*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB.toFixed(2), dataB.toFixed(2),  0,1);
     return Stp15;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     dataB = Stp15*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB.toFixed(2), dataB.toFixed(2),  0,1);
     return Stp15;
   }
 }
}

function getFormul_MDV30( synop_sit, precip, St, Stm15, Stm30, Stm45 )
{
  if(!isValidNum(St) ||
    !isValidNum(Stm15) ||
    !isValidNum(Stm30) ||
    !isValidNum(Stm45) )
   return "Метод не работает с заданными значениями параметров";

  if(  Stm15 == 0.   &&
       Stm30 == 0.   &&
       Stm45 == 0.   )
  {
    return "Метод не работает с заданными значениями параметров";
  }
 

 St = parseFloat(St);
 if ( St>20 ){
  St = (St)/1000.;
 }
 Stm15 = parseFloat(Stm15);
 if (Stm15>20) {
  Stm15 = (Stm15)/1000.;
 }
 Stm30 = parseFloat(Stm30);
 if (Stm30>20) {
  Stm30 = (Stm30)/1000.;
 }
 Stm45 = parseFloat(Stm45);
 if (Stm45>20) {
  Stm45 = (Stm45)/1000.;
 }

 
 
 var S1, S2;
 var Stp15, Stp30;
 var dataB;
 // var dtime = "0.5;0.5";
 
 S1= (parseFloat(St)+parseFloat(Stm15)+parseFloat(Stm30))/3.;
 S2= (parseFloat(Stm15)+parseFloat(Stm30)+parseFloat(Stm45))/3.;
 
 if( precip == "Без осадков" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     dataB = Stp30*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp30;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     dataB = Stp30*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp30;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     dataB = Stp30*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp30;
   }
 
 } else
 if( precip == "Осадки" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     dataB = Stp30*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp30;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     dataB = Stp30*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp30;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     dataB = Stp30*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp30;
   }
 }

}

function getFormul_MDV45( synop_sit, precip, St, Stm15, Stm30, Stm45 )
{
  if(!isValidNum(St) ||
    !isValidNum(Stm15) ||
    !isValidNum(Stm30) ||
    !isValidNum(Stm45) )
   return "Метод не работает с заданными значениями параметров";

  if(  Stm15 == 0.   &&
       Stm30 == 0.   &&
       Stm45 == 0.   )
  {
    return "Метод не работает с заданными значениями параметров";
  }
 

 St = parseFloat(St);
 if ( St>20 ){
  St = (St)/1000.;
 }
 Stm15 = parseFloat(Stm15);
 if (Stm15>20) {
  Stm15 = (Stm15)/1000.;
 }
 Stm30 = parseFloat(Stm30);
 if (Stm30>20) {
  Stm30 = (Stm30)/1000.;
 }
 Stm45 = parseFloat(Stm45);
 if (Stm45>20) {
  Stm45 = (Stm45)/1000.;
 }

 
 
 var S1, S2;
 var Stp15, Stp30;
 var Stp45;
 var dataB;

 
 S1= (St+Stm15+Stm30)/3.;
 S2= (Stm15+Stm30+Stm45)/3.;
 // var dtime = "0.75;0.75";
 
 if( precip == "Без осадков" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     Stp45 = 1.8*Stp30 - 0.8*Stp15;
     dataB = Stp45*1000.;
     dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp45;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     Stp45 = 1.7*Stp30 - 0.7*Stp15;
       dataB = Stp45*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp45;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     Stp45 = 1.8*Stp30 - 0.8*Stp15;
       dataB = Stp45*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp45;
   }
 
 }else
 if( precip == "Осадки" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     Stp45 = 1.7*Stp30 - 0.7*Stp15;
       dataB = Stp45*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp45;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     Stp45 = 1.7*Stp30 - 0.7*Stp15;
       dataB = Stp45*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp45;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     Stp45 = 1.8*Stp30 - 0.8*Stp15;
       dataB = Stp45*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp45;
   }
 }
}

function getFormul_MDV60( synop_sit, precip, St, Stm15, Stm30, Stm45 )
{
  if(!isValidNum(St) ||
    !isValidNum(Stm15) ||
    !isValidNum(Stm30) ||
    !isValidNum(Stm45) ){
   return "Метод не работает с заданными значениями параметров";
  }
 

  if(  Stm15 == 0.   ||
       Stm30 == 0.   ||
       Stm45 == 0.   )
  {
    return "Метод не работает с заданными значениями параметров";
  }


 St = parseFloat(St);
 if ( St>20 ){
  St = (St)/1000.;
 }
 Stm15 = parseFloat(Stm15);
 if (Stm15>20) {
  Stm15 = (Stm15)/1000.;
 }
 Stm30 = parseFloat(Stm30);
 if (Stm30>20) {
  Stm30 = (Stm30)/1000.;
 }
 Stm45 = parseFloat(Stm45);
 if (Stm45>20) {
  Stm45 = (Stm45)/1000.;
 }

  
  
 
 var S1, S2;
 var Stp15, Stp30;
 var Stp45, Stp60;
 var dataB;
 // var dtime = "1;1";
 
 S1= (parseFloat(St)+parseFloat(Stm15)+parseFloat(Stm30))/3.;
 S2= (parseFloat(Stm15)+parseFloat(Stm30)+parseFloat(Stm45))/3.;
 
 if( precip == "Без осадков" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     Stp45 = 1.8*Stp30 - 0.8*Stp15;
     Stp60 = 1.8*Stp45 - 0.8*Stp30;
       dataB = Stp60*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp60;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     Stp45 = 1.7*Stp30 - 0.7*Stp15;
     Stp60 = 1.7*Stp45 - 0.7*Stp30;
       dataB = Stp60*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp60;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     Stp45 = 1.8*Stp30 - 0.8*Stp15;
     Stp60 = 1.8*Stp45 - 0.8*Stp30;
       dataB = Stp60*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp60;
   }
 
 }else
 if( precip == "Осадки" ){
   if( synop_sit == "Теплый фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     Stp45 = 1.7*Stp30 - 0.7*Stp15;
     Stp60 = 1.7*Stp45 - 0.7*Stp30;
       dataB = Stp60*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp60;
   }else
   if( synop_sit == "Холодный фронт"){
     Stp15 = 1.7*S1 - 0.7*S2;
     Stp30 = 1.7*Stp15 - 0.7*S1;
     Stp45 = 1.7*Stp30 - 0.7*Stp15;
     Stp60 = 1.7*Stp45 - 0.7*Stp30;
       dataB = Stp60*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp60;
   }else
   if( synop_sit == "Внутримассовый процесс"){
     Stp15 = 1.8*S1 - 0.8*S2;
     Stp30 = 1.8*Stp15 - 0.8*S1;
     Stp45 = 1.8*Stp30 - 0.8*Stp15;
     Stp60 = 1.8*Stp45 - 0.8*Stp30;
       dataB = Stp60*1000.;
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, dataB, dataB,  0,1);
     return Stp60;
   }
 }
}

