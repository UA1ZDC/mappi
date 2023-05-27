#include "exprdraw/Parent/Chain/Bracketed/texpr_round.h"

TExpr_Round::TExpr_Round(TExpr_Class *_son)
  : TExpr_Bracketed(_son, ExprDraw::ebRound, ExprDraw::ebRound)
{

}

int TExpr_Round::FTType()
{
  return ExprDraw::efLeft | ExprDraw::efRight | ExprDraw::efBrackets;
}
