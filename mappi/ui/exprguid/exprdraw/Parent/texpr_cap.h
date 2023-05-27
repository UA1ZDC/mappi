#ifndef TEXPR_CAP_H
#define TEXPR_CAP_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>

class TExpr_Cap : public TExpr_Parent
{
public:
  TExpr_Cap(TExpr_Class* _son, ExprDraw::TExprCapStyle _capStyle,
            int _count = 1);
  int FTType();
protected:
  int CapWidth();
  int CapHeight();
  int SelfHeight();
  int CalcPowerXPos();
  int CalcPowerYPos();
  int CalcIndexXPos();
  int CalcCapDY();
  void Paint(QPainter* painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
private:
  ExprDraw::TExprCapStyle m_capStyle;
  int m_count;
};

#endif // TEXPR_CAP_H
