#include "exprdraw/ExtSymbol/texpr_plank.h"

#include <QPainter>

TExpr_Plank::TExpr_Plank()
  : TExpr_ExtSymbol(295)
{

}

void TExpr_Plank::SetPainterFont(QPainter *painter)
{
  QFont fn = TExpr_ExtSymbol::font();
  fn.setItalic(true);
  painter->setFont(fn);
}

int TExpr_Plank::CalcCapDY()
{
  return qRound(4.0 * m_rwy);
}

void TExpr_Plank::CalcCapDX(int &DLeft, int &DRight)
{
  DLeft = qRound(m_rwx);
  DRight = DLeft;
}
