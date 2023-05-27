#include "exprdraw/Simple/texpr_simple.h"

#include <QString>
#include <QFontMetrics>
#include <QPainter>


TExpr_Simple::TExpr_Simple(const QString &expr)
  :TExpr_Class()
{
  m_text = expr;
}

TExpr_Simple::TExpr_Simple(const char _ch)
  :TExpr_Class()
{
  m_text.append(_ch);
}

int TExpr_Simple::CalcWidth()
{
  QFontMetrics fm(font());
  int result = fm.width(m_text);

  if (!m_text.isEmpty()){
    int lBearing = fm.leftBearing(m_text[0]);
    if (lBearing < 0)
      result = result - lBearing;

    int rBearing = fm.rightBearing(m_text[m_text.length()-1]);
    if (rBearing < 0)
      result = result - rBearing;
  }
  return result;
}

int TExpr_Simple::CalcHeight()
{
  QFontMetrics fm(font());
  return fm.height();
}

void TExpr_Simple::Paint(QPainter *painter, int x, int y)
{
  if (m_text.isEmpty())
    return;

  QFontMetrics fm(font());
  QPoint pos(x, y + fm.ascent());
  int lBearing = fm.leftBearing(m_text[0]);

  if (lBearing < 0)
    pos.setX(pos.x() - lBearing);

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(pos, m_text);

  painter->restore();
}

int TExpr_Simple::CalcCapDY()
{
  // Переделать.
  // подбирай DY в зависимости от высоты символа,
  // а не в зависимости от символа.
  double DY;
  ushort code = m_text.isEmpty() ? 0 : m_text.at(0).unicode();

  switch (code) {
  case 'a': case 'c': case 'e': case 'g':
    DY = 9.5;
    break;
  case 'b': case 'd': case 'f': case 'h': case 'k': case 'l':
    DY = 4.0;
    break;
    //                                  й           ё
  case 'i': case 'j': case 't': case 0x439: case 0x451:
    DY = 7.0;
    break;
    //  Ё          Й
  case 0x401: case 0x419:
    DY = 2.5;
    break;
  default:
    DY = 0.0;
    break;
  }

  if (DY == 0.0){
    if (((code >= 'A') && (code <= 'Z')) ||
        ((code >= 0x410) && (code <= 0x418)) ||   // А - И
        ((code >= 0x41A) && (code <= 0x42F)))     // К - Я
      DY = 4.0;
    else
      if (((code >= 'm') && (code <= 's')) ||
          ((code >= 'u') && (code <= 'z')) ||
          ((code >= 0x430) && (code <= 0x438)) || // а - и
          ((code >= 0x43A) && (code <= 0x44F)))   // к - я
        DY = 9.5;
  }

  return qRound(DY*m_rwy);
}
