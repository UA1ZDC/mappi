#ifndef TEXPR_FUNC_H
#define TEXPR_FUNC_H

#include <mappi/ui/exprguid/exprdraw/Parent/BigParent/CommonFunc/texpr_commonfunc.h>

class QString;

class TExpr_Func : public TExpr_CommonFunc
{
public:
  TExpr_Func(const QString &_funcName, TExpr_Class* _daughter);
};

#endif // TEXPR_FUNC_H
