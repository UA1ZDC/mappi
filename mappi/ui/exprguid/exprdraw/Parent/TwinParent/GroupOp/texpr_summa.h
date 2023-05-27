#ifndef TEXPR_SUMMA_H
#define TEXPR_SUMMA_H

#include <mappi/ui/exprguid/exprdraw/Parent/TwinParent/GroupOp/texpr_groupop.h>

class TExpr_Summa : public TExpr_GroupOp
{
public:
  TExpr_Summa(TExpr_Class* _son,
                       TExpr_Class* _firstTwin,
                       TExpr_Class* _secondTwin);
};

#endif // TEXPR_SUMMA_H
