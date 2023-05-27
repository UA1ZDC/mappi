#include "exprdraw/texpr_strokes.h"

#include <QFontMetrics>
#include <QPainter>
//#include <QPolygonF>

#include <QRect>

const ushort STROKE = 0xB4;

TExpr_Strokes::TExpr_Strokes(uint _countStrokes)
  :TExpr_Class()
{
  m_countStrokes = _countStrokes;
}

int TExpr_Strokes::CalcWidth()
{
  return m_wx * (4*m_countStrokes +2);
}

int TExpr_Strokes::CalcHeight()
{
  QFontMetrics fm(font());
  return qRound(0.6*fm.height());
}

void TExpr_Strokes::Paint(QPainter *painter, int X, int Y)
{
  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);

  // Рисуем сами
//    QPolygonF poligon;
//    poligon << QPointF(X + 2*m_wx, Y + qRound(2*m_rwy));
//    poligon << QPointF(X + 4*m_wx - 1, poligon.at(0).y());
//    poligon << QPointF(X + 2*m_wx, poligon.at(0).y() + 6*m_wy - 1);

//    for (uint i = 0; i < m_countStrokes; ++i) {
//      painter->drawConvexPolygon(poligon);
//      poligon[0].setX(poligon.at(0).x() + 4*m_wx);
//      poligon[1].setX(poligon.at(1).x() + 4*m_wx);
//      poligon[2].setX(poligon.at(2).x() + 4*m_wx);
//    }

  // Рисуем символ из шрифта

  // Увеличим шрифт на 40%, т.к. TExpr_Strokes используется в индексах
  // без увеличения будет слишком мелко
  QFont fn = font();
  fn.setPointSizeF(fn.pointSizeF() + (fn.pointSizeF() * 0.4));
  painter->setFont(fn);
  QFontMetrics fm(fn);

 // QFontMetrics fm(font());
  QRect rect = fm.boundingRect(STROKE);

  QPoint pos;
  pos.setX(X - rect.left());
  pos.setY(Y + fm.ascent() - (fm.ascent() + rect.top()) + qRound(2*m_rwy));

  QString ch = QString(STROKE);

  for (uint i = 0; i < m_countStrokes; ++i) {
    painter->drawText(pos.x() + 2*m_wx, pos.y(), ch);
    pos.setX(pos.x() + 4*m_wx);
  }

  painter->restore();
}
