#include "exprdraw/Parent/BigParent/texpr_root.h"

#include <QPainter>
#include <QPolygonF>
#include <cmath>


TExpr_Root::TExpr_Root(TExpr_Class *_son, TExpr_Class *_daughter)
  : TExpr_BigParent(_son, _daughter)
{
  SetDaughterFont();
}

int TExpr_Root::CalcWidth()
{
  int result = 8*m_wx;
  if (hasSon())
    result += son()->width() + qRound((son()->height()+m_wy)/2.0);
  if (hasDaughter())
    result += qMax(0, daughter()->width() - 5*m_wx);
  return result;
}

int TExpr_Root::CalcHeight()
{
  int result = 3*m_wy;
  if (hasSon())
    result += son()->height();
  if (hasDaughter())
    result += qMax(0, daughter()->height() - 4*m_wx);
  return result;
}

int TExpr_Root::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (!hasSon())
    return TExpr_BigParent::CalcMidLine(origin);

  if (origin == ExprDraw::eoBottom)
    return son()->midLineDn();

  if ((hasDaughter()) && (daughter()->height() > 4*m_wy))
    return son()->midLineUp() - m_wy + daughter()->height();
  return son()->midLineUp() + 3*m_wy;
}

void TExpr_Root::SetDaughterFont()
{
  if (hasDaughter()){
    QFont fn = font();
    fn.setPointSize(qRound(0.7 * fn.pointSize()));
    daughter()->SetFont(fn);
  }
}

//void TExpr_Root::SetDaughterPaintDevice()
//{
//  if (hasDaughter())
//    daughter()->SetPaintDevice(m_paintDevice);
//}

void TExpr_Root::Paint(QPainter *painter, int X, int Y)
{  
  if (!hasSon())
    return;      // без son лучше ничего не рисовать.

  int sonH = 0;
  int sonW = 0;
  if (hasSon()){
    sonH = son()->height();
    sonW = son()->width();
  }

  int H = 3*m_wy + sonH;
  int W = sonW + 8*m_wx + qRound((sonH + m_wy)/2.0);

  QPolygonF pl;
  pl << QPointF(X+m_wx, Y + 6*m_wy); // 0
  pl << QPointF(X + 6*m_wx - 1, Y + 6*m_wy); // 1
  pl << QPointF(X + 6*m_wx - 1, Y + qRound(H - 3*sqrt(3)*m_wy)); // 2
  pl << QPointF(X + W - 3*m_wx - sonW - m_wx/2 - 1, Y + m_wy); // 3
  pl << QPointF(X + W - m_wx - 1, Y + m_wy); // 4
  pl << QPointF(X + W - m_wx -1, Y + 4*m_wy - 1); // 5
  pl << QPointF(X + W - 2*m_wx, Y + 4*m_wy - 1); // 6
  pl << QPointF(X + W - 2*m_wx, Y + 2*m_wy - 1); // 7
  pl << QPointF(X + W -3*m_wx - sonW - 1, Y + 2*m_wy - 1); // 8
  pl << QPointF(X + 4*m_wx, Y + H); // 9
  pl << QPointF(X + 4*m_wx, Y + 7*m_wy -1); // 10
  pl << QPointF(X + m_wx, Y + 7*m_wy -1); // 11
  pl << pl[0];

  if (hasDaughter()){
    int DX = qMax(0, daughter()->width() - 5*m_wx);
    int DY = qMax(0, daughter()->height() - 4*m_wy);
    for (int i = 0; i < pl.size(); ++i) {
      pl[i].setX(pl.at(i).x() + DX);
      pl[i].setY(pl.at(i).y() + DY);
    }
    daughter()->Draw(painter,
                     pl.at(1).x(),
                     pl.at(1).y(),
                     ExprDraw::ehRight, ExprDraw::evBottom);
  }

  if (hasSon())
    son()->Draw(painter,
                pl.at(8).x(),
                pl.at(8).y(),
                ExprDraw::ehLeft, ExprDraw::evTop);

  painter->save();
  SetPenAndBrush(painter);
  painter->drawConvexPolygon(pl);
  painter->restore();
}
