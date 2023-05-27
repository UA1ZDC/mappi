#ifndef TEXPR_PLANK_H
#define TEXPR_PLANK_H

#include <mappi/ui/exprguid/exprdraw/ExtSymbol/texpr_extsymbol.h>

class QPainter;

class TExpr_Plank : public TExpr_ExtSymbol
{
public:
  TExpr_Plank();
protected:
  int CalcCapDY();
  void CalcCapDX(int& DLeft, int& DRight);
  void SetPainterFont(QPainter* painter);
};

#endif // TEXPR_PLANK_H
