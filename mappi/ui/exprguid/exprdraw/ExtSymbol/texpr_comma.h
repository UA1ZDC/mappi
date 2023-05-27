#ifndef TEXPR_COMMA_H
#define TEXPR_COMMA_H

#include <mappi/ui/exprguid/exprdraw/ExtSymbol/texpr_extsymbol.h>

class TExpr_Comma : public TExpr_ExtSymbol
{
public:
  TExpr_Comma();
protected:
  bool NeedBrackets();
  int CalcCapDY();
};

#endif // TEXPR_COMMA_H
