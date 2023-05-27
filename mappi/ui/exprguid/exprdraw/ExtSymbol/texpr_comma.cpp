#include "exprdraw/ExtSymbol/texpr_comma.h"

TExpr_Comma::TExpr_Comma()
  : TExpr_ExtSymbol(44)
{

}

bool TExpr_Comma::NeedBrackets()
{
  return true;
}

int TExpr_Comma::CalcCapDY()
{
  return INT_MAX;
}
