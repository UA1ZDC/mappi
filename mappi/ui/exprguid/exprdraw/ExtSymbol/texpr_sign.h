#ifndef TEXPR_SIGN_H
#define TEXPR_SIGN_H

#include <mappi/ui/exprguid/exprdraw/ExtSymbol/texpr_extsymbol.h>

class QPainter;

class TExpr_Sign : public TExpr_ExtSymbol
{
private:
  void DrawMuchLess(QPainter* painter, int X, int Y);
  void DrawMuchGreater(QPainter *painter, int X, int Y);
  void DrawApproxLess(QPainter* painter, int X, int Y);
  void DrawApproxGreater(QPainter* painter, int X, int Y);
  void DrawPlusMinus(QPainter* painter, int X, int Y);
  void DrawMinusPlus(QPainter* painter, int X, int Y);
  void DrawAlmostEqual(QPainter* painter, int X, int Y);
  void DrawParallel(QPainter* painter, int X, int Y);
  void DrawPerpendicular(QPainter* painter, int X, int Y);
  void DrawAngle(QPainter* painter, int X, int Y);

public:
  TExpr_Sign(int _SymbolCode);
  int FTType();  
protected:
  bool NeedBrackets();
  void Paint(QPainter* painter, int X, int Y);
  int CalcCapDY();
  int CalcWidth();
};

#endif // TEXPR_SIGN_H
