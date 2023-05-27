#include "exprdraw/Parent/texpr_stand.h"

TExpr_Stand::TExpr_Stand(TExpr_Class *_son, ExprDraw::TExprHorAlign _align)
  : TExpr_Parent(_son)
{
  m_hAlig = _align;
}

int TExpr_Stand::CalcWidth()
{
  if (!hasSon())
    return 0;

  int W = son()->width();
  TExpr_Class* P = son();

  while (P->hasNext()) {
    P = P->next();
    W = qMax(W, P->width());
  }
  return W;
}

int TExpr_Stand::CalcHeight()
{
  if (!hasSon())
    return 0;

  int H = son()->height();
  TExpr_Class* P = son();

  while (P->hasNext()) {
    P = P->next();
    H +=  P->height();
  }
  return H;
}

void TExpr_Stand::Paint(QPainter *painter, int X, int Y)
{
  if (!hasSon())
    return ;

  int W = width();
  TExpr_Class* P = son();

  while (P != 0) {
    switch (m_hAlig) {
    case ExprDraw::ehLeft:
      P->Draw(painter, X, Y, m_hAlig, ExprDraw::evTop);
      break;
    case ExprDraw::ehCenter:
      P->Draw(painter, X + (W/2), Y, m_hAlig, ExprDraw::evTop);
      break;
    case ExprDraw::ehRight:
      P->Draw(painter, X + W, Y, m_hAlig, ExprDraw::evTop);
      break;
    default:
      break;
    }
    Y += P->height();
    P = P->next();
  }
}
