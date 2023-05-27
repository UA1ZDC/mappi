#ifndef TEXPR_BASE_H
#define TEXPR_BASE_H

#include <mappi/ui/exprguid/exprdraw/Parent/Chain/Bracketed/texpr_bracketed.h>

class TExpr_Base : public TExpr_Bracketed
{
public:
  TExpr_Base(TExpr_Class* _son);
protected:
  bool isBracketed();
};

#endif // TEXPR_BASE_H
