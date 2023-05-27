#ifndef TEXPR_NABLA_H
#define TEXPR_NABLA_H

#include <mappi/ui/exprguid/exprdraw/ExtSymbol/texpr_extsymbol.h>


class TExpr_Nabla : public TExpr_ExtSymbol
{
public:
  TExpr_Nabla();
protected:
  void Paint(QPainter* painter, int X, int Y);
};

#endif // TEXPR_NABLA_H
