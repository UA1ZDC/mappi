//R1 – количество осадков в пункте оценивания за последние 12 часов, мм;
//R2 – количество осадков за предыдущие 12 час, мм;
//R3 – количество осадков за предшествующие вторые сутки, мм;
//R4 – количество осадков за предшествующие 3-5е сутки, мм;
//R5 – количество осадков за предшествующие 6-10е сутки, мм;
//R6 –  количество осадков за предшествующие 11-20е сутки, мм;
//R7 – количество осадков за предшествующие 21-30е сутки, мм;
//R8 – количество осадков за предшествующие 31-40е сутки, мм;
//R9 – количество осадков за предшествующие 41-50е сутки, мм;
//R10 – количество осадков за предшествующие 51-60е сутки, мм.

function R4(r1,r2,r3)
{
  if( !isValidNum( r1 )   ||
      !isValidNum( r2 )     ||
      !isValidNum( r3 )   )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  return parseFloat(r1)+parseFloat(r2)+parseFloat(r3);

}


function R5(r1,r2,r3,r4,r5)
{

  if( !isValidNum( r1 )   ||
      !isValidNum( r2 )     ||
      !isValidNum( r3 )  ||
      !isValidNum( r4 )     ||
      !isValidNum( r5 ))
  {
    return "Метод не работает с заданными значениями параметров";
  }

  return parseFloat(r1)+parseFloat(r2)+parseFloat(r3)+parseFloat(r4)+parseFloat(r5);

}

function Rbig(r1,r2,r3,r4,r5,r6,r7,r8,r9,r10)
{
    if( !isValidNum( r1 )   ||
        !isValidNum( r2 )     ||
        !isValidNum( r3 )  ||
        !isValidNum( r4 )     ||
        !isValidNum( r5 )  ||
        !isValidNum( r6 )   ||
        !isValidNum( r7 )     ||
        !isValidNum( r8 )  ||
        !isValidNum( r9 )     ||
        !isValidNum( r10 ))
    {
      return "Метод не работает с заданными значениями параметров";
    }


    return parseFloat(r1)+parseFloat(r2)+parseFloat(r3)+parseFloat(r4)+parseFloat(r5)+parseFloat(r6)+parseFloat(r7)+parseFloat(r8)+parseFloat(r9)+parseFloat(r10);



}




function uvlag(val,tip,weight,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10)
{
  if( !isValidNum( R1 )   ||
      !isValidNum( R2 )     ||
      !isValidNum( R3 )  ||
      !isValidNum( R4 )   ||
      !isValidNum( R5 )     ||
      !isValidNum( R6 )  ||
      !isValidNum( R7 )   ||
      !isValidNum( R8 )     ||
      !isValidNum( R9 )  ||
      !isValidNum( R10 ) )
  {
    return "Метод не работает с заданными значениями параметров";
  }

  if( R1 == 0.    &&
      R2 == 0.    &&
      R3 == 0.    &&
      R4 == 0.    &&
      R5 == 0.    &&
      R6 == 0.    &&
      R7 == 0.    &&
      R8 == 0.    &&
      R9 == 0.    &&
      R10 == 0.    )
  {
    return "Метод не работает с заданными значениями параметров";
  }



  var I;
  var WWt;
  var stUvlag;
  var stGrunt;
  var dataB;
  var dataWrd;


  I = 0.85*R1 + 0.75*R2 + 0.6*R3 + 0.5*R4 + 0.4*R5 + 0.3*R6 + 0.17*R7 + 0.09*R8 + 0.05*R9 + 0.03*R10;

  //////////////////////////////Супесь

  if( tip == "Супесь")
  {
      if( weight == "Легкая")
      {
          if( I < 7.)
          {
             WWt = "<0.5";
             stUvlag = "Очень сухой, сухой";
             stGrunt = "Твердый";
             dataB = 0;
             dataWrd = "Поверхность почвы сухая";
          }
          if( I >= 7. && I < 15.)
          {
              WWt = "0.5-0.8";
              stUvlag = "Слабо увлажненный, влажный";
              stGrunt = "Твердо-пластичный";
              dataB = 1;
              dataWrd = "Поверхность почвы сырая";
          }
          if( I >= 15. && I < 21.)
          {
              WWt = "0.8-1.0";
              stUvlag = "Очень влажный";
              stGrunt = "Мягко-пластичный";
              dataB = 2;
              dataWrd = "Поверхность почвы влажная";
          }
          if( I >= 21.)
          {
              WWt =  "≥1.0"
              stUvlag = "Насыщен влагой";
              stGrunt = "Текучий";
              dataB = 3;
              dataWrd = "Затопленная";
          }

      }
      if( weight == "Тяжелая")
      {
          if( I < 20.)
          {
              WWt = "<0.5";
              stUvlag = "Очень сухой, сухой";
              stGrunt = "Твердый";
              dataB = 0;
              dataWrd = "Поверхность почвы сухая";
          }
          if( I >= 20. && I < 41.)
          {
              WWt = "0.5-0.8";
              stUvlag = "Слабо увлажненный, влажный";
              stGrunt = "Твердо-пластичный";
              dataB = 1;
              dataWrd = "Поверхность почвы сырая";
          }
          if( I >= 41. && I < 52.)
          {
              WWt = "0.8-1.0";
              stUvlag = "Очень влажный";
              stGrunt = "Мягко-пластичный";
              dataB = 2;
              dataWrd = "Поверхность почвы влажная";

          }
          if( I >= 52.)
          {
              WWt =  "≥1.0"
              stUvlag = "Насыщен влагой";
              stGrunt = "Текучий";
              dataB = 3;
              dataWrd = "Затопленная";

          }

      }
  }
  //////////////////////////////Суглинок
  if( tip == "Суглинок")
  {
      if( weight == "Легкая")
      {
          if( I < 12.)
          {
              WWt = "<0.5";
              stUvlag = "Очень сухой, сухой";
              stGrunt = "Твердый";
              dataB = 0;
              dataWrd = "Поверхность почвы сухая";
          }
          if( I >= 12. && I < 27.)
          {
              WWt = "0.5-0.8";
              stUvlag = "Слабо увлажненный, влажный";
              stGrunt = "Твердо-пластичный";
              dataB = 1;
              dataWrd = "Поверхность почвы сырая";
          }
          if( I >= 27. && I < 37.)
          {
              WWt = "0.8-1.0";
              stUvlag = "Очень влажный";
              stGrunt = "Мягко-пластичный";
              dataB = 2;
              dataWrd = "Поверхность почвы влажная";
          }
          if( I >= 37.)
          {
              WWt =  "≥1.0"
              stUvlag = "насыщен влагой";
              stGrunt = "текучий";
              dataB = 3;
              dataWrd = "Затопленная";
          }

      }
      if( weight == "Тяжелая")
      {
          if( I < 35.)
          {
              WWt = "<0.5";
              stUvlag = "Очень сухой, сухой";
              stGrunt = "Твердый";
              dataB = 0;
              dataWrd = "Поверхность почвы сухая";
          }
          if( I >= 35. && I < 71.)
          {
              WWt = "0.5-0.8";
              stUvlag = "Слабо увлажненный, влажный";
              stGrunt = "Твердо-пластичный";
              dataB = 1;
              dataWrd = "Поверхность почвы сырая";
          }
          if( I >= 71. && I < 90.)
          {
              WWt = "0.8-1.0";
              stUvlag = "Очень влажный";
              stGrunt = "Мягко-пластичный";
              dataB = 2;
              dataWrd = "Поверхность почвы влажная";
          }
          if( I >= 90.)
          {
              WWt =  "≥1.0"
              stUvlag = "Насыщен влагой";
              stGrunt = "Текучий";
              dataB = 3;
              dataWrd = "Затопленная";
          }

      }
  }
  //////////////////////////////Глина
  if( tip == "Глина")
  {
      if( weight == "Легкая")
      {
          if( I < 21.)
          {
              WWt = "<0.5";
              stUvlag = "Очень сухой, сухой";
              stGrunt = "Твердый";
              dataB = 0;
              dataWrd = "Поверхность почвы сухая";

          }
          if( I >=21. && I < 44.)
          {
              WWt = "0.5-0.8";
              stUvlag = "Слабо увлажненный, влажный";
              stGrunt = "Твердо-пластичный";
              dataB = 1;
              dataWrd = "Поверхность почвы сырая";

          }
          if( I >= 44. && I < 61.)
          {
              WWt = "0.8-1.0";
              stUvlag = "Очень влажный";
              stGrunt = "Мягко-пластичный";
              dataB = 2;
              dataWrd = "Поверхность почвы влажная";

          }
          if( I >= 61.)
          {
              WWt =  "≥1.0"
              stUvlag = "Насыщен влагой";
              stGrunt = "Текучий";
              dataB = 3;
              dataWrd = "Затопленная";

          }

      }
      if( weight == "Тяжелая")
      {
          if( I < 55.)
          {
              WWt = "<0.5";
              stUvlag = "Очень сухой, сухой";
              stGrunt = "Твердый";
              dataB = 0;
              dataWrd = "Поверхность почвы сухая";

          }
          if( I >= 55. && I < 112.)
          {
              WWt = "0.5-0.8";
              stUvlag = "Слабо увлажненный, влажный";
              stGrunt = "Твердо-пластичный";
              dataB = 1;
              dataWrd = "Поверхность почвы сырая";

          }
          if( I >= 112. && I < 141.)
          {
              WWt = "0.8-1.0";
              stUvlag = "Очень влажный";
              stGrunt = "Мягко-пластичный";
              dataB = 2;
              dataWrd = "Поверхность почвы влажная";

          }
          if( I >= 141.)
          {
              WWt =  "≥1.0"
              stUvlag = "Насыщен влагой";
              stGrunt = "Текучий";
              dataB = 3;
              dataWrd = "Затопленная";

          }
      }
  }



  if( val == 1)
  {
    dataObj.savePrognoz(obj.getStationData(),name(),title(),hours(), 20062, dataB, dataWrd, 0,1);
    return WWt;
  }
  if( val == 2)
  {
    return stUvlag;

  }
  if( val == 3)
  {
    return stGrunt;

  }
  //"20062"


}







