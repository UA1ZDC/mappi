#ifndef TEXPR_RATIO_H
#define TEXPR_RATIO_H

#include <mappi/ui/exprguid/exprdraw/Parent/BigParent/texpr_bigparent.h>

class TExpr_Ratio : public TExpr_BigParent
{
public:
  TExpr_Ratio(TExpr_Class* _son, TExpr_Class* _daughter);
protected:
  void Paint(QPainter* painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
};

#endif // TEXPR_RATIO_H
