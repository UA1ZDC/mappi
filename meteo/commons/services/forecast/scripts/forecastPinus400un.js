function badvalue()
{
  return -9999;
}
function nodata()
{
  return "Нет данных";
}
function emptyvalue()
{
  return " - ";
}

function Pinus400undTmax()
{
    zondObj.resetData();
    //максимальная разность температур кривой состояния и стратификации
    var res = zondObj.oprPsostRaznAllFastDelta(obj.getStationData());

    if ( !isValidNum(res) ) { 
        return "Не удалось определить максимальную разность температур автоматически";
    }
    return Math.round(res*10.)/10.;
}
function Pinus400unP()
{
    //уровень ГПа
  var res = zondObj.oprPsostRaznAllFastP(obj.getStationData());
    if ( !isValidNum(res) ) { 
        return "Не удалось определить уровень автоматически";
    }
    var fin = Math.round(res);
    return fin;
    
    if(fin<=950. && fin >= 900.)
    {
        return 925;
    }
    if(fin<900. && fin >=800.)
    {
        return 850.;
    }
    if(fin < 800. && fin >= 650.)
    {
        return 700.;
    }
    if(fin < 650. && fin >= 550.)
    {
        return 600.;
    }
   
    return 500.;
}

function Pinus400unDescr(dTmax,P)
{
//    if (dTmax == badvalue()) { return badvalue(); }

    if (!isValidNum(dTmax)||
        !isValidNum(P))
    { 
        return "Метод не работает с заданными значениями параметров";
    }


    var ret;
    if(dTmax<=0.)
    {
        ret = "Без болтанки";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 8, ret, P,100);
        return ret;
    }
    if(dTmax>0.&&dTmax<=3.)
    {
        ret  = "Слабая болтанка";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 9, ret, P,100);
        return ret;
    }
    if(dTmax>3.&&dTmax<=6.)
    {
        ret = "Умеренная болтанка";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 10, ret, P,100);
        return ret;
    }
    if(dTmax>6.)
    {
        ret = "Сильная болтанка";
        dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11030, 11, ret, P,100);
        return ret;
    }
}

