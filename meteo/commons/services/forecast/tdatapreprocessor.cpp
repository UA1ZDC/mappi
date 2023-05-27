#include "tdatapreprocessor.h"

#include <qscriptvalue.h>

enum SunType {
  Sunrise    = 0,
  Sunset     = 1,
};



QDateTime TDataPreprocessor::getSunRise(int year, int month, int day, double d_fi0, double d_la0, int i_N, const QScriptValue& typeSun)
{
  QDateTime res(QDate(year, month, day));

  QTime zah, vosh;
  int ret_val = getSunRise(res.date(), d_fi0, d_la0, i_N, zah, vosh);


  switch (static_cast<SunType>(typeSun.toInt32())) {
  case Sunrise:
    if (ret_val != -1) {
      res.setTime(vosh);
    }
    else {
      res.setDate(QDate(0, 0, 0));
    }
    break;
  case Sunset:
    if (ret_val != -2) {
      res.setTime(zah);
    }
    else {
      res.setDate(QDate(0, 0, 0));
    }
    break;
  }

  return res;
}



