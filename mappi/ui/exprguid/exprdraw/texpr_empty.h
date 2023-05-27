#ifndef TEXPR_EMPTY_H
#define TEXPR_EMPTY_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>

class TExpr_Empty : public TExpr_Class
{
public:
  TExpr_Empty();
protected:
  int CalcHeight();
};

#endif // TEXPR_EMPTY_H
