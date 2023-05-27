#include "exprdraw/Parent/BigParent/texpr_ratio.h"

#include <QPainter>

TExpr_Ratio::TExpr_Ratio(TExpr_Class *_son, TExpr_Class *_daughter)
  : TExpr_BigParent(_son, _daughter)
{

}

int TExpr_Ratio::CalcWidth()
{
  int sW = hasSon() ? son()->width() : 0;
  int dW = hasDaughter() ? daughter()->width() : 0;
  return 8*m_wx + qMax(sW, dW);
}

int TExpr_Ratio::CalcHeight()
{
  int sH = hasSon() ? son()->height() : 0;
  int dH = hasDaughter() ? daughter()->height() : 0;
  return 3*m_wy + sH + dH;
}

int TExpr_Ratio::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (origin == ExprDraw::eoTop){
    int sH = hasSon() ? son()->height() : 0;
    return sH + m_wy + (m_wy /2);
  }

  int dH = hasDaughter() ? daughter()->height() : 0;
  return dH - m_wy - (m_wy / 2);
}

void TExpr_Ratio::Paint(QPainter *painter, int X, int Y)
{
  int YC = midLineUp();
  int XC = width() / 2;

  if (hasSon())
    son()->Draw(painter,
                X + XC,
                Y + YC - m_wy - (m_wy / 2),
                ExprDraw::ehCenter, ExprDraw::evBottom);

  if (hasDaughter())
    daughter()->Draw(painter,
                     X + XC,
                     Y + YC + m_wy + (m_wy / 2),
                     ExprDraw::ehCenter, ExprDraw::evTop);

  painter->save();
  SetPenAndBrush(painter);
  QPen pen = painter->pen();
  QRectF rec;
  rec.setLeft(X + 3*m_wx);
  rec.setTop(Y + YC - (m_wy / 2));
  rec.setRight(X+width()-3*m_wx+1-pen.width());
  rec.setBottom(Y + YC + (m_wy / 2) + (m_wy & 1) - pen.width());
  painter->drawRect(rec);

  painter->restore();
}
