#include "exprdraw/ExtSymbol/texpr_nabla.h"
#include <QPainter>

TExpr_Nabla::TExpr_Nabla()
  : TExpr_ExtSymbol(916)
{

}

void TExpr_Nabla::Paint(QPainter *painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);

  painter->translate(X+width(), Y-fm.boundingRect(QChar(m_symbol)).height()+1);
  painter->rotate(180.0);
  painter->drawText(0, 0, m_symbol);

  painter->restore();
}
