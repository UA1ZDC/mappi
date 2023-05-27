//Функция для прогноза высоты ветровых волн на 12
function prognVolnaAll(Hvol_t0,ff_t0,dd_t0,progFF,progDD)
{

      if( !isValidNum( Hvol_t0 )   ||
          !isValidNum( ff_t0 )     ||
          !isValidNum( dd_t0 )  ||
          !isValidNum( progFF )  ||
          !isValidNum( progDD ))
      {
        return "Метод не работает с заданными значениями параметров";
      }

  var zabl,i;

  if (ff_t0<5. && progFF<5.) {return "Волны отсутствуют";}
//  for(i=0;i<(zabl/12.);i++)

  Hvol_t0 = ftoi_norm(prognVolna12(Hvol_t0,ff_t0,dd_t0,progFF,progDD));

  if( Hvol_t0 !== -999 )
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 22021, Hvol_t0.toFixed(2), Hvol_t0.toFixed(2), 0,1);
  }
  else
  {
     return "Волны отсутствуют";
  }


  return Hvol_t0;

}


function prognVolna12( Hvol_t0, ff_t0, dd_t0, progFF, progDD)
{
  var dff,k,dd,H12Lim,H0Lim,Per_t0,Per_t12,T_ekv,T_eff,X_ekv,X_eff;

  dff = progFF-ff_t0;//Расчет разницы скоростей ветра в пункте погноза в срок t0 и через 12 часов у поверхности моря
  k = Math.abs(-dff/12.);//Расчет абсолютного значения скорости затухания ветра за 12 часов
  dd = Math.abs(progDD-dd_t0);//Расчет изменения направления ветра в пункте прогноза через 12 часов у поверхности моря
  H12Lim = 0.0206*progFF*progFF;//Расчет предельной высоты волны
  H0Lim = 0.0206*ff_t0*ff_t0;//Расчет предельной высоты волны
  Per_t0 = 0.526*ff_t0;//Расчет предельного периода волны
  Per_t12 = 0.526*progFF;//Расчет предельного периода волны


  var b = parseFloat(1.)-parseFloat(Hvol_t0/H0Lim);
  b = parseFloat(b);

  if( b < 0. )
  {
    return -999;// "Отношение высоты волны в исходный срок и предельной высоты волны меньше нуля.";
  }

  var a = parseFloat(-0.769)*Math.log(b);
//  print(a);


  T_ekv = Per_t0*Math.pow(a,2);//Расчет эквивалентного времени, потребного для разгона волны до высоты h0
  T_eff = parseFloat(T_ekv) + parseFloat(12.); //Расчет эквивалентного времени, потребного для разгона волны до высоты h12
//  print(T_eff);
  X_ekv = Per_t0*ff_t0*Math.pow((-0.847*Math.log(b)),2);//Расчет эквивалентного расстояния для разгона волны до высоты h0
//  print(X_ekv);
  X_eff = parseFloat(X_ekv)+parseFloat(300.);//Расчет эффективного расстояния для разгона волны до высоты h12

  if(ff_t0>=5. && dff>=0. && dd<=45.)
      return  Math.max(H12Lim*(1.-Math.exp(-1.3*Math.pow((T_eff/Per_t12),0.6))),H12Lim*(1.-Math.exp(-1.18*Math.pow((X_eff/(progFF*Per_t12)),0.5))));
  if(ff_t0>=5. && dff>=0. && dd>45.)
      return Math.max(H12Lim*(1.-Math.exp(-1.3*Math.pow((12./Per_t12),0.6))),H12Lim*(1.-Math.exp(-1.18*Math.pow((300./(progFF*Per_t12)),0.5))));
  if(ff_t0>=5. && dff<0.)
      return 1.6*Hvol_t0*Math.exp(-12./(parseFloat(14.7)+parseFloat(3.78/k)));
}




