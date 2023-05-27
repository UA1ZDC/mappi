#include "exprdraw/ExtSymbol/texpr_lambda.h"
#include <QPainter>

TExpr_Lambda::TExpr_Lambda()
  :TExpr_ExtSymbol(955)
{

}

void TExpr_Lambda::Paint(QPainter *painter, int X, int Y)
{
  TExpr_ExtSymbol::Paint(painter, X, Y);

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);

  int DY = Y+qRound(9.0*m_rwy);
  QPen pen = painter->pen();

  QRectF rec;
  rec.setLeft(X+qRound(1.5*m_rwx));
  rec.setTop(DY);
  rec.setRight(X+width()-qRound(2.0*m_rwx)-pen.width());
  rec.setBottom(DY+m_wy-pen.width());
  painter->drawRect(rec);

  painter->restore();
}
