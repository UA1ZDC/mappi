#include "exprdraw/Parent/TwinParent/GroupOp/texpr_prod.h"

TExpr_Prod::TExpr_Prod(TExpr_Class* _son,
                       TExpr_Class* _firstTwin,
                       TExpr_Class* _secondTwin)
  : TExpr_GroupOp(_son, _firstTwin, _secondTwin)
{
  m_ch = QChar(8719);
}
