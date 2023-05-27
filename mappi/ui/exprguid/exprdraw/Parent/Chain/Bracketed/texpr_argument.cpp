#include "exprdraw/Parent/Chain/Bracketed/texpr_argument.h"

#include "exprdraw/Parent/BigParent/CommonFunc/texpr_commonfunc.h"

TExpr_Argument::TExpr_Argument(TExpr_Class *_son)
  : TExpr_Bracketed(_son, ExprDraw::ebRound, ExprDraw::ebRound)
{
  m_forcedBrackets = false;
}

bool TExpr_Argument::isBracketed()
{
  TExpr_CommonFunc* cmFunc = dynamic_cast<TExpr_CommonFunc*>(parent());

  if (cmFunc != 0){
    if (m_forcedBrackets || (cmFunc->ArgNeedBrackets()))
      return true;
  }

  TExpr_Class* P = son();
  while (P != 0) {
    if (P->NeedBrackets())
      return true;
    P = P->next();
  }

  return false;
}

void TExpr_Argument::SetBrackets()
{
  m_forcedBrackets = true;
  m_toChange = ExprDraw::CHANGED;
}
