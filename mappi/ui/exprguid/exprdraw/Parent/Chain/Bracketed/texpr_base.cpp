#include "exprdraw/Parent/Chain/Bracketed/texpr_base.h"

TExpr_Base::TExpr_Base(TExpr_Class *_son)
  : TExpr_Bracketed(_son, ExprDraw::ebRound, ExprDraw::ebRound)
{

}

bool TExpr_Base::isBracketed()
{
  if (hasSon()){
    return son()->hasNext();
  }

  return false;
}
