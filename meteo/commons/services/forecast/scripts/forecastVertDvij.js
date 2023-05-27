function SkorVertDvig925(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W925, Lapla_0sredni;
  Lapla0        = parseFloat(Lapla0);
  LaplaAdv      = parseFloat(LaplaAdv);
  Lapla_0sredni = lapSredni(Lapla0,LaplaAdv);
  W925=(-3.5*(parseFloat(Lapla_0sredni/**2.5e11*/)))/**0.85*/;
//  W925=(-3.5*(parseFloat(Lapla_0sredni/**2.5e11*/)+parseFloat(Lapla0)-parseFloat(LaplaAdv)))*0.85;
  if (!isValidNum(W925))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11005, W925.toFixed(2), W925.toFixed(2), 925,100);


  return W925;
}

function SkorVertDvig850(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W850, Lapla_0sredni;
  Lapla0        = parseFloat(Lapla0);
  LaplaAdv      = parseFloat(LaplaAdv);
  Lapla_0sredni = lapSredni(Lapla0,LaplaAdv);
  W850          = -3.5*(parseFloat(Lapla_0sredni)/**2.5e11*/+parseFloat(Lapla0-LaplaAdv));
//  W850=-3.5*(parseFloat(Lapla_0sredni)/**2.5e11*/+parseFloat(Lapla0)-parseFloat(LaplaAdv));
  if (!isValidNum(W850))
  {
    return badvalue();
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11005, W850.toFixed(2), W850.toFixed(2), 850,100);

  return W850;
}

function SkorVertDvig700(Lapla700,Lapla850,Lapla0,LaplaAdv,Lapla700adv,Lapla850adv)
{
  if (!isValidNum(Lapla700)||
      !isValidNum(Lapla850)||
      !isValidNum(Lapla0)  ||
      !isValidNum(LaplaAdv) ||
      !isValidNum(Lapla700adv)||
      !isValidNum(Lapla850adv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W700, W850;
  Lapla700    = parseFloat(Lapla700);
  Lapla850    = parseFloat(Lapla850);
  Lapla0      = parseFloat(Lapla0);
  LaplaAdv    = parseFloat(LaplaAdv);
  Lapla700adv = parseFloat(Lapla700adv);
  Lapla850adv = parseFloat(Lapla850adv);
  W850        = SkorVertDvig850(parseFloat(Lapla0),parseFloat(LaplaAdv));
  if( W850    == badvalue())
  {
      return "Метод не работает с заданными значениями параметров";
  }

  W700= W850-2.1*(parseFloat(Lapla850-Lapla850adv)+parseFloat(Lapla700-Lapla700adv));

//  W700= parseFloat(W850)-2.1*(parseFloat(Lapla850)-parseFloat(Lapla850adv)+parseFloat(Lapla700)-parseFloat(Lapla700adv));
  if (!isValidNum(W700))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 11005, W700.toFixed(2), W700.toFixed(2),700,100);

  return W700;
}

function SkorVertDvig500(Lapla700,Lapla500,Lapla850,Lapla0,LaplaAdv,Lapla700adv,Lapla850adv,Lapla500adv)
{
  if (!isValidNum(Lapla850)||
      !isValidNum(Lapla700)||
      !isValidNum(Lapla0)||
      !isValidNum(LaplaAdv)||
      !isValidNum(Lapla500) ||
      !isValidNum(Lapla700adv) ||
      !isValidNum(Lapla850adv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var W500, W700;
  Lapla850    = parseFloat(Lapla850);
  Lapla700    = parseFloat(Lapla700);
  Lapla0      = parseFloat(Lapla0);
  LaplaAdv    = parseFloat(LaplaAdv);
  Lapla500    = parseFloat(Lapla500);
  Lapla700adv = parseFloat(Lapla700adv);
  Lapla850adv = parseFloat(Lapla850adv);
  W700        = SkorVertDvig700(Lapla700,Lapla850,Lapla0,LaplaAdv,Lapla700adv,Lapla850adv);
  if (!isValidNum(W700))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  W500= W700-2.8*(parseFloat(Lapla700-Lapla700adv)+parseFloat(Lapla500-Lapla500adv));

//  W500= W700-2.8*(parseFloat(Lapla700)-parseFloat(Lapla700adv)+parseFloat(Lapla500)-parseFloat(Lapla500adv));
  if (!isValidNum(W500))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(),11005, W500.toFixed(2), W500.toFixed(2),500,100);

  return W500;
}


//среднее арифметическое между значением лапласиана давления в пункте расчета
//в конечный момент 12-часового интервала и значением лапласиана давления в
//начальной точке траектории в начальный момент интервала;
function lapSredni(Lapla0,LaplaAdv)
{
  if (!isValidNum(Lapla0)||
      !isValidNum(LaplaAdv))
  {
    return "Метод не работает с заданными значениями параметров";
  }
  var sred;
  sred = (parseFloat(Lapla0)+parseFloat(LaplaAdv))/2.;
  return sred;
}
