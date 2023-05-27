#ifndef TEXPR_STAND_H
#define TEXPR_STAND_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>

class TExpr_Stand : public TExpr_Parent
{
public:
  TExpr_Stand(TExpr_Class* _son, ExprDraw::TExprHorAlign _align);
protected:
  void Paint(QPainter *painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
private:
  ExprDraw::TExprHorAlign m_hAlig;
};

#endif // TEXPR_STAND_H
