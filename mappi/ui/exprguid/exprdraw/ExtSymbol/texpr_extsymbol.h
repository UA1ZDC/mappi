#ifndef TEXPR_EXTSYMBOL_H
#define TEXPR_EXTSYMBOL_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>

class TExpr_ExtSymbol : public TExpr_Class
{
public:
  TExpr_ExtSymbol(int _SymbolCode);
protected:
  QChar m_symbol;
  void Paint(QPainter* painter, int x, int y);
  int CalcCapDY();
  void CalcCapDX(int& DLeft, int& DRight);
  int CalcPowerXPos();
  int CalcWidth();
  int CalcHeight();
public:

};

#endif // TEXPR_EXTSYMBOL_H
