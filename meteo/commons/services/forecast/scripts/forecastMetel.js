function metelForc(state, prOsadk, prVeter, prT)
{
  var res;
  if( !isValidNum( prOsadk )     ||
      !isValidNum( prT )     ||
      !isValidNum( prVeter )  )
  {
    return badvalue();
  }
  if(prOsadk < 0.)
  {
    return "Метель не ожидается";
  }

  prT = parseFloat(prT);

  if(prT > 0.)
  {
    return "Метель не ожидается";
  }
  var dataB;


  if( state == "Рыхлый снег")
  {
    if( prVeter < 5.0)
    {
       res = "Метель не ожидается";
       dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, dataB, res, 0,1);
       return res;
    }else
    if( prVeter >= 5.0 && prVeter <= 9.9)
    {
      if(prOsadk > 0. && prOsadk <= 0.5)
      {
         res = "Слабая низовая метель";
         dataB = 36;
      }else
      if( prOsadk > 0.5)
      {
         res = "Умеренная общая метель";
         dataB = 38;
      }else
      if( prOsadk == 0. || prOsadk == badvalue())
      {
         res = "Слабая низовая метель";
         dataB = 36;
      }

    }else
    if( prVeter >= 10.0 && prVeter <= 12.4)
    {
      if( prOsadk > 0. && prOsadk <= 0.5)
      {
        res =  "Умеренная низовая метель";
        dataB = 36;
      }else
      if( prOsadk > 0.5)
      {
        res =  "Умеренная общая метель";
        dataB = 38;
      }else
      if( prOsadk == 0. || prOsadk == badvalue())
      {
        res =  "Умеренная низовая метель";
        dataB = 36;
      }
    }else
    if( prVeter >= 12.5 && prVeter <= 15.0)
    {
      if( prOsadk > 0. && prOsadk <= 0.5)
      {
        res =  "Сильная низовая метель";
        dataB = 37;
      }else
      if( prOsadk > 0.5)
      {
        res =  "Сильная общая метель";
        dataB = 39;
      }else
      if( prOsadk === 0. || prOsadk === badvalue())
      {
        res =  "Сильная низовая метель";
        dataB = 37;
      }
    }else
    if( prVeter > 15.0)
    {
      if( prOsadk > 0. && prOsadk <= 0.5)
      {
        res =  "Сильная низовая метель";
        dataB = 37;
      }else
      if( prOsadk > 0.5)
      {
        res =  "Очень сильная общая метель";
        dataB = 39;
      }else
      if( prOsadk == 0. || prOsadk == badvalue())
      {
        res =  "Сильная низовая метель";
        dataB = 37;
      }
    }
  }else
  /////////////////////////////
  if( state == "Плотный снег")
  {
    if( prVeter < 5.0)
    {
      res = "Метель не ожидается";
      dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, dataB, res, 0,1);
      return res;
    }else

    if( prOsadk > 0.5)
    {
      if( prVeter >= 5.0 && prVeter <= 12.4)
      {
         res =  "Умеренная общая метель";
         dataB = 38;
      }else
      if( prVeter >= 12.5 && prVeter <= 15.0)
      {
         res =  "Сильная общая метель";
         dataB = 39;
      }else
      if( prVeter > 15.0)
      {
        res =  "Очень сильная общая метель";
        dataB = 39;
      }

    }else
    if( prOsadk === 0. || prOsadk === badvalue())
    {
      dataB = 0;
      res =  "Метель не ожидается";
    }

//    dataB = 0;
//    res =  "Метели нет";
  }

  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20003, dataB, res, 0,1);
  return res;

}


