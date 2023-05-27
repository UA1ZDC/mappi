#include "exprdraw/Parent/TwinParent/GroupOp/texpr_integral.h"

#include <QFontMetrics>
#include <QPainter>

TExpr_Integral::TExpr_Integral(TExpr_Class* _son,
                               TExpr_Class* _firstTwin,
                               TExpr_Class* _secondTwin,
                               int _mult)
  : TExpr_GroupOp(_son, _firstTwin, _secondTwin)
{
  m_ch = QChar(8747);

  if (_mult <= 0){
    m_integral.append(m_ch);
    m_integral.append(m_ch);
    m_integral.append(QChar(ExprDraw::esEllipsis));
    m_integral.append(m_ch);
  }
  else{
    for (int i = 0; i < _mult; ++i) {
      m_integral.append(m_ch);
    }
  }
}

int TExpr_Integral::CalcSymbolWidth()
{
  QFontMetrics fm(m_symbol_font);
  return fm.width(m_integral);
}

void TExpr_Integral::DrawSymbol(QPainter *painter, int X, int Y)
{
  QFontMetrics fm(m_symbol_font);

  QPoint pos(X, Y + fm.ascent());
  QRect rec = fm.boundingRect(m_ch);
  int dY = fm.ascent() + rec.top() + rec.height()/2;
  int dX = fm.width(m_integral)/2;

  painter->save();
  SetPenAndBrush(painter);
  painter->setFont(m_symbol_font);

  painter->translate(pos.x() - dX, pos.y() - dY);
  painter->drawText(0,0, m_integral);
  painter->restore();
}
