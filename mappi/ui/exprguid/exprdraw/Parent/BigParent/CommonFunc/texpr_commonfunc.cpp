#include "exprdraw/Parent/BigParent/CommonFunc/texpr_commonfunc.h"


TExpr_CommonFunc::TExpr_CommonFunc(TExpr_Class* _son, TExpr_Class* _daughter)
  : TExpr_BigParent(_son, _daughter)
{

}

int TExpr_CommonFunc::CalcWidth()
{
  int sonW = hasSon() ? son()->width() : 0;
  int daughterW = hasDaughter() ? daughter()->width() : 0;
  return sonW + 3*m_wx + daughterW;
}

int TExpr_CommonFunc::CalcHeight()
{
  return midLineUp() - midLineDn() + 1;
}

int TExpr_CommonFunc::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (origin == ExprDraw::eoTop){
    int sU = hasSon() ? son()->midLineUp() : 0;
    int dU = hasDaughter() ? daughter()->midLineUp() : 0;
    return qMax(sU, dU);
  }
  int sD = hasSon() ? son()->midLineDn() : 0;
  int dD = hasDaughter() ? daughter()->midLineDn() : 0;
  return qMin(sD, dD);}

bool TExpr_CommonFunc::ArgNeedBrackets()
{
  if (hasSon())
    return son()->ArgNeedBrackets();
  return TExpr_BigParent::ArgNeedBrackets();
}

int TExpr_CommonFunc::FTType()
{
  int dFTT = hasDaughter() ? daughter()->FTType() : 0;
  int D = (dFTT & ExprDraw::efBrackets) > 0;
  return ExprDraw::efLeft + ExprDraw::efRight * D;
}

void TExpr_CommonFunc::Paint(QPainter *painter, int X, int Y)
{
  int DY = Y + midLineUp();
  int sW = 0;
  if (hasSon()){
    DY -= son()->midLineUp();
    son()->Draw(painter, X, DY, ExprDraw::ehLeft, ExprDraw::evTop);
    sW = son()->width();
  }

  if (hasDaughter()){
    int DX = X + 3*m_wx + sW;
    int DY = Y + midLineUp() - daughter()->midLineUp();
    daughter()->Draw(painter, DX, DY, ExprDraw::ehLeft, ExprDraw::evTop);
  }
}
