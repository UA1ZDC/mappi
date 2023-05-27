#include "exprdraw/Parent/TwinParent/GroupOp/texpr_groupop.h"

#include "exprdraw/Parent/Chain/texpr_chain.h"

#include <QFontMetrics>
#include <QPainter>

TExpr_GroupOp::TExpr_GroupOp(TExpr_Class* _son,
                             TExpr_Class* _firstTwin,
                             TExpr_Class* _secondTwin)
  : TExpr_TwinParent(_son, _firstTwin, _secondTwin)
{
  m_symbolHeight = 0;
  m_symbolWidth = 0;
}

int TExpr_GroupOp::CalcSymbolHeight()
{
  TExpr_GroupOp* G;

  TExpr_Chain* S = dynamic_cast<TExpr_Chain*>(son());

  if ((S != 0) &&
      ((S->FTType() & ExprDraw::efRoundBrackets) == 0)){
    TExpr_Class* P = S->son();

    while (P != 0) {
      G = dynamic_cast<TExpr_GroupOp*>(P);
      if (G != 0){
        return G->GetSymbolHeight();
      }

      P = P->next();
    }
  }

  G = dynamic_cast<TExpr_GroupOp*>(son());
  if (G != 0)
    return G->GetSymbolHeight();

  int H = 0;
  if (hasSon())
    H = qMax(son()->midLineUp(), -son()->midLineDn());

  QFontMetrics fm(font());
  return qRound(2.25 * qMax(H, fm.height() / 2));
}

int TExpr_GroupOp::GetSymbolWidth()
{
  if ((m_toChange & ExprDraw::tcSymbolWidth) > 0){
    m_symbolWidth = CalcSymbolWidth();
    m_toChange = m_toChange & ~ExprDraw::tcSymbolWidth;
  }
  return m_symbolWidth;
}

int TExpr_GroupOp::GetSymbolHeight()
{
  if ((m_toChange & ExprDraw::tcSymbolHeight) > 0){
    m_symbolHeight = CalcSymbolHeight();
    m_toChange = m_toChange & ~ExprDraw::tcSymbolHeight;
  }
  return m_symbolHeight;
}

int TExpr_GroupOp::CalcWidth()
{
  int W1 = hasFirstTwin() ? firstTwin()->width() : 0;
  int W2 = hasSecondTwin() ? secondTwin()->width() : 0;
  int sW = hasSon() ? son()->width() : 0;
  W1 = qMax(W1, W2);
  W1 = qMax(W1, GetSymbolWidth());

  return W1 + sW + 5*m_wx;
}

int TExpr_GroupOp::CalcHeight()
{
  int sH1 = 0;
  int sH2 = 0;

  TExpr_GroupOp* G = dynamic_cast<TExpr_GroupOp*>(son());
  if (G != 0){
    if (G->hasFirstTwin())
      sH1 = G->firstTwin()->height();
    if (G->hasSecondTwin())
      sH2 = G->secondTwin()->height();
  }

  int H1 = hasFirstTwin() ? firstTwin()->height() : 2*m_wy;
  int H2 = hasSecondTwin() ? secondTwin()->hasNext() : 2*m_wy;

  return GetSymbolHeight() + qMax(H1, sH1) + qMax(H2, sH2);
}

int TExpr_GroupOp::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (origin == ExprDraw::eoTop){
    int sH = 0;

    TExpr_GroupOp* G = dynamic_cast<TExpr_GroupOp*>(son());
    if ((G != 0) && (G->hasSecondTwin()))
      sH = G->secondTwin()->height();

   int H = hasSecondTwin() ? secondTwin()->height() : 2*m_wy;
   return (GetSymbolHeight()/2) + qMax(H, sH);
  }

  int sH = 0;

  TExpr_GroupOp* G = dynamic_cast<TExpr_GroupOp*>(son());
  if ((G != 0) && (G->hasFirstTwin()))
    sH = G->firstTwin()->height();

 int H = hasFirstTwin() ? firstTwin()->height() : 2*m_wy;
 int S = (GetSymbolHeight()-1)/2;
 return -(S + qMax(H, sH));
}

void TExpr_GroupOp::Paint(QPainter *painter, int X, int Y)
{
  int W1 = hasFirstTwin() ? firstTwin()->width() : 0;
  int W2 = hasSecondTwin() ? secondTwin()->width() : 0;

  W1 = qMax(W1, W2);
  W1 = qMax(W1, GetSymbolWidth());
  W2 = X + W1/2 + 2*m_wx;

  int H = midLineUp();
  int HS = GetSymbolHeight()/2;

  if (hasSecondTwin())
    secondTwin()->Draw(painter,
                       W2, Y + H - HS,
                       ExprDraw::ehCenter, ExprDraw::evBottom);

  if (hasFirstTwin())
    firstTwin()->Draw(painter,
                      W2, Y + H + HS,
                      ExprDraw::ehCenter, ExprDraw::evTop);

  DrawSymbol(painter, W2, Y + H);

  if (hasSon())
    son()->Draw(painter,
                X + W1 + 5*m_wx, Y + H,
                ExprDraw::ehLeft, ExprDraw::evCenter);
}

int TExpr_GroupOp::CalcSymbolWidth()
{
  QFontMetrics fm(m_symbol_font);
  return fm.width(m_ch)+4*m_wx;
}

void TExpr_GroupOp::DrawSymbol(QPainter *painter, int X, int Y)
{
  QFontMetrics fm(m_symbol_font);

  //Выставляем символ посередине. В разных шрифтах символы находятся на разной
  //высоте от базовой линии, нам надо чтобы символ всегда был посередине
  QPoint pos(X, Y + fm.ascent());
  QRect rec = fm.boundingRect(m_ch);
  //чаще всего rec.top() отрицательное число, поэтому мы его прибавляем
  int dY = fm.ascent() + rec.top() + rec.height()/2;
  int dX = fm.width(m_ch)/2;

  painter->save();
  SetPenAndBrush(painter);
  painter->setFont(m_symbol_font);

  painter->translate(pos.x() - dX, pos.y() - dY);
  painter->drawText(0,0, QString(m_ch));
  painter->restore();
}

void TExpr_GroupOp::DynaSetFont()
{
  TExpr_TwinParent::DynaSetFont();
  m_symbol_font = font();

  //Увеличиваем шрифт для символа.
  QFontMetrics fm(m_symbol_font);
  int H = GetSymbolHeight();
  m_symbol_font.setPointSize(m_symbol_font.pointSize() * H/fm.height());
}
