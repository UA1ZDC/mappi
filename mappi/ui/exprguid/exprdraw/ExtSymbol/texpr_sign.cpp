#include "exprdraw/ExtSymbol/texpr_sign.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPolygonF>

using namespace ExprDraw;

TExpr_Sign::TExpr_Sign(int _SymbolCode)
  : TExpr_ExtSymbol(_SymbolCode)
{

}

int TExpr_Sign::CalcWidth()
{
  QFontMetrics fm(font());
  ushort code = m_symbol.unicode();
  int result;

  switch (code) {
  case esMuchLess:
  case esMuchGreater:
    result = qRound(1.7 * fm.width('<'));
    break;
  case esApproxLess:
  case esApproxGreater:
    result = fm.width('<');
    break;
  case esPlusMinus:
  case esMinusPlus:
    result = fm.width('+');
    break;
  case esAlmostEqual:
    result = fm.width('~');
    break;
  case esParallel:
    result = 4 * m_wx;
    break;
  case esPerpendicular:
  case esAngle:
    result = fm.width('_');
    // (a&1) -> true - нечетное число, false - четное число
    if ((result & 1) != (m_wx & 1))
      ++result;
    break;
  default:
    result = TExpr_ExtSymbol::CalcWidth();
    break;
  }

  return result + (4 * m_wx);
}

void TExpr_Sign::Paint(QPainter* painter, int X, int Y)
{
  ushort code = m_symbol.unicode();

  switch (code) {
  case esMuchLess:
    DrawMuchLess(painter, X, Y);
    break;
  case esMuchGreater:
    DrawMuchGreater(painter, X, Y);
    break;
  case esApproxLess:
    DrawApproxLess(painter, X, Y);
    break;
  case esApproxGreater:
    DrawApproxGreater(painter, X, Y);
    break;
  case esPlusMinus:
    DrawPlusMinus(painter, X, Y);
    break;
  case esMinusPlus:
    DrawMinusPlus(painter, X, Y);
    break;
  case esAlmostEqual:
    DrawAlmostEqual(painter, X, Y);
    break;
  case esParallel:
    DrawParallel(painter, X, Y);
    break;
  case esPerpendicular:
    DrawPerpendicular(painter, X,Y);
    break;
  case esAngle:
    DrawAngle(painter, X, Y);
    break;
  default:
    TExpr_ExtSymbol::Paint(painter, X+2*m_wx, Y);
    break;
  }
}

void TExpr_Sign::DrawMuchLess(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();
  QString ch('<');

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(X+2*m_wx, Y, ch);
  painter->drawText(X+2*m_wx+qRound(0.7*fm.width('<')), Y, ch);
  painter->restore();
}

void TExpr_Sign::DrawMuchGreater(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();
  QString ch('>');

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(X+2*m_wx, Y, ch);
  painter->drawText(X+2*m_wx+qRound(0.7*fm.width('<')), Y, ch);
  painter->restore();
}

void TExpr_Sign::DrawApproxLess(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(X+2*m_wx, Y, QString('<'));
  painter->drawText(X+2*m_wx, Y+qRound(7.0 * m_rwy), QString('~'));
  painter->restore();
}

void TExpr_Sign::DrawApproxGreater(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(X+2*m_wx, Y, QString('>'));
  painter->drawText(X+2*m_wx, Y+qRound(7.0 * m_rwy), QString('~'));
  painter->restore();
}

void TExpr_Sign::DrawPlusMinus(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(X+2*m_wx, Y-m_wy, QString(QChar(esPlus)));
  painter->drawText(X+2*m_wx, Y+qRound(7.0 * m_rwy), QString(QChar(esMinus)));
  painter->restore();
}

void TExpr_Sign::DrawMinusPlus(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(X+2*m_wx, Y-qRound(7.0 * m_rwy), QString(QChar(esMinus)));
  painter->drawText(X+2*m_wx, Y+m_wy, QString(QChar(esPlus)));
  painter->restore();
}

void TExpr_Sign::DrawAlmostEqual(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  Y = Y + fm.ascent();

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  painter->drawText(X+2*m_wx, Y-qRound(5.0 * m_rwy), QString('~'));
  painter->drawText(X+2*m_wx, Y+2*m_wy, QString(QChar(esMinus)));
  painter->restore();
}

void TExpr_Sign::DrawParallel(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  QPen pen = painter->pen();

  QRectF rec;
  rec.setLeft(X+2*m_wx);
  rec.setTop(Y+fm.descent());
  rec.setWidth(m_wx-pen.width());
  rec.setHeight(fm.ascent()-fm.descent());
  painter->drawRect(rec);

  rec.setLeft(X+5*m_wx);
  rec.setTop(Y+fm.descent());
  rec.setWidth(m_wx-pen.width());
  rec.setHeight(fm.ascent()-fm.descent());
  painter->drawRect(rec);
  painter->restore();
}

void TExpr_Sign::DrawPerpendicular(QPainter* painter, int X, int Y)
{
  QFontMetrics fm(font());
  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  QPen pen = painter->pen();

  int XL = X+2*m_wx;
  int XR = X+width()-2*m_wx;

  QRectF rec;
  rec.setLeft(XL);
  rec.setTop(Y+fm.ascent()+1-m_wy);
  rec.setRight(XR-pen.width());
  rec.setBottom(Y+fm.ascent()+1-pen.width());
  painter->drawRect(rec);

  int XC = X+(width()-m_wx) / 2;
  rec.setLeft(XC);
  rec.setTop(Y+fm.descent()+qRound(4.0*m_rwy));
  rec.setRight(XC+m_wx-pen.width());
  rec.setBottom(Y+fm.ascent()+1-pen.width());
  painter->drawRect(rec);
  painter->restore();
}

void TExpr_Sign::DrawAngle(QPainter* painter,int X, int Y)
{
  QFontMetrics fm(font());
  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  QPen pen = painter->pen();

  int XL = X+2*m_wx;
  int XR = X+width()-2*m_wx;

  QRectF rec;
  rec.setLeft(XL);
  rec.setTop(Y+fm.ascent()+1-m_wy);
  rec.setRight(XR-pen.width());
  rec.setBottom(Y+fm.ascent()+1-pen.width());
  painter->drawRect(rec);

  int XC = X+width()/2;
  QPolygonF poligon;
  poligon << QPointF(XL,rec.top());
  poligon << QPointF(XC, Y+fm.descent()+qRound(4.0*m_rwy)+1);
  poligon << QPointF(poligon.at(1).x(), poligon.at(1).y()+m_wy-1);
  poligon << QPointF(poligon.at(0).x(), poligon.at(0).y()+m_wy-1);
  painter->drawConvexPolygon(poligon);
  painter->restore();
}

int TExpr_Sign::FTType()
{
  ushort code = m_symbol.unicode();
  return efNegative * (code == esMinus || code == esPlusMinus ||
                       code == esMinusPlus || code == esPlusMinusU);
}

bool TExpr_Sign::NeedBrackets()
{
  ushort code = m_symbol.unicode();
  return (code == esMinus) || (code == esPlus) ||
      (code == esPlusMinus) || (code == esMinusPlus) ||
      (code == esPlusMinusU);
}

int TExpr_Sign::CalcCapDY()
{
  return INT_MAX;
}
