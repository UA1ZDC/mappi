#ifndef TEXPR_CIRC_H
#define TEXPR_CIRC_H

#include <mappi/ui/exprguid/exprdraw/Parent/TwinParent/GroupOp/texpr_groupop.h>

class TExpr_Circ : public TExpr_GroupOp
{
public:
  TExpr_Circ(TExpr_Class* _son,
                      TExpr_Class* _firstTwin,
                      TExpr_Class* _secondTwin);
protected:
  void DrawSymbol(QPainter* painter, int X, int Y);
};

#endif // TEXPR_CIRC_H
