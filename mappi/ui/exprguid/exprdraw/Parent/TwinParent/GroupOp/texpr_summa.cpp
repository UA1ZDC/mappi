#include "exprdraw/Parent/TwinParent/GroupOp/texpr_summa.h"

TExpr_Summa::TExpr_Summa(TExpr_Class* _son,
                         TExpr_Class* _firstTwin,
                         TExpr_Class* _secondTwin)
  : TExpr_GroupOp(_son, _firstTwin, _secondTwin)
{
  m_ch = QChar(8721);
}

