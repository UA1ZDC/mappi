#ifndef TEXPR_ROOT_H
#define TEXPR_ROOT_H

#include <mappi/ui/exprguid/exprdraw/Parent/BigParent/texpr_bigparent.h>

class TExpr_Root : public TExpr_BigParent
{
public:
  TExpr_Root(TExpr_Class* _son, TExpr_Class* _daughter);
protected:
  void Paint(QPainter* painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
private:
  void SetDaughterFont();
//  void SetDaughterPaintDevice();
};

#endif // TEXPR_ROOT_H
