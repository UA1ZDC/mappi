#ifndef TEXPR_COMMONFUNC_H
#define TEXPR_COMMONFUNC_H

#include <mappi/ui/exprguid/exprdraw/Parent/BigParent/texpr_bigparent.h>

class TExpr_CommonFunc: public TExpr_BigParent
{
public:
  TExpr_CommonFunc(TExpr_Class* _son, TExpr_Class* _daughter);
  int FTType();
  bool ArgNeedBrackets();
protected:
  void Paint(QPainter* painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
};

#endif // TEXPR_COMMONFUNC_H
