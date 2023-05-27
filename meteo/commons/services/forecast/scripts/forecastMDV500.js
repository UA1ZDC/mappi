
// Функция для расчета вероятности МДВ менее 500 м в различных ЯП
//метод применим только при сухой почве!
function verMDV500(grad,vrem_goda)
{

  if( !isValidNum( grad ) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

 /* grad - градиент давления
   result - состояние почвы
*/

 var type, P, result, prog;

// return 1;
// vrem_goda = ;

 if (grad<0.) {result="менее 50";return result;}
 if (grad>4) {result="100";return result;}

 if(vrem_goda=="Теплый")
 {
   if(type==0)
   {
     if (grad<0.9) {result="менее 50";return result;}
     if (grad>2.6) {result="100";return result;}
     P = pic114aA(grad);
     if(P!=-9999)
     {
       prog = P;
     }
     else {result="";return result;}
   }
   else
   {
     if (grad<1.4) {result="менее 50";return result;}
     if (grad>3.3) {result="100";return result;}
     P = pic114bA(grad);
     if(P!=-9999)
     {
       prog = P;
     }
     else {result="";return result;}
   }
 }
 else
 {
   if(type==1)
   {
     if (grad<1.2) {result="менее 50";return result;}
     if (grad>3.2) {result="100";return result;}
     P=pic114aB(grad);
     if(P!=-9999)
     {
       prog = P;
     }
     else {result="";return result;}
   }
   else
   {
     if (grad<2.1) {result="менее 50";return result;}
     if (grad>4) {result="100";return result;}
     P=pic114bB(grad);
     if(P !== -9999)
     {
       prog = P;
     }
     else {result="";return result;}
   }
 }

//  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),"20001_33045", prog, 0,1);
//  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),20001, prog, prog,0,1);
  return prog;
}

////Функция для оценки вероятности P уменьшения видимости при пыльных бурях до 500 м и менее
////для теплого полугодия южной части Средней Азии по значению барического градиента в //гектопаскалях на 100 км рис.114
function pic114aA(grad)
{
  var a=-15.9317;
  var b= 85.173;
  var c=-13.7515;
  var y;

  if (grad<0. || grad>4.) {
    return "Метод не работает с заданными значениями параметров";
  }
  if (grad<0.9) { return 0;}
  if (grad>2.6) { return 100;}

  y=parseFloat(a*grad*grad)+parseFloat(b*grad)+parseFloat(c);
  if (y>100.){ y=100;};
  return y;
}

//Функция для оценки вероятности P уменьшения видимости при пыльных бурях до 500 м и менее
//для холодного полугодия южной части Средней Азии по значению барического градиента в //гектопаскалях на 100 км рис.114
function pic114aB(grad)
{
  var a=-10.4166;
  var b=70.833;
  var c=-20.0;
  var y;

  if (grad<0. || grad>4.) { return badvalue();}
  if (grad<1.2) { return 0;}
  if (grad>3.2) { return 100;}

  y=parseFloat(a)*parseFloat(grad)*parseFloat(grad)+parseFloat(b)*parseFloat(grad)+parseFloat(c);
  if (y>100.) y=100.;
  return y;
}

////Функция для оценки вероятности P уменьшения видимости при пыльных бурях до 500 м и менее
////для теплого полугодия северной части Средней Азии по значению барического градиента в //гектопаскалях на 100 км рис.114
function pic114bA(grad)
{
  var a=-12.5598;
  var b=85.3468;
  var c=-44.8683;
  var y;

  if (grad<0. || grad>4.) {
    return "Метод не работает с заданными значениями параметров";
  }
  if (grad<1.4) { return 0;}
  if (grad>3.3) { return 100;}

  y=parseFloat(a*grad*grad)+parseFloat(b*grad)+parseFloat(c);
  if (y>100.) y=100.;
  return y;
}


////Функция для оценки вероятности P уменьшения видимости при пыльных бурях до 500 м и менее
////для холодного полугодия северной части Средней Азии по значению барического градиента в //гектопаскалях на 100 км рис.114
function pic114bB(grad)
{
  var a=-9.7222;
  var b=86.1109;
  var c=-87.9579;
  var y;

  if (grad>4.) {
    return 100;
  }
  if (grad<2.1) { return 50;}

  y=parseFloat(a*grad*grad)+parseFloat(b*grad)+parseFloat(c);
  if (y>100.) y=100.;
  return y;
}


