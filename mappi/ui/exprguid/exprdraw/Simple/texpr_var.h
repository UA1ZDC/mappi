#ifndef TEXPR_VAR_H
#define TEXPR_VAR_H

#include <mappi/ui/exprguid/exprdraw/Simple/texpr_simple.h>

class TExpr_Var :public TExpr_Simple
{
public:
  TExpr_Var(const QString& expr);
  TExpr_Var(const char _ch);
  void CalcCapDX(int& DLeft, int& DRight);
protected:
  QFont GetFont();
  int CalcPowerXPos();
  int CalcIndexXPos();
  void SetPainterFont(QPainter* painter);
};

#endif // TEXPR_VAR_H
