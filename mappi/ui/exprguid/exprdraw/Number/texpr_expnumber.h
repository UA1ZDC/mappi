#ifndef TEXPR_EXPNUMBER_H
#define TEXPR_EXPNUMBER_H

#include <mappi/ui/exprguid/exprdraw/Number/texpr_number.h>

class TExpr_ExpNumber : public TExpr_Number
{
public:
  // _number - число для вывода
  // _precision - точность числа
  // _digits - количество цифр после запятой
  // _maxDeg - (смысл в примере)

  //  maxDeg = 2
  //  number > 0.01xxx
  //    на выходе 0.01xxx
  //  number <= 0.01xxx
  //    на выходе 10^x
  TExpr_ExpNumber(double _number, int _precision = 4,
                  int _digits = 4, int _maxDeg = 2);
private:
  int m_precision;
  int m_digits;
  int m_maxDeg;
protected:
  virtual QString numberToString();
};

#endif // TEXPR_EXPNUMBER_H
