#ifndef TEXPR_ARGUMENT_H
#define TEXPR_ARGUMENT_H

#include <mappi/ui/exprguid/exprdraw/Parent/Chain/Bracketed/texpr_bracketed.h>

class TExpr_Argument : public TExpr_Bracketed
{
public:
  TExpr_Argument(TExpr_Class* _son);
  void SetBrackets();
protected:
  bool m_forcedBrackets;
  bool isBracketed();
};

#endif // TEXPR_ARGUMENT_H
