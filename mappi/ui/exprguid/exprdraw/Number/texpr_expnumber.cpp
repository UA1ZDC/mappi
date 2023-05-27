#include "exprdraw/Number/texpr_expnumber.h"

#include <cmath>
#include <QLocale>

TExpr_ExpNumber::TExpr_ExpNumber(double _number, int _precision,
                                 int _digits, int _maxDeg)
  :TExpr_Number(0.0, false)
{
  //m_precision = _precision;
  m_precision = _precision-1;
  if (m_precision <= 0)
    m_precision = 1;
  m_digits = _digits;
  m_maxDeg = _maxDeg;

  setNumber(_number);
}

QString TExpr_ExpNumber::numberToString()
{
  if ((m_number != 0.0) && (log10(fabs(m_number)) <= (-m_maxDeg)))
    return m_local.toString(m_number,'E',m_precision);
    //return QString::number(m_number,'E',m_precision);
  else{
    // Обрезать знаки после запятой, используя значение m_digits
  QString result = m_local.toString(m_number,'E',m_precision);
  result = m_local.toString(m_local.toDouble(result), 'f',m_digits);
//    QString result = m_local.toString(m_number,'f',m_precision);
//    QChar decimalPoint = m_local.decimalPoint();
//    int P = result.lastIndexOf(decimalPoint);

//    if (P != -1)
//      result = result.left(P+m_digits+1);

//    //Если '.' была последней добавим '0'
//    if (result[result.length()-1] == decimalPoint)
//      result += '0';

    return result;
  }

}
