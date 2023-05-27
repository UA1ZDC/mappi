////14.1 для метода болтанки на посадке (Пинуса)
//V -  скорость ветра в приземном слое
function PinusPrizemn(typeAvia,V)
{
  if(!isValidNum(V) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var ret_val;
  var bol;


  if(typeAvia == "Фронтовая авиация и ИЛ-76 горизонтальный полет"){typeAvia=1;}
  else if(typeAvia == "Фронтовая авиация и ИЛ-76 при заходе на посадку"){typeAvia=2;}
  else if(typeAvia == "Военно-транспортная и дальняя авиация горизонтальный полет"){typeAvia=3;}
  else if(typeAvia == "Военно-транспортная и дальняя авиация при заходе на посадку"){typeAvia=4;}
  else if(typeAvia == "Вертолеты"){typeAvia=5;}
  else typeAvia=1;


  ret_val= pic127(V, typeAvia);
  if(!isValidNum(ret_val) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  switch(ret_val)
  {
    case -1:{dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 8, "Без болтанки", 0,1); return "Болтанка отсутствует";}
    case 1: {return "Исходные данные вне области допустимых значений метода: скорость ветра V < 0 или V > 60 м/с";}
    case 2: {dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 9, "Слабая болтанка", 0,1); return "Болтанка слабая или отсутствует";}
    case 3: {dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 10, "Умеренная болтанка", 0,1); return "Болтанка умеренная";}
    case 4: {dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 11, "Сильная болтанка", 0,1); return "Болтанка сильная";}
    default:{return "Не удалось составить прогноз. Повторите ввод.";}
  }
}

//Функция для оценки интенсивности болтанки bol самолетов и вертолетов при заходе на посадку и горизонтальном полете на предельно малых высотах по значениям скорости ветра V и типу воздушного судна t рис.127
//"Tip vozdushnogo sudna t:
//1-vertoleti
//2-transportnaia i dalniaa aviacia(pri zaxode na posadku)
//3-transportnaia i dalniaa aviacia(gorizontalnii polet s kreiserskoi skorostiu)
//4-frontovaia aviacia i IL-76(pri zaxode na posadku)
//5-frontovaia aviacia i IL-76(gorizontalnii polet s kreiserskoi skorostiu)
function pic127 ( V,  t)
{
   var k=[2.3,1.8,4.0,1.1,2.0];
   var b=[-1.,-1.,-3.,0.,-1.];
   var y1, y2, y3, y4, y5;
//граничные условия
   var x_str=0., x_end=60.;
   var bol;
   if (V<x_str || V>x_end)
   {
     return 1;
   }
   if (V==0.)
   {
     return -1;
   }

   if (t!=1 && t!=2 && t!=3 && t!=4 && t!=5)
   {
     return 1;
   }
   if (t==1)
   {
     bol=parseFloat(k[0]*V)+parseFloat(b[0]);
   }
   if (t==2)
   {

     y2=parseFloat(k[1]*V)+parseFloat(b[1]);
     bol=y2;
   }
   if (t==3)
   {
     y3=parseFloat(k[2]*V)+parseFloat(b[2]);
     bol=y3;
   }
   if (t==4)
   {
     y4=parseFloat(k[3]*V)+parseFloat(b[3]);
     bol=y4;
   }
   if (t==5)
   {
     y5=parseFloat(k[4]*V)+parseFloat(b[4]);
     bol=y5;
   }

  if (bol>=0. && bol<25.)
  {//printf ("Boltanka slabaia ili otsutstvuet!\n\n");
    return 2;
  }
  if (bol>=25. && bol<50.)
  {//printf ("Boltanka umerennaia!\n\n");
//    print("3 ",bol);
    return 3;
  }

  if (bol>=50.)
  {//printf ("Boltanka silnaia!\n\n");
    return 4;
  }

  return 0;

}

