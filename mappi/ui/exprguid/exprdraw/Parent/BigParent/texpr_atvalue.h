#ifndef TEXPR_ATVALUE_H
#define TEXPR_ATVALUE_H

#include <mappi/ui/exprguid/exprdraw/Parent/BigParent/texpr_bigparent.h>

class TExpr_AtValue : public TExpr_BigParent
{
public:
  TExpr_AtValue(TExpr_Class* _son, TExpr_Class* _daughter);
  int FTType();
protected:
  void Paint(QPainter* painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
private:
  void SetDaughterFont();
//  void SetDaughterPaintDevice();
};

#endif // TEXPR_ATVALUE_H
