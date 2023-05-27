
//Функция для расчета МДВ в дожде
function MDV(intensiv,ff,tip) //интеснивность в мм/ч
{
  if( !isValidNum( intensiv ) ||
    !isValidNum( ff ) || intensiv <=0. )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  intensiv = parseFloat(intensiv);
  ff = parseFloat(ff);

  var Vid;
  var dataB;
  if (tip == "Дождь")
  {
      var result;
      if((intensiv < 0.0 || intensiv > 40.))  {return "Исходные данные вне области допустимых значений метода: интенсивность от 0 до 40 мм/ч";}

      Vid=13.6*Math.pow(intensiv,-0.79);
      if(Vid==badvalue()) {
        return "Метод не работает с заданными значениями параметров";
      }
      if (Vid>=10.)
      {
          dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, 8000, 8000, 0,1);
          return Vid="6-10";
      }
      dataB = Vid*1000;
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, dataB.toFixed(2), dataB.toFixed(2), 0,1);

      return Vid;

  }
  if (tip=="Снег")
  {
      var ret;
      var ret_val;

      intensiv/=10.;//26102007AS
      if(intensiv == badvalue()) {
        return "Метод не работает с заданными значениями параметров";
      }
      if(intensiv < 0.)  {return "Исходные данные вне области допустимых значений метода: интенсивность осадков меньше 0";}
      if((ff <= 0. || ff > 26.))  {return "Скорость ветра вне области допустимых значений метода (от 0 до 26)";}

      if(ff<=3.)
      {
         if(intensiv <= 0.||intensiv > 1.3)  {return "Исходные данные вне области допустимых значений метода: интенсивность снегопада при слабом ветре от 0 до 1.3";}
         Vid=pic86a(intensiv);
         if (Vid == "err1"){return "Исходные данные вне области допустимых значений метода: интенсивность снегопада при слабом ветре от 0 до 1.3";}
         if (Vid == badvalue()){
          return "Метод не работает с заданными значениями параметров";
         }
         dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, Vid*1000, Vid*1000, 0,1);
         return Vid;
      }
      if(ff>3.)
      {
        Vid=pic112a(intensiv, ff);
        if (Vid == badvalue()){
          return "Метод не работает с заданными значениями параметров";
        }
        if (Vid == "err1"){return "Исходные данные вне области допустимых значений метода: При данной интенсивности снегопада скорость ветра V>3 и V<16";}
        if (Vid == "err2"){return "Исходные данные вне области допустимых значений метода: При данной интенсивности снегопада скорость ветра V>3.5 и V<22";}/*(V>3. && V<3.5)||(V>22. && V<=26.)*/
        if (Vid == "err3"){return "Исходные данные вне области допустимых значений метода: При данной интенсивности снегопада скорость ветра V>3 и V<=20";}
        dataB = Vid*1000;
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20001, dataB.toFixed(2), dataB.toFixed(2), 0,1);
        return Vid;
      }
  }
  return "Метод не работает с заданными значениями параметров";

}

//График для прогноза видимости при снеге и ветре до 3 м/с (рис. 86)
function pic86a(I)
{
  if (!isValidNum(I))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  var Sm;
  if (I<0. || I>1.3) {return "err1"};
  if (I<0.15)  Sm=-30.*I+5.75;
  else Sm=1.78*I*I-3.75*I+2.68;
  return Sm;
}

//График зависимости МДВ в снеге от интенсивности снегопада и скорости ветра более 3 м/с
function pic112a( intens, V)
{
  var Smin, Smax, result;
  if (!isValidNum(intens) ||
      !isValidNum(V) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var k=[-0.14,-0.25,-0.45];
  var b=[3.79,6.5,10.6];
  var y1,y2,y3;

  y1=parseFloat(k[0]*V)+parseFloat(b[0]);
  y2=parseFloat(k[1]*V)+parseFloat(b[1]);
  y3=parseFloat(k[2]*V)+parseFloat(b[2]);

//Сильный снег
  if (intens>=3.)
  {
    if (V>3. && V<5.) {return "err1"};
    if (V>=5. && V<16.)
    {
      Smin=0.2;
      Smax=1.5;
    }
    if (V>=16. && V<=26.)
    {
      Smin=0.1;
      Smax=y1;
    }
  }
//Умеренный снег
  if (intens>=1. && intens<3.)
  {
    if ((V>3. && V<3.5)||(V>22. && V<=26.)) {return "err2"};
    if (V>=3.5 && V<15.)
    {
      Smin=1.;
      Smax=3.;
    }
    if (V>=15. && V<=22.)
    {
      Smin=1.;
      Smax=y2;
    }
  }
  if (intens<1.)
  {
    if (V>20.) {return "err3"};
    if (V>3. && V<14.)
    {
      Smin=2.;
      Smax=4.2;
    }
    if (V>=14. && V<=20.)
    {
      Smin=2.;
      Smax=y3;
    }

  }
  result = (parseFloat(Smin)+parseFloat(Smax))*0.5
  return result;
}

