#ifndef TEXPR_CHAIN_H
#define TEXPR_CHAIN_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>

class TExpr_Chain : public TExpr_Parent
{
public:
  TExpr_Chain(TExpr_Class* _son);
  void BuildUpChain(TExpr_Class* _value);
  int FTType();
protected:
  void Paint(QPainter *painter, int X, int Y);
  int CalcCapDY();
  void CalcCapDX(int &DLeft, int &DRight);
  int CalcMidLine(ExprDraw::TExprOrigin origin);
  int CalcWidth();
  int CalcHeight();
private:
  void CalcOverAbove(int& Over, int& Above);
};

#endif // TEXPR_CHAIN_H
