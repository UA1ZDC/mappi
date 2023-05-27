#ifndef TEXPR_INTEGRAL_H
#define TEXPR_INTEGRAL_H

#include <mappi/ui/exprguid/exprdraw/Parent/TwinParent/GroupOp/texpr_groupop.h>

class TExpr_Integral : public TExpr_GroupOp
{
public:
  TExpr_Integral(TExpr_Class* _son,
                          TExpr_Class* _firstTwin,
                          TExpr_Class* _secondTwin,
                          int _mult = 1);
protected:
  int CalcSymbolWidth();
  void DrawSymbol(QPainter* painter, int X, int Y);
private:
  QString m_integral;
};

#endif // TEXPR_INTEGRAL_H
