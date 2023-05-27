#ifndef TEXPR_LAMBDA_H
#define TEXPR_LAMBDA_H

#include <mappi/ui/exprguid/exprdraw/ExtSymbol/texpr_extsymbol.h>

class TExpr_Lambda : public TExpr_ExtSymbol
{
public:
  TExpr_Lambda();
protected:
  void Paint(QPainter* painter, int X, int Y);
};

#endif // TEXPR_LAMBDA_H
