#include "exprdraw/Parent/Chain/texpr_chain.h"

TExpr_Chain::TExpr_Chain(TExpr_Class* _son)
  : TExpr_Parent(_son)
{

}

void TExpr_Chain::CalcOverAbove(int &Over, int &Above)
{
  Over = 0;
  Above = 0;
  TExpr_Class* P = son();
  while (P != 0) {
    Over = qMax(Over, P->midLineUp() + 1);
    Above = qMax(Above, P->height() - P->midLineUp() - 1);
    P = P->next();
  }
}

void TExpr_Chain::BuildUpChain(TExpr_Class *_value)
{
  if (hasSon()){
    TExpr_Class* P = son();
    while (P->hasNext()) {
      P = P->next();
    }

    P->SetNext(_value);
    _value->SetParent(this);
  }
  else{
    SetSon(_value);
  }

  m_toChange = ExprDraw::CHANGED;
}

int TExpr_Chain::CalcWidth()
{
  int result = 0;
  TExpr_Class* P = son();
  while (P != 0) {
    result += P->width();
    P = P->next();
  }
  return result;
}

int TExpr_Chain::CalcHeight()
{
  int Over, Above;
  CalcOverAbove(Over, Above);
  return Over + Above;
}

int TExpr_Chain::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  int Over, Above;
  CalcOverAbove(Over, Above);
  if (origin == ExprDraw::eoTop)
    return Over - 1;
  return - Above;
}

int TExpr_Chain::FTType()
{
  TExpr_Class* P = 0;
  if (hasSon()){
    P = son();
    while (P->hasNext()) {
      P = P->next();
    }
  }

  int fttSon = hasSon() ? son()->FTType() : TExpr_Class::FTType();
  int fttP = (P != 0) ? P->FTType() : TExpr_Class::FTType();
  return (fttSon & ExprDraw::efLeft) |
      (fttP & ExprDraw::efRight) |
      (fttSon & ExprDraw::efNegative);
}

int TExpr_Chain::CalcCapDY()
{
  int result = INT_MAX;
  TExpr_Class* P = son();
  while (P != 0) {
    int DY = P->capDY();
    if (DY < result)
      result = DY;
    P = P->next();
  }
  return result;
}

void TExpr_Chain::CalcCapDX(int &DLeft, int &DRight)
{
  DLeft = hasSon() ? son()->capDXLeft() : 0;

  TExpr_Class* P = 0;
  if (hasSon()){
    P = son();
    while (P->hasNext()) {
      P = P->next();
    }
  }

  DRight = (P != 0) ? P->capDXRight() : 0;
}

void TExpr_Chain::Paint(QPainter *painter, int X, int Y)
{
  Y += midLineUp();

  TExpr_Class* P = son();
  while (P != 0) {
    P->Draw(painter, X, Y, ExprDraw::ehLeft, ExprDraw::evCenter);
    X += P->width();
    P = P->next();
  }
}
