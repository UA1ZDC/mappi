#ifndef TEXPR_INDEX_H
#define TEXPR_INDEX_H

#include <mappi/ui/exprguid/exprdraw/Parent/TwinParent/texpr_twinparent.h>

class TExpr_Index : public TExpr_TwinParent
{
public:
  TExpr_Index(TExpr_Class* _son,
                       TExpr_Class* _firstTwin,
                       TExpr_Class* _secondTwin);
  bool ArgNeedBrackets();
  int FTType();
protected:
  int CalcCapDY();
  void Paint(QPainter *painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
};

#endif // TEXPR_INDEX_H
