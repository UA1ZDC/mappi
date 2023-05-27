#include "exprdraw/Parent/BigParent/texpr_atvalue.h"

#include <QPainter>


TExpr_AtValue::TExpr_AtValue(TExpr_Class *_son, TExpr_Class *_daughter)
  :TExpr_BigParent(_son, _daughter)
{
  SetDaughterFont();
}

int TExpr_AtValue::CalcWidth()
{
  int result = 3*m_wx;
  if (hasSon())
    result += son()->width();
  if (hasDaughter())
    result += daughter()->width();
  return result;
}

int TExpr_AtValue::CalcHeight()
{
  int sH = hasSon() ? son()->height() : 0;
  int dH = hasDaughter() ? daughter()->height() : 0;
  return qMax(sH,dH);
}

int TExpr_AtValue::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (origin == ExprDraw::eoTop){
    int sonH = 0;
    int sonMidUp = 0;
    if (hasSon()){
      sonH = son()->height();
      sonMidUp = son()->midLineUp();
    }

    int daughterH = hasDaughter() ? daughter()->height() : 0;
    return sonMidUp + qMax(0, daughterH - sonH);
  }

  if (hasSon())
    return son()->midLineDn();
  return TExpr_BigParent::CalcMidLine(origin);
}

void TExpr_AtValue::SetDaughterFont()
{
  if (hasDaughter()){
    QFont fn = font();
    fn.setPointSize(qRound(0.7 * fn.pointSize()));
    daughter()->SetFont(fn);
  }
}

//void TExpr_AtValue::SetDaughterPaintDevice()
//{
//  if (hasDaughter())
//    daughter()->SetPaintDevice(m_paintDevice);
//}

int TExpr_AtValue::FTType()
{
  if ((hasSon()) && ((son()->FTType() & ExprDraw::efLeft) > 0))
    return ExprDraw::efLeft;
  return 0;
}

void TExpr_AtValue::Paint(QPainter* painter, int X, int Y)
{
  int sonH = 0;
  int sonW = 0;
  if (hasSon()){
    sonH = son()->height();
    sonW = son()->width();
  }
  int daughterH = hasDaughter() ? daughter()->height() : 0;
  int DH = qMax(0, daughterH -sonH);

  if (hasSon())
    son()->Draw(painter,
                X, Y+DH,
                ExprDraw::ehLeft, ExprDraw::evTop);

  int H = qMax(sonH, daughterH);
  int W = X + sonW;

  painter->save();
  SetPenAndBrush(painter);
  QPen pen = painter->pen();
  QRectF rec;
  rec.setLeft(W);
  rec.setTop(Y);
  rec.setRight(W + m_wx - pen.width());
  rec.setBottom(Y + H - pen.width());
  painter->drawRect(rec);
  painter->restore();

  if (hasDaughter())
    daughter()->Draw(painter,
                     W + 3*m_wx,
                     Y + H,
                     ExprDraw::ehLeft, ExprDraw::evBottom);
}
