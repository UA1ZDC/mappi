#include "exprdraw/ExtSymbol/texpr_extsymbol.h"

#include <QFontMetrics>
#include <QPainter>


TExpr_ExtSymbol::TExpr_ExtSymbol(int _SymbolCode)
  :TExpr_Class()
{
  m_symbol = QChar(_SymbolCode);
}

int TExpr_ExtSymbol::CalcWidth()
{
  QFontMetrics fm(font());
  int result = fm.width(m_symbol);

  int lBearing = fm.leftBearing(m_symbol);
  if (lBearing < 0)
    result = result - lBearing;

  int rBearing = fm.rightBearing(m_symbol);
  if (rBearing < 0)
    result = result - rBearing;
  return result;
}

int TExpr_ExtSymbol::CalcHeight()
{
  QFontMetrics fm(font());
  return fm.height();
}

void TExpr_ExtSymbol::Paint(QPainter *painter, int x, int y)
{
  QFontMetrics fm(font());
  QPoint pos(x, y + fm.ascent());

  int lBearing = fm.leftBearing(m_symbol);
  if (lBearing < 0)
    pos.setX(pos.x() - lBearing);

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(pos, m_symbol);
  painter->restore();
}

int TExpr_ExtSymbol::CalcCapDY()
{
  double DY;
  ushort code = m_symbol.unicode();

  switch (code) {
  case 0x3B1:
  case 0x3B3:
  case 0x3B5:
  case 0x3B7:
  case 0x3B9:
  case 0x3BA:
  case 0x3BC:
  case 0x3BD:
  case 0x3BF:
  case 0x3C0:
  case 0x3C1:
    DY = 8.8;
    break;

  case 0x3B2:
  case 0x3B4:
  case 0x3B6:
  case 0x3B8:
  case 0x3BB:
  case 0x3BE:
    DY = 4.0;
    break;

  case ExprDraw::esEllipsis:
    DY = INT_MAX / m_rwy - 1;
    break;
  default:
    DY = 0.0;
    break;
  }

  if (DY == 0.0){
    if ((code >= 0x3C3) && (code <= 0x3C9))
      DY = 8.8;
    else
      if (((code >= 0x391) && (code <= 0x3A1)) ||
          ((code >= 0x3A3) && (code <= 0x3A9)))
          DY = 4.0;
  }
  return qRound(DY * m_rwy);
}

void TExpr_ExtSymbol::CalcCapDX(int &DLeft, int &DRight)
{
  double DX = 0.0;
  ushort code = m_symbol.unicode();

  if (((code >= 0x393) && (code <= 0x3A1)) ||
      ((code >= 0x3A3) && (code <= 0x3A9)) ||
      (code == 0x391) || (code == 0x3B8))
    DX = 1.0;
  DLeft = qRound(DX * m_rwx);

  switch (code) {
  case 0x3B5:
    DX = 1.0;
    break;
  case 0x3B8:
    DX = -0.5;
    break;
  default:
    DX = 0.0;
    break;
  }
  if (DX == 0.0){
    if (((code >= 0x391) && (code <= 0x3A1)) ||
        ((code >= 0x3A3) && (code <= 0x3A9)))
      DX = -1.0;
  }
  DRight = qRound(DX * m_rwx);
}

int TExpr_ExtSymbol::CalcPowerXPos()
{
  int DX = 0;
  ushort code = m_symbol.unicode();

  if (code == ExprDraw::esPartDiff)
    DX = 2;
  return TExpr_Class::CalcPowerXPos() + qRound(DX * m_rwx);
}
