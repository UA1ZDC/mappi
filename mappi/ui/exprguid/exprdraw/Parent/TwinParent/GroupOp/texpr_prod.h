#ifndef TEXPR_PROD_H
#define TEXPR_PROD_H

#include <mappi/ui/exprguid/exprdraw/Parent/TwinParent/GroupOp/texpr_groupop.h>

class TExpr_Prod : public TExpr_GroupOp
{
public:
  TExpr_Prod(TExpr_Class* _son,
             TExpr_Class* _firstTwin,
             TExpr_Class* _secondTwin);
};

#endif // TEXPR_PROD_H
