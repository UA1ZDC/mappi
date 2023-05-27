#ifndef TEXPR_ROUND_H
#define TEXPR_ROUND_H

#include <mappi/ui/exprguid/exprdraw/Parent/Chain/Bracketed/texpr_bracketed.h>

class TExpr_Round : public TExpr_Bracketed
{
public:
  TExpr_Round(TExpr_Class* _son);
  int FTType();
};

#endif // TEXPR_ROUND_H
