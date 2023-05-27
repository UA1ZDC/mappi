#include "exprdraw/Parent/texpr_lim.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>

static const QString S_LIM("lim");

TExpr_Lim::TExpr_Lim(TExpr_Class* _son)
  :TExpr_Parent(_son)
{
  SetSonFont();
}

void TExpr_Lim::SetSonFont()
{
  if (hasSon()){
    QFont fn = font();
    fn.setPointSize(qRound(0.7 * fn.pointSize()));
    son()->SetFont(fn);
  }
}

void TExpr_Lim::SetSonPaintDevice()
{
  if (hasSon())
    son()->SetPaintDevice(m_paintDevice);
}

int TExpr_Lim::CalcWidth()
{
  QFontMetrics fm(font());
  int sW = hasSon() ? son()->width() : 0;
  return qMax(fm.width(S_LIM), sW);
}

int TExpr_Lim::CalcHeight()
{
  QFontMetrics fm(font());
  int sH = hasSon() ? son()->height() : 0;
  return fm.height() + sH;
}

int TExpr_Lim::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  QFontMetrics fm(font());
  int H = fm.height();

  if (origin == ExprDraw::eoTop)
    return H/2;
  int sH = hasSon() ? son()->height() : 0;
  return -((H - 1) / 2) - sH;
}

bool TExpr_Lim::ArgNeedBrackets()
{
  return false;
}

void TExpr_Lim::Paint(QPainter *painter, int X, int Y)
{
  QFontMetrics fm(font());
  int sW = hasSon() ? son()->width() : 0;
  int textW = fm.width(S_LIM);

  QPoint pos;
  pos.setX(X + qMax(0,(sW - textW) / 2));
  pos.setY(Y + fm.ascent());

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(pos, S_LIM);
  painter->restore();

  if (hasSon()){
    int textH = fm.height();
    son()->Draw(painter,
                X + qMax(0, (textW - sW) / 2),
                Y + textH,
                ExprDraw::ehLeft, ExprDraw::evTop);
  }
}
