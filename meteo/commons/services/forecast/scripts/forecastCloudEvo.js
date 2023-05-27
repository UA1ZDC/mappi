function cloudSt(height,T,Ta,D,Da,time) //адвекция 12 или 18 часовая
{
    if( !isValidNum(T)  ||
       !isValidNum(Ta)  ||
       !isValidNum(D) ||
       !isValidNum(Da) ||
       !isValidNum(height) )
    {
      return "Метод не работает с заданными значениями параметров";
    }
  var ret_val,result;
  var hour;
  if(time == 12){
      hour = "0;12";
  } else{
    if(time == 18) {
      hour = "0;18";
    } else {
      return "Срок должен быть 12 или 18 часов";
    }
  }


  //int ret_val=0,F;
  ret_val = pic80(Ta-T,Da-D);
//  return ret_val;
  switch(ret_val)
  {
    case 1:
        result= "Облачность не образуется (размывается)";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 1, result, height,100);
        break;
    case 2:
        result= "Ситуация не определена";
//        dataObj.savePrognoz(obj.getStationData(),name(),hour, 20003, 0, result, height,100);
        break;
    case 3:
        result= "Облачность  образуется (не размывается)";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hour, 20003, 3, result, height,100);
        break;
    case 4:  result = "Не удалось произвести расчет: адвективное изменение T или D лежит вне диапазона (-10;10)"; break;
//    default: result= "-";
  }

  return result;
}


