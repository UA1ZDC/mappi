#include "exprdraw/Parent/Chain/Bracketed/texpr_bracketed.h"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>

TExpr_Bracketed::TExpr_Bracketed(TExpr_Class *_son, ExprDraw::TExprBracket LeftBracket, ExprDraw::TExprBracket RightBracket)
  : TExpr_Chain(_son)
{
  m_leftBracket = LeftBracket;
  m_rightBracket = RightBracket;

  switch (m_leftBracket) {
  case ExprDraw::ebRound:
    m_leftCh = QChar('(');
    break;
  case ExprDraw::ebSquare:
    m_leftCh = QChar('[');
    break;
  case ExprDraw::ebFigure:
    m_leftCh = QChar('{');
    break;
  case ExprDraw::ebModule:
    m_leftCh = QChar('|');
    break;
  default:
    m_leftCh = QChar('(');  // не имеет значения, лиш бы не пробел
    break;
  }

  switch (m_rightBracket) {
  case ExprDraw::ebRound:
    m_rightCh = QChar(')');
    break;
  case ExprDraw::ebSquare:
    m_rightCh = QChar(']');
    break;
  case ExprDraw::ebFigure:
    m_rightCh = QChar('}');
    break;
  case ExprDraw::ebModule:
    m_rightCh = QChar('|');
    break;
  default:
    m_rightCh = QChar(')'); // не имеет значения, лиш бы не пробел
    break;
  }
}

bool TExpr_Bracketed::isBracketed()
{
  return true;
}

int TExpr_Bracketed::FTType()
{
  if (isBracketed() &&
      (m_leftBracket != ExprDraw::ebNone) &&
      (m_rightBracket != ExprDraw::ebNone)){

    int result = ExprDraw::efLeft |
        ExprDraw::efRight | ExprDraw::efBrackets;

    if ((m_leftBracket == ExprDraw::ebRound) &&
        (m_rightBracket == ExprDraw::ebRound))
      result = result | ExprDraw::efRoundBrackets;

    return result;
  }
  else
    return TExpr_Chain::FTType();
}

int TExpr_Bracketed::CalcWidth()
{
  int result = TExpr_Chain::CalcWidth();

  if (isBracketed()){
    if (m_leftBracket != ExprDraw::ebNone){
      QFontMetrics fm(getBracketFont(m_leftCh));
      result += fm.width(m_leftCh);
    }

    if (m_rightBracket != ExprDraw::ebNone){
      QFontMetrics fm(getBracketFont(m_rightCh));
      result += fm.width(m_rightCh);
    }
  }

  return result;
}

int TExpr_Bracketed::CalcHeight()
{
  int result = TExpr_Chain::CalcHeight();
  //result += 2*m_wy;

  return result;
}

int TExpr_Bracketed::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (origin == ExprDraw::eoTop)
    return TExpr_Chain::CalcMidLine(ExprDraw::eoTop) + m_wy;
  else{
    int result = TExpr_Chain::CalcMidLine(ExprDraw::eoBottom) - m_wy;
    if (isBracketed() &
        ((m_leftBracket == ExprDraw::ebFigure) | (m_rightBracket == ExprDraw::ebFigure)) &
        ((result - m_wy) & 1))
      --result;
    return result;
  }
}

int TExpr_Bracketed::CalcCapDY()
{
  if (isBracketed() &
      ((m_leftBracket != ExprDraw::ebNone) | (m_rightBracket != ExprDraw::ebNone)))
    return 0;
  return TExpr_Chain::CalcCapDY();
}

void TExpr_Bracketed::CalcCapDX(int &DLeft, int &DRight)
{
  if (isBracketed() &
      ((m_leftBracket != ExprDraw::ebNone) | (m_rightBracket != ExprDraw::ebNone))){
    DLeft = 0;
    DRight = 0;
  }
  else
    TExpr_Chain::CalcCapDX(DLeft, DRight);
}

void TExpr_Bracketed::removeBrackets()
{
  m_leftBracket = ExprDraw::ebNone;
  m_rightBracket = ExprDraw::ebNone;
//  m_leftCh = QChar('(');  // не имеет значения, лиш бы не пробел
//  m_rightCh = QChar(')');
  m_toChange = m_toChange | ExprDraw::tcWidth | ExprDraw::tcHeight |
      ExprDraw::tcCapDX | ExprDraw::tcCapDY;
}

void TExpr_Bracketed::Paint(QPainter* painter, int X, int Y)
{  
  // если скобки не нужны, или они не установленны
  // рисуем только то что в скобках
  if ((!isBracketed()) ||
      ((m_leftBracket == ExprDraw::ebNone) &&
       (m_rightBracket == ExprDraw::ebNone))){
    TExpr_Chain::Paint(painter, X, Y);
    return;
  }

  // иначе рисуем левую скобку (если она установленна),
  // затем то что в скобках,
  // и затем правую скобку (если она установленна)
  int DX = 0;
  if (m_leftBracket != ExprDraw::ebNone){
    DrawBracket(painter, X, Y, DX, brLeft);
  }
  TExpr_Chain::Paint(painter, X + DX, Y);

  if (m_rightBracket != ExprDraw::ebNone){
    int W = TExpr_Chain::CalcWidth();
    DrawBracket(painter, X + DX + W, Y, DX, brRigth);
  }
}

void TExpr_Bracketed::DrawBracket(QPainter* painter, int X, int Y, int &dX, Bracket _bracket)
{
  painter->save();
  SetPenAndBrush(painter);

  QChar ch;
  switch (_bracket) {
  case brLeft:
    ch = m_leftCh;
    break;
  case brRigth:
    ch = m_rightCh;
    break;
  }

  QFont fn = getBracketFont(ch);

  QFontMetrics fm(fn);
  dX = fm.width(ch);

  //QPoint pos(X, Y + fm.ascent() + midLineUp());
  QPoint pos(X, Y + fm.ascent() + height()/2);
  QRect rec = fm.boundingRect(ch);
  int dY = fm.ascent() + rec.top() + rec.height()/2; //Растаяние от верха скобки до её середины

  painter->setFont(fn);
  painter->translate(pos.x(), pos.y() - dY);
  painter->drawText(0,0, QString(ch));
  painter->restore();
}

QFont TExpr_Bracketed::getBracketFont(const QChar &bracketChar)
{
  QFont result = font();
  QFontMetrics fm(result);
  result.setPointSize(result.pointSize() * height()/fm.boundingRect(bracketChar).height());
  return result;
}
