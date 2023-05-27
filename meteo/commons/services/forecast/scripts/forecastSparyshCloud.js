function badvalue()
{
  return -9999;
}


function NGOSparyshkinoi (Tprog6h, gradT)
{

  if( !isValidNum(Tprog6h)||
      !isValidNum(gradT) )
  {
    return "Метод не работает с заданными значениями параметров";
  }
  Tprog6h = parseFloat(Tprog6h);
  gradT   = parseFloat(gradT);

  var ret_val, result;
  ret_val = pic10_9(Tprog6h, gradT);
//  return ret_val;
  switch(ret_val)
  {
    case 0:
           result="Менее 200";//менее 200 м
           dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, result, result, 0,1);
           return result;

    case 2:
           result="200-300";//от 200 до 300 м
           dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, 250, 250, 0,1);
           return result;
    case 4:
           result="Более 300";//более 300 м
           dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20013, result, result, 0,1);
           return result;
    case "err1":
           result="Не удалось произвести расчет: значение прогноза температуры вне диапазона от -12 до 12";
           return result;
    case "err2":
           result="Не удалось произвести расчет: значение прогноза градиента температуры вне диапазона от -3 до 3";
           return result;
    case "err3":
           result="Не удалось произвести расчет: неизвестная ошибка";
           return result;
    default:
           return "Метод не работает с заданными значениями параметров";
  }
}


function pic10_9 (Tp6, VgradT)
{

  if( !isValidNum(Tp6) ||
      !isValidNum(VgradT) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  Tp6    = parseFloat(Tp6);
  VgradT = parseFloat(VgradT);

  var a = [-0.0341, -0.0234];
  var b = [0.0341, 0.0543];
  var c = [1.069, 1.883];
  var y1, y2;

  if (Tp6 < -12. || Tp6 > 12.)
  {
    return "err1";
  }
  if (VgradT < -3. || VgradT > 3.)
  {
    return "err2";
  }

  y1=parseFloat(a[0]*Tp6*Tp6)+parseFloat(b[0]*Tp6)+parseFloat(c[0]);
  y2=parseFloat(a[1]*Tp6*Tp6)+parseFloat(b[1]*Tp6)+parseFloat(c[1]);

  if (VgradT < y1)
  {
    return 0;
  }
  if (VgradT >= y1 && VgradT < y2)
  {
    return 2;
  }
  if (VgradT >= y2)
  {
    return 4;
  }
  return "err3";

}


function gradT(T0, Tinv, Hinv)
{
  T0 = parseFloat(T0);
  Tinv = parseFloat(Tinv);
  Hinv = parseFloat(Hinv);

  var grad;

  if( !isValidNum(T0) ||
      !isValidNum(Tinv) ||
      !isValidNum(Hinv) )
  {
    return "Метод не работает с заданными входными значениями параметров";
  }
  grad = 100*((parseFloat(T0) - parseFloat(Tinv))/parseFloat(Hinv));
  grad = parseFloat(grad);
  return grad;

}




//Возвращает значение температуры на уровне 925 гПа (вместо уровня 500-600 м)
function T925Point()
{
  var T925;
//  zondObj.resetData();
  T925 = zondObj.getUr(obj.getStationData(),925, 2);

  if(!isValidNum(T925))
  {
    return badvalue();
  }

//  T925 = parseFloat(T925);
  return T925;
}


//Возвращает значение высоты уровня 925 гПа
function getH925point()
{
  var H600;
//  zondObj.resetData();
  H600 = zondObj.getUr(obj.getStationData(),925, 1);

  if( H600 == badvalue())
  {
    return badvalue();
  }

  H600 = parseFloat(H600);
  return H600;

}

function getTinvers() //Возвращает температуру на верхней границе инверсии(обычной инверсии)
{
  var Tinv;
  Tinv  = zondObj.getInvData(obj.getStationData(),3);

  if (!isValidNum(Tinv))
  {
    Tinv = T925Point();
    if( !isValidNum(Tinv) )
    {
      return "Инверсия не обнаружена, а температуру на высоте 925гпа запросить не удалось";
    }
    return Tinv;
  }
  Tinv = parseFloat(Tinv);
  return Tinv;


}

function getHinvers() //Возвращает высота верхней границы инверсии
{
    var Hinv;
    Hinv  = zondObj.getInvData(obj.getStationData(),1);

    if (!isValidNum(Hinv))
    {
      Hinv = getH925point();
      if( Hinv == badvalue() )
      {
        return "Инверсия не обнаружена";
      }
  //    Hinv = parseFloat(Hinv);
      return Hinv;
    }
//    Hinv = parseFloat(Hinv);
    return Hinv;

}






