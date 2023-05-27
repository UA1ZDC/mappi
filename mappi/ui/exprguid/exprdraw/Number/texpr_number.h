#ifndef TEXPR_NUMBER_H
#define TEXPR_NUMBER_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>

#include <qlocale.h>

class QString;

class TExpr_Number : public TExpr_Class
{
public:
  TExpr_Number(double _number, bool _exponentForm);
  int FTType();
  double number() {return m_number;}
  void setNumber(double _number);

private:
  QString m_sMantis;
  QString m_sExponent;
  bool m_exponentForm;
  QFont smallFont();
protected:
  double m_number;
  QLocale m_local;
  virtual QString numberToString();
  int CalcCapDY();
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);

  void Paint(QPainter* painter, int x, int y);
};

#endif // TEXPR_NUMBER_H

