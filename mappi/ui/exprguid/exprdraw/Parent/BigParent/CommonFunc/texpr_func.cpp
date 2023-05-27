#include "exprdraw/Parent/BigParent/CommonFunc/texpr_func.h"
#include "exprdraw/Simple/texpr_var.h"
#include "exprdraw/Simple/texpr_funcname.h"

#include <QString>

TExpr_Func::TExpr_Func(const QString &_funcName, TExpr_Class *_daughter)
  : TExpr_CommonFunc(0, _daughter)
{
  if (_funcName.length() == 1)
    SetSon(new TExpr_Var(_funcName));
  else
    SetSon(new TExpr_FuncName(_funcName));
}
