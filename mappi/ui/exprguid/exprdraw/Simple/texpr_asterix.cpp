#include "exprdraw/Simple/texpr_asterix.h"

#include <QFontMetrics>
#include <QPainter>

TExpr_Asterix::TExpr_Asterix()
  : TExpr_Simple('*')
{

}

void TExpr_Asterix::Paint(QPainter *painter, int X, int Y)
{
  QFontMetrics fm(font());
  QPoint pos(X, Y + qRound(8*m_rwy) + fm.ascent());

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(pos, m_text);

  painter->restore();
}
