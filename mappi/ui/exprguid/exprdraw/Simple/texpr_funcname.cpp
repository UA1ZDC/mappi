#include "texpr_funcname.h"

TExpr_FuncName::TExpr_FuncName(const QString &expr)
  : TExpr_Simple(expr)
{

}

bool TExpr_FuncName::ArgNeedBrackets()
{
  return false;
}
