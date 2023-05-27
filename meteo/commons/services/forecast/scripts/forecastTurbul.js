
function funcDx(x1, x2, h1,h2)
{
  
  if(!isValidNum(h1)||!isValidNum(h2)||
    !isValidNum(x1)||
    !isValidNum(x2))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  return parseFloat(x2-x1)/parseFloat(h2-h1);
}

function turb(dudx, dvdy, dvdx, dudy)
{
  if(!isValidNum(dudx)||
      !isValidNum(dvdy)||
      !isValidNum(dudy)||
      !isValidNum(dvdx))
  {
   return "Метод не работает с заданными значениями параметров";
  }

  var DEF;
  dudx = dudx*1.e-2;
  dvdy = dvdy*1.e-2;
  dvdx = dvdx*1.e-2;
  dudy = dudy*1.e-2;
 
  DEF = Math.sqrt(Math.pow((dudx - dvdy) ,2) + Math.pow((dvdx - dudy),2));

  return DEF;

}

function vs(dudz,dvdz) //вертикальный сдвиг ветра
{
  if(!isValidNum(dudz) ||
     !isValidNum(dvdz))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var result;
  result = Math.sqrt(Math.pow(dudz,2) + Math.pow(dvdz,2));
  return result;
}

function dvsi(def,vs,V) //V - скорость на уровне
{
  if(!isValidNum(def)||
      !isValidNum(vs) ||
      !isValidNum(V))
  {
   return "Метод не работает с заданными значениями параметров";
  }
  
  V = parseFloat(V);
  
  var DVSI = parseFloat(def)*parseFloat(vs)*(V/45.);
  
  return DVSI;
}

function layerRes(level1,level2,h1,h2,dvsi) //высоты со значениями критерия турбулентности равного или больше критического
{
  if(!isValidNum(level1)
    ||!isValidNum(level2)
    ||!isValidNum(h1)
    ||!isValidNum(h2)
    ||!isValidNum(dvsi) )
    {
      return "Метод не работает с заданными значениями параметров";
    }
    dataB = parseFloat(h2) - parseFloat(h1);
  if((dvsi >= 12.))
    {
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, dataB, "Ожидается турбулентность", level1,100);
      return "В слое "+level1+"-"+level2 +" ожидается турбулентность";
    }

    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 0, "Турбулентность не ожидается", level1,100);
    return "В слое "+level1+"-"+level2 +" турбулентность не ожидается";

}

  

