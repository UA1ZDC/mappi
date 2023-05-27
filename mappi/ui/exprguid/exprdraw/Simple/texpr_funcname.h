#ifndef TEXPR_FUNCNAME_H
#define TEXPR_FUNCNAME_H

#include <mappi/ui/exprguid/exprdraw/Simple/texpr_simple.h>

class TExpr_FuncName : public TExpr_Simple
{
public:
  TExpr_FuncName(const QString& expr);
protected:
  bool ArgNeedBrackets();
};

#endif // TEXPR_FUNCNAME_H
