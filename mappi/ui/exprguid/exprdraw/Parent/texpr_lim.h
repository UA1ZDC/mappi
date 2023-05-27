#ifndef TEXPR_LIM_H
#define TEXPR_LIM_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>

class TExpr_Lim : public TExpr_Parent
{
public:
  TExpr_Lim(TExpr_Class* _son);
protected:
  void SetSonFont();
  void SetSonPaintDevice();
  void Paint(QPainter* painter, int X, int Y);
  bool ArgNeedBrackets();
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
};

#endif // TEXPR_LIM_H
