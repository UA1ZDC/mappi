#include "exprdraw/Parent/texpr_case.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>

static const QChar BRACKET('{');

TExpr_Case::TExpr_Case(TExpr_Class *_son)
  : TExpr_Parent(_son)
{

}

int TExpr_Case::CalcWidth()
{
// (a&1) -> true - нечетное число, false - четное число
  int W1 = 0;
  int W2 = 0;
  int i = 1;
  TExpr_Class* P = son();
  while (P != 0) {
    if (i&1)
      W1 = qMax(W1, P->width());
    else
      W2 = qMax(W2,P->width());
    ++i;
    P = P->next();
  }

  QFont fn = font();
  QFontMetrics fm(fn);
  int H = height() + 2*m_wx;
  fn.setPointSize(fn.pointSize() * H/fm.boundingRect(BRACKET).height());
  fm = QFontMetrics(fn);

  // Итоговая ширина =
  //  ширина скобки + отступ(2*m_wx) + ширина 1-го столбца + отступ(4*m_wx) + ширина 2-го столбца
  return fm.boundingRect(BRACKET).width() + 6*m_wx + W1 + W2;
}

int TExpr_Case::CalcHeight()
{
  int result = 0;
  int H1 = 0;
  int H2 = 0;
  int i = 1;
  TExpr_Class* P = son();
  while (P != 0){
    if (i&1){
      H1 = P->midLineUp();
      H2 = qAbs(P->midLineDn());
    }
    else{
      result += qMax(H1, P->midLineUp());
      result += qMax(H2, qAbs(P->midLineDn()));
      if (P->hasNext())
        result += 2*m_wy;
    }
    ++i;
    P = P->next();
  }

  if (!(result&1))
    ++result;
  return result;
}

void TExpr_Case::Paint(QPainter *painter, int X, int Y)
{
  // Считаем высоту скобки и рисуем её
  QFont fn = font();
  QFontMetrics fm(fn);
  int H = height() + 2*m_wx;
  fn.setPointSize(fn.pointSize() * H/fm.boundingRect(BRACKET).height());
  fm = QFontMetrics(fn);

  //QPoint pos(X - fm.leftBearing(BRACKET), Y + fm.ascent() + midLineUp());
  QPoint pos(X - fm.leftBearing(BRACKET), Y + fm.ascent() + height()/2);
  QRect rec = fm.boundingRect(BRACKET);
  int dY = fm.ascent() + rec.top() + rec.height()/2;

  painter->save();
  SetPenAndBrush(painter);
  painter->setFont(fn);
  painter->translate(pos.x(), pos.y() - dY);
  painter->drawText(0,0, QString(BRACKET));
  painter->restore();

  // Скобка нарисована. Считаем координаты столбцов.S
  int W = X + width();                      // X для второго столбца
  int DX = X + rec.width() + 2*m_wx;       // X для первого столбца
  int H1 = 0;
  int H2 = 0;
  H = Y;
  TExpr_Class* P = son();
  TExpr_Class* PP = 0;

  int i = 1;
  while (P != 0) {
    if (i&1){
      H1 = P->midLineUp();
      H2 = P->midLineDn();
      PP = P;
    }
    else{
      H1 = qMax(H1, P->midLineUp());
      H2 = qMax(H2, -P->midLineDn());
      H += H1;
      if (PP != 0)
        PP->Draw(painter, DX, H, ExprDraw::ehLeft, ExprDraw::evCenter);
      P->Draw(painter, W, H, ExprDraw::ehRight, ExprDraw::evCenter);
      H += H2 + 2*m_wy;
    }
    ++i;
    P = P->next();
  }
}
